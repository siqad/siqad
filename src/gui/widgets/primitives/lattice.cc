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
#include <QDialog>
#include <algorithm>


qreal prim::Lattice::rtn_acc = 1e-3;
int prim::Lattice::rtn_iters = 1;

prim::Lattice::Lattice(const QString &fname, int lay_id)
  : Layer(tr("Lattice"),Layer::Lattice,0)
{
  layer_id = lay_id;
  settings::LatticeSettings::updateLattice(fname);
  // if(fname.isEmpty())
  //   settings::LatticeSettings::updateLattice(fname);

  construct();
  //tileApprox();
}


QPointF prim::Lattice::nearestSite(const QPointF &scene_pos)
{
  int n0[2];
  qreal proj;
  QPointF x = scene_pos/prim::Item::scale_factor;
  qreal x2 = QPointF::dotProduct(x,x);

  for(int i=0; i<2; i++){
    proj = QPointF::dotProduct(x, a[i])/a2[i];
    n0[i] = qFloor(proj - coth*qSqrt(x2/a2[i]-proj*proj));
  }

  qreal mdist = qMax(a2[0], a2[1]);         // nearest Manhattan length
  QPointF mp;                               // nearest lattice site
  for(int n=n0[0]-1; n<n0[0]+2; n++){
    for(int m=n0[1]-1; m<n0[1]+2; m++){
      QPointF x0 = n*a[0]+m*a[1];
      for(const QPointF& dp : b){
        QPointF temp = x0 + dp;
        qreal dist = (temp-x).manhattanLength();
        if(dist<mdist){
          mdist = dist;
          mp = temp;
        }
      }
    }
  }

  //qDebug() << tr("Nearest Lattice Site: %1 :: %2").arg(mp.x()).arg(mp.y());

  return mp;                            // physical (angstrom) coords
  //return mp*prim::Item::scale_factor;   // scene (pixel) coords
}


QRectF prim::Lattice::tileApprox()
{
  qreal r = QPointF::dotProduct(a[0], a[1])/QPointF::dotProduct(a[0], a[0]);
  QPair<int,int> frac = rationalize(r);

  qDebug() << tr("Lattice skew: %1 :: ( %2 / %3 )").arg(r).arg(frac.first).arg(frac.second);
  qreal height = qSqrt(QPointF::dotProduct(a[0], a[0]));
  QPointF v = frac.first*a[0] - frac.second*a[1];
  qreal width = qSqrt(QPointF::dotProduct(v,v));

  QRectF rect(0,0,width,height);

  qDebug() << tr("Width: %1 :: Height: %2").arg(width).arg(height);

  return rect;
}


void prim::Lattice::construct()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  settings::LatticeSettings *lattice_settings = settings::LatticeSettings::instance();

  // get values from the lattice_settings
  n_cell = lattice_settings->get<int>("cell/N");
  for(int i=0;i<n_cell;i++)
    b.append(lattice_settings->get<QPointF>(QString("cell/b%1").arg(i+1)));
  for(int i=0;i<2;i++){
    a[i] = lattice_settings->get<QPointF>(QString("lattice/a%1").arg(i+1));
    a2[i] = QPointF::dotProduct(a[i], a[i]);
  }

  // construct bounds for lattice vectors
  qreal dx = qMax(qAbs(a[0].x()),qAbs(a[1].x()));
  qreal dy = qMax(qAbs(a[0].y()),qAbs(a[1].y()));
  QPoint nxy = gui_settings->get<QPoint>("lattice/xy");

  Lx = dx*nxy.x();
  Ly = dy*nxy.y();

  qreal dtrm = a[0].x()*a[1].y() - a[0].y()*a[1].x();
  coth = QPointF::dotProduct(a[0], a[1]) / dtrm;

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


QPair<int,int> prim::Lattice::rationalize(qreal x, int k)
{
  int n = qFloor(x);
  qreal r = x - n;

  qDebug() << tr("%1 :: %2 :: %3").arg(x).arg(n).arg(r);
  QPair<int,int> pair;

  if ( r < rtn_acc || k == rtn_iters ){
    pair.first = n;
    pair.second = 1;
  }
  else{
    QPair<int,int> other = rationalize(1./r, k+1);
    pair.first = other.first*n+other.second;
    pair.second = other.first;
  }

  return pair;
}
