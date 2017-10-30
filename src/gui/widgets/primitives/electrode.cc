// @file:     electrode.cc
// @author:   Nathan
// @created:  2017.10.27
// @editted:  2017.10.27 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     Electrode classes

#include <algorithm>
#include "electrode.h"
#include "src/settings/settings.h"



// Initialize statics
//
QColor prim::Electrode::edge_col;
QColor prim::Electrode::fill_col;


// Draw on layer 0 for now.
prim::Electrode::Electrode(int lay_id, QPoint p1, QPoint p2):
  prim::Item(prim::Item::Electrode)
  // prim::Item(prim::Item::Electrode, lay_id), QPoint p1, QPoint p2,
{
  // // construct static class variables
  // if(diameter < 0)
  //   constructStatics();
  //
  // // set dot location in pixels
  // setPos(p_loc*scale_factor);
  elec_width = std::max(p1.x(), p2.x()) - std::min(p1.x(), p2.x());
  elec_height = std::max(p1.y(), p2.y()) - std::min(p1.y(), p2.y());
  topLeft = QPoint(std::min(p1.x(), p2.x()), std::min(p1.y(), p2.y()));
  // flags
  setFlag(QGraphicsItem::ItemIsSelectable, true);
}

QRectF prim::Electrode::boundingRect() const
{
  // qreal width = diameter+edge_width;
  qreal width = 2;
  return QRectF(-.5*width, -.5*width, width, width);
}

// NOTE: nothing in this paint method changes... possibly cache background as
// pre-rendered bitma for speed.
void prim::Electrode::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  // QRectF rect = boundingRect();
  // qreal dxy = .5*edge_width;
  // rect.adjust(dxy,dxy,-dxy,-dxy);
  //
  // // draw outer circle
  // painter->setPen(QPen(edge_col, edge_width));
  // painter->setBrush(fill_col.isValid() ? fill_col : Qt::NoBrush);
  // painter->drawEllipse(rect);
  //
  // // draw inner circle
  // if(!select_mode && isSelected()){
  //     QPointF center = rect.center();
  //     QSizeF size(diameter, diameter);
  //     rect.setSize(size*in_fill);
  //     rect.moveCenter(center);
  //
  //     painter->setPen(Qt::NoPen);
  //     painter->setBrush(in_fill_col);
  //     painter->drawEllipse(rect);
  // }
  QRect rect(topLeft.x(), topLeft.y(), elec_width, elec_height);
  painter->setBrush(fill_col);
  painter->drawRect(rect);
  painter->drawEllipse(rect);
}

prim::Item *prim::Electrode::deepCopy() const
{
  return new prim::Electrode(layer_id, p1, p2);
}

void prim::Electrode::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  qDebug() << QObject::tr("Electrode has seen the mousePressEvent");

  switch(e->buttons()){
    case Qt::RightButton:
      qDebug() << QObject::tr("Electrode: User right clicked.");
      break;
    default:
      prim::Item::mousePressEvent(e);
      break;
  }
}


void prim::Electrode::constructStatics() //needs to be changed to look at electrode settings instead.
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  edge_col= gui_settings->get<QColor>("dbdot/edge_col");
  fill_col= gui_settings->get<QColor>("dbdot/fill_col");
}
