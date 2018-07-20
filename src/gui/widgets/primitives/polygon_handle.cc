// @file:     polygon_handle.cc
// @author:   Nathan
// @created:  2018.07.17
// @editted:  2018.07.17 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     Handles for polygons, similar to ResizeHandle

#include "polygon_handle.h"

prim::PolygonHandle::PolygonHandle(QPointF point, QGraphicsItem* parent)
  : prim::Item(prim::Item::PolygonHandle, -1, parent)
{
  initPolygonHandle(point);
}

void prim::PolygonHandle::initPolygonHandle(QPointF point)
{
  center = point;
}

QRectF prim::PolygonHandle::boundingRect() const
{
  return QRectF(center.x()-0.5*handle_dim,center.y()-0.5*handle_dim,handle_dim,handle_dim);
}


void prim::PolygonHandle::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  QRectF rect = boundingRect();
  painter->setPen(QPen(QColor(0,0,0), 1));
  painter->setBrush(QColor(255,255,255));
  painter->drawRect(rect);
  return;
}

void prim::PolygonHandle::constructStatics() //needs to be changed to look at electrode settings instead.
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  handle_dim = gui_settings->get<qreal>("polygonhandle/handle_dim");
}
