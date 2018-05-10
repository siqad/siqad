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

  qDebug() << tr("Nearest Lattice Site: %1 :: %2").arg(mp.x()).arg(mp.y());

  return mp;                            // physical (angstrom) coords
  //return mp*prim::Item::scale_factor;   // scene (pixel) coords
}


QRectF prim::Lattice::tileApprox()
{
  qreal r = QPointF::dotProduct(a[0], a[1])/QPointF::dotProduct(a[0], a[0]);
  QPair<int,int> frac = rationalize(r);

  qDebug() << tr("Lattice skew: %1 :: ( %2 / %3 )").arg(r).arg(frac.first).arg(frac.second);
  qreal width = qSqrt(QPointF::dotProduct(a[0], a[0]));
  QPointF v = frac.first*a[0] - frac.second*a[1];
  qreal height = qSqrt(QPointF::dotProduct(v,v));

  QRectF rect(0,0,width,height);

  qDebug() << tr("Width: %1 :: Height: %2").arg(width).arg(height);

  return rect;
}


QImage prim::Lattice::tileableLatticeImage(QColor bkg_col)
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  qreal lat_diam = gui_settings->get<qreal>("latdot/diameter") * prim::Item::scale_factor;
  qreal lat_edge_width = gui_settings->get<qreal>("latdot/edge_width") * lat_diam;
  QColor lat_edge_col = gui_settings->get<QColor>("latdot/edge_col");
  // TODO the rest

  // first generate a single tile with fully contained circles
  QPixmap bkg_pixmap(QSize(QSizeF(tileApprox().size()*prim::Item::scale_factor).toSize()));
  bkg_pixmap.fill(bkg_col);
  QPainter painter(&bkg_pixmap);
  painter.setBrush(Qt::NoBrush);
  painter.setPen(QPen(lat_edge_col, lat_edge_width));
  painter.setRenderHint(QPainter::Antialiasing);
  for (QPointF site : b) {
    painter.drawEllipse(site.x()*prim::Item::scale_factor+lat_edge_width, site.y()*prim::Item::scale_factor+lat_edge_width, lat_diam, lat_diam);
  }
  painter.end();

  // then generate a single tile with properly offset circles
  QImage bkg_img(QSize(
            QSizeF(tileApprox().size()*prim::Item::scale_factor).toSize()),
            QImage::Format_ARGB32);
  QPainter painter_offset(&bkg_img);
  int offset = 0.5 * lat_diam + lat_edge_width;
  painter_offset.drawTiledPixmap(bkg_pixmap.rect(), bkg_pixmap, QPoint(offset,offset));
  painter_offset.end();

  return bkg_img;
}


void prim::Lattice::construct()
{
  settings::LatticeSettings *lattice_settings = settings::LatticeSettings::instance();

  // get values from the lattice_settings
  n_cell = lattice_settings->get<int>("cell/N");
  for(int i=0;i<n_cell;i++)
    b.append(lattice_settings->get<QPointF>(QString("cell/b%1").arg(i+1)));
  for(int i=0;i<2;i++){
    a[i] = lattice_settings->get<QPointF>(QString("lattice/a%1").arg(i+1));
    a2[i] = QPointF::dotProduct(a[i], a[i]);
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
