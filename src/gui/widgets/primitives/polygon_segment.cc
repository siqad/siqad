// @file:     polygon_handle.cc
// @author:   Nathan
// @created:  2018.07.17
// @editted:  2018.07.17 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     Handles for polygons, similar to ResizeHandle

#include "polygon_segment.h"

qreal prim::PolygonSegment::line_width = -1;

prim::PolygonSegment::PolygonSegment(QPointF start, QPointF end, QGraphicsItem* parent)
  : prim::Item(prim::Item::PolygonSegment, -1, parent)
{
  initPolygonSegment(start, end);
}

void prim::PolygonSegment::initPolygonSegment(QPointF start_in, QPointF end_in)
{
  start = start_in;
  end = end_in;
  constructStatics();
}

QRectF prim::PolygonSegment::boundingRect() const
{
  qreal x_min = qMin(start.x(), end.x()) - line_width*0.5;
  qreal y_min = qMin(start.y(), end.y()) - line_width*0.5;
  qreal dx = qMax(start.x(), end.x()) - x_min + line_width;
  qreal dy = qMax(start.y(), end.y()) - y_min + line_width;
  return QRectF(x_min, y_min, dx, dy);
}

void prim::PolygonSegment::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  painter->setPen(QPen(QColor(255,255,255), line_width));
  painter->setBrush(QColor(255,255,255));
  painter->drawLine(start, end);
}

void prim::PolygonSegment::constructStatics() //needs to be changed to look at electrode settings instead.
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  line_width = gui_settings->get<qreal>("polygonsegment/line_width");
}
