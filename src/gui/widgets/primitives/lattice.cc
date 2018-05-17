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

  construct();
  //tileApprox();
}


void prim::Lattice::saveLayer(QXmlStreamWriter *ws) const
{
  ws->writeStartElement("layer_prop");

  int fp = settings::AppSettings::instance()->get<int>("float_prc");
  QString fmt_st = settings::AppSettings::instance()->get<QString>("float_fmt");
  char fmt = fmt_st.at(0).toLatin1();
  QString str;

  // common layer properties
  saveLayerProperties(ws);

  // lattice specific properties
  ws->writeStartElement("lat_vec");
  for (int i=0; i<2; i++) {
    ws->writeEmptyElement(QObject::tr("a%1").arg(i+1));
    ws->writeAttribute("x", str.setNum(a[i].x(), fmt, fp));
    ws->writeAttribute("y", str.setNum(a[i].y(), fmt, fp));
  }
  ws->writeTextElement("N", QString::number(n_cell));
  for (int i=0; i<b.size(); i++) {
    ws->writeEmptyElement(QObject::tr("b%1").arg(i+1));
    ws->writeAttribute("x", str.setNum(b[i].x(), fmt, fp));
    ws->writeAttribute("y", str.setNum(b[i].y(), fmt, fp));
  }
  ws->writeEndElement();  // end of lat_vec

  ws->writeEndElement();  // end of layer_prop
}


prim::LatticeCoord prim::Lattice::nearestSite(const QPointF &scene_pos) const
{
  QPointF dummy_site_pos;
  return nearestSite(scene_pos, dummy_site_pos);
}


prim::LatticeCoord prim::Lattice::nearestSite(const QPointF &scene_pos, QPointF &nearest_site_pos) const
{
  // TODO ask Jake if these calculations should be done using the scene integer
  //      version of the variables
  LatticeCoord coord(0,0,-1);

  int n0[2];
  qreal proj;
  QPointF x = scene_pos/prim::Item::scale_factor;

  for(int i=0; i<2; i++){
    proj = QPointF::dotProduct(x, a[i])/a2[i];
    if(!orthog){
      qreal x2 = QPointF::dotProduct(x,x);
      proj += (proj>0 ? -1:1)*coth*qSqrt(qMax(0.,x2/a2[i]-proj*proj));
    }
    n0[i] = qFloor(proj);
  }

  qreal mdist = qMax(a2[0], a2[1]);         // nearest Manhattan length
  for(int n=n0[0]-1; n<n0[0]+2; n++){
    for(int m=n0[1]-1; m<n0[1]+2; m++){
      QPointF x0 = n*a[0]+m*a[1];
      for (int l=0; l<b.size(); l++){
        QPointF temp = x0 + b[l];
        qreal dist = (temp-x).manhattanLength();
        if(dist<=mdist) {
          mdist = dist;
          nearest_site_pos = temp * prim::Item::scale_factor;
          coord.n = n;
          coord.m = m;
          coord.l = l;
        }
      }
    }
  }

  if (coord.l == -1)
    qFatal("No result for nearest site");

  //qDebug() << tr("Nearest Lattice Site: %1 :: %2").arg(mp.x()).arg(mp.y());

  return coord;
  //return mp;                            // physical (angstrom) coords
  //return mp*prim::Item::scale_factor;   // scene (pixel) coords
}


QList<prim::LatticeCoord> prim::Lattice::enclosedSites(const prim::LatticeCoord &coord1,
    const prim::LatticeCoord &coord2) const
{
  // WARNING assumes n is purely horizontal and m is purely vertical. Might not
  // be the case!
  int n_min = qMin(coord1.n, coord2.n);
  int n_max = qMax(coord1.n, coord2.n);
  int m_min = qMin(coord1.m, coord2.m);
  int m_max = qMax(coord1.m, coord2.m);
  int l_tl, l_br; // top left and bottom right
  if (coord1.m == coord2.m) {
    l_tl = qMin(coord1.l, coord2.l);
    l_br = qMax(coord1.l, coord2.l);
  } else {
    l_tl = coord1.m < coord2.m ? coord1.l : coord2.l; // top left
    l_br = coord1.m > coord2.m ? coord1.l : coord2.l; // bottom right
  }

  QList<prim::LatticeCoord> coords;

  for (int n_site=n_min; n_site<=n_max; n_site++) {
    for (int m_site=m_min; m_site<=m_max; m_site++) {
      for (int l_site=0; l_site<n_cell; l_site++) {
        if (  !(m_min == m_max && l_tl != l_br)
              && ((m_site == m_min && l_site < l_tl)
                  || (m_site == m_max && l_site > l_br)))
          continue;
        coords.append(prim::LatticeCoord(n_site, m_site, l_site));
      }
    }
  }
  return coords;
}


QPointF prim::Lattice::latticeCoord2ScenePos(const prim::LatticeCoord &l_coord) const
{
  if (l_coord.l >= b_scene.size() || l_coord.l <= -b_scene.size())
    qCritical() << QObject::tr("coordinate invalid! (%1,%2,%3)").arg(l_coord.n).arg(l_coord.m).arg(l_coord.l);
  //qDebug() << QObject::tr("Coordinates: (%1,%2,%3)").arg(l_coord.n).arg(l_coord.m).arg(l_coord.l);
  QPointF lattice_scene_loc;
  lattice_scene_loc += l_coord.n * a_scene[0];
  lattice_scene_loc += l_coord.m * a_scene[1];
  lattice_scene_loc += b_scene[qAbs(l_coord.l)] * (l_coord.l > 0 ? 1 : -1);
  return lattice_scene_loc;
}


bool prim::Lattice::collidesWithLatticeSite(const QPointF &scene_pos,
    const prim::LatticeCoord &l_coord) const
{
  QPointF target_pos = latticeCoord2ScenePos(l_coord);
  QPointF delta = target_pos - scene_pos;
  if (delta.manhattanLength() < 0.5 * settings::GUISettings::instance()->
      get<qreal>("latdot/diameter") * prim::Item::scale_factor)
    return true;
  return false;
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
  // TODO publish mode

  QPixmap bkg_pixmap(QSize(a_scene[0].x(), a_scene[1].y()));
  bkg_pixmap.fill(bkg_col);
  QPainter painter(&bkg_pixmap);
  painter.setBrush(Qt::NoBrush);
  painter.setPen(QPen(lat_edge_col, lat_edge_width));
  painter.setRenderHint(QPainter::Antialiasing);
  for (QPoint site : b_scene)
    painter.drawEllipse(site.x()+lat_edge_width, site.y()+lat_edge_width, lat_diam, lat_diam);
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

  qreal dtrm = a[0].x()*a[1].y() - a[0].y()*a[1].x();
  coth = QPointF::dotProduct(a[0], a[1]) / dtrm;

  orthog = qAbs(coth) < 1e-4;

  // generate lattice and site vectors for display (all integer)
  a_scene[0] = QPointF(tileApprox().topRight() * prim::Item::scale_factor).toPoint();
  a_scene[1] = QPointF(tileApprox().bottomLeft() * prim::Item::scale_factor).toPoint();
  for (QPointF site : b)
    b_scene.append(QPointF(site * prim::Item::scale_factor).toPoint());
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
