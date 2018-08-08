// @file:     resizable_poly.cc
// @author:   Nathan
// @created:  2018.08.08
// @editted:  2018.08.08 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     ResizablePoly class for the functionality of resizable polygons

#include <algorithm>
#include "src/settings/settings.h"
#include "resizable_poly.h"

prim::ResizablePoly::ResizablePoly(prim::Item::ItemType type, const QPolygonF &poly, const QRectF &scene_rect, int lay_id)
  : prim::Item(type)
{
  initResizablePoly(lay_id, poly, scene_rect);
}

prim::ResizablePoly::~ResizablePoly()
{
  for (prim::PolygonHandle* handle: poly_handles) {
    delete handle;
    handle = 0;
  }
}

QVariant prim::ResizablePoly::itemChange(GraphicsItemChange change, const QVariant &value)
{
  if (change == QGraphicsItem::ItemSelectedChange) {
    if (value == true) {
      for (prim::PolygonHandle* handle : poly_handles) {
        handle->setVisible(true);
      }
    } else {
      for (prim::PolygonHandle* handle : poly_handles) {
        handle->setVisible(false);
      }
    }
  }
  return QGraphicsItem::itemChange(change, value);
}

void prim::ResizablePoly::initResizablePoly(int lay_id, QPolygonF poly_in, QRectF scene_rect_in)
{
  layer_id = lay_id;
  setPolygon(poly_in);
  setSceneRect(scene_rect_in);
  setPos(scene_rect.topLeft());
  //the polygon has points relative to the item's origin.
  //changing the origin with setPos() means we have to readjust the polygon coords.
  poly.translate(-scene_rect.topLeft());
  update();
  setZValue(-1);
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  createHandles();
}

void prim::ResizablePoly::moveItemBy(qreal dx, qreal dy)
{
  scene_rect.moveTopLeft(QPointF(dx, dy)+scene_rect.topLeft());
  moveBy(dx, dy);
}

void prim::ResizablePoly::createHandles()
{
  for (QPointF point: poly) {
    prim::PolygonHandle *handle = new prim::PolygonHandle(point, this);
    poly_handles.append(handle);
    handle->setVisible(false);
  }
}

QRectF prim::ResizablePoly::boundingRect() const
{
  return QRectF(0,0, poly.boundingRect().width(), poly.boundingRect().height());
}
