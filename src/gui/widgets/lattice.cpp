#include "lattice.h"

#include <QtMath>
#include <QVariant>
#include <QDebug>
#include <QStringList>
#include <QGraphicsItem>

#include <algorithm>


gui::Lattice::Lattice()
  : Layer("Lattice")
{
  settings::LatticeSettings lattice_settings;
  construct(lattice_settings);
}
gui::Lattice::Lattice(const QString &fname)
  : Layer("Lattice")
{
  settings::LatticeSettings lattice_settings(fname);
  construct(lattice_settings);
}

gui::Lattice::~Lattice()
{}


void gui::Lattice::construct(settings::LatticeSettings &lattice_settings)
{

  settings::GUISettings gui_settings;

  // get values from the lattice_settings
  n_cell = lattice_settings.get<int>("cell/N");
  for(int i=0;i<n_cell;i++)
    b.append(lattice_settings.get<QPointF>(QString("cell/b%1").arg(i+1)));
  for(int i=0;i<2;i++)
    a[i] = lattice_settings.get<QPointF>(QString("lattice/a%1").arg(i+1));

  // construct bounds for lattice vectors
  qreal dx = qMax(qAbs(a[0].x()),qAbs(a[1].x()));
  qreal dy = qMax(qAbs(a[0].y()),qAbs(a[1].y()));
  QPoint nxy = gui_settings.get<QPoint>("lattice/xy");

  Lx = dx*nxy.x();
  Ly = dy*nxy.y();

  // find all lattice vector indices within the bounding region
  QList<QPoint> inds;
  getLatticeInds(&inds);

  // for each set of indices, create the associated unit cell
  for(int i=0; i<inds.count(); i++)
    buildUnitCell(inds.at(i));
}

// recursively checks each n for pairs (n,m) and return true if such pairs exist
// terminates recursion branch if no pairs found.
void gui::Lattice::getLatticeInds(QList<QPoint> *inds, int n)
{
  settings::GUISettings gui_settings;

  // both lattice vectors must have non-zero length
  if(a[0].manhattanLength()==0 || a[1].manhattanLength()==0)
    qFatal("Lattice vector has zero length...");

  // if a[1].x()==0, we must have 0 <= n*a[0].x() <= Lx to have pairs (n,m)
  if(a[1].x()==0 && (n*a[0].x()<0 || n*a[0].x()>Lx))
    return;

  // if a[1].y()==0, we must have 0 <= n*a[0].y() <= Ly to have pairs (n,m)
  if(a[1].x()==0 && (n*a[0].x()<0 || n*a[0].x()>Lx))
    return;

  // find inclusive lower and upper bounds for m
  int low, high;
  findBounds(&low, &high, n);

  // if no valid values of m, terminate recursion in branch
  if(low>high)
    return;

  // append indices to list
  for(int m=low; m<=high; m++)
    inds->append(QPoint(n,m));

  // if n==0, step in both directions, else continue with sign of n
  if(n>=0)
    getLatticeInds(inds, n+1);
  if(n<=0)
    getLatticeInds(inds, n-1);
}


void gui::Lattice::findBounds(int *low, int *high, int n)
{
  // compute useful values for finding bound of m
  qreal sx, tx, sy, ty;
  if(a[1].x()!=0){
    sx = -n*a[0].x()/a[1].x();
    tx = sx+Lx/a[1].x();
  }
  if(a[1].y()!=0){
    sy = -n*a[0].y()/a[1].y();
    ty = sy+Ly/a[1].y();
  }

  // now swap s and t if ex. a[1].x()<0
  if(a[1].x()<0)
    std::swap(sx, tx);

  if(a[1].y()<0)
    std::swap(sy, ty);

  // select tight bounds

  if(a[1].x()==0){
    *low = qCeil(sy);
    *high = qFloor(ty);
  }
  else if(a[1].y()==0){
    *low = qCeil(sx);
    *high = qFloor(tx);
  }
  else{
    *low = qCeil(qMax(sx,sy));
    *high = qFloor(qMin(tx,ty));
  }
}


void gui::Lattice::buildUnitCell(QPoint ind)
{
  // compute unit cell origin
  QPointF lattice_loc = a[0]*ind.x()+a[1]*ind.y();

  for(int n=0; n<n_cell; n++){
    prim::DBDot *dot = new prim::DBDot(lattice_loc+b.at(n), true);
    dot->setFlag(QGraphicsItem::ItemIsSelectable, true);
    addItem(dot);
  }
}
