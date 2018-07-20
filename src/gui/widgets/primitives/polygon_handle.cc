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
  qDebug() << point;
  qDebug() << "POLYGON HANDLE";
  initPolygonHandle(point);
}

void prim::PolygonHandle::initPolygonHandle(QPointF point)
{
  center = point;
  qDebug() << "initPolygonHandle()";
}

QRectF prim::PolygonHandle::boundingRect() const
{
  return QRectF(center.x()-50,center.y()-50,100,100);
}


void prim::PolygonHandle::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  qDebug() << "paint()";
  QRectF rect = boundingRect();
  painter->setPen(QPen(QColor(0,0,0), 1));
  painter->setBrush(QColor(255,255,255));
  painter->drawRect(rect);
  return;
}
