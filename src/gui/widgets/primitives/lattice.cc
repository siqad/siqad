// @file:     lattice.cc
// @author:   Jake
// @created:  2016.11.15
// @editted:  2017.06.07  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Lattice definitions


#include "lattice.h"
#include "src/settings/settings.h"

#include <QtMath>
#include <algorithm>


prim::Lattice::Lattice(const QString &fname, int lay_id)
  : Layer(tr("Lattice"),tr("lattice"))
{
  layer_id = lay_id;
  settings::LatticeSettings::updateLattice(fname);
  // if(fname.isEmpty())
  //   settings::LatticeSettings::updateLattice(fname);

  construct();
}


void prim::Lattice::construct()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  settings::LatticeSettings *lattice_settings = settings::LatticeSettings::instance();

  // get values from the lattice_settings
  n_cell = lattice_settings->get<int>("cell/N");
  for(int i=0;i<n_cell;i++)
    b.append(lattice_settings->get<QPointF>(QString("cell/b%1").arg(i+1)));
  for(int i=0;i<2;i++)
    a[i] = lattice_settings->get<QPointF>(QString("lattice/a%1").arg(i+1));

  // construct bounds for lattice vectors
  qreal dx = qMax(qAbs(a[0].x()),qAbs(a[1].x()));
  qreal dy = qMax(qAbs(a[0].y()),qAbs(a[1].y()));
  QPoint nxy = gui_settings->get<QPoint>("lattice/xy");

  Lx = dx*nxy.x();
  Ly = dy*nxy.y();

  // find all lattice vector indices within the bounding region
  QList<QPoint> inds;
  getLatticeInds(inds);

  // for each set of indices, create the associated unit cell
  for(int i=0; i<inds.count(); i++)
    buildUnitCell(inds.at(i));
}


void prim::Lattice::getLatticeInds(QList<QPoint> &inds, int n)
{
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
  int lo, hi;
  findBounds(lo, hi, n);

  // if no valid values of m, terminate recursion in branch
  if(lo>hi)
    return;

  // append indices to list
  for(int m=lo; m<=hi; m++)
    inds.append(QPoint(n,m));

  // if n==0, step in both directions, else continue with sign of n
  if(n>=0)
    getLatticeInds(inds, n+1);
  if(n<=0)
    getLatticeInds(inds, n-1);
}

void prim::Lattice::findBounds(int &lo, int &hi, int n)
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
    lo = qCeil(sy);
    hi = qFloor(ty);
  }
  else if(a[1].y()==0){
    lo = qCeil(sx);
    hi = qFloor(tx);
  }
  else{
    lo = qCeil(qMax(sx,sy));
    hi = qFloor(qMin(tx,ty));
  }
}


void prim::Lattice::buildUnitCell(const QPoint &ind)
{
  // compute unit cell origin
  QPointF lattice_loc = a[0]*ind.x()+a[1]*ind.y();

  for(int n=0; n<n_cell; n++){
    prim::LatticeDot *dot = new prim::LatticeDot(layer_id, lattice_loc+b.at(n));
    dot->setFlag(QGraphicsItem::ItemIsSelectable, true);
    addItem(dot);
  }
}
