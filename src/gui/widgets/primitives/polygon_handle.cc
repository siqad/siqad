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
  setParentItem(parent);
  initPolygonHandle(point);
}

prim::PolygonHandle::~PolygonHandle()
{
  setVisible(false);
}

void prim::PolygonHandle::initPolygonHandle(QPointF point)
{
  constructStatics();
  center = point;
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setFlag(QGraphicsItem::ItemIsMovable, true);

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

void prim::PolygonHandle::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  update();
  switch(e->buttons()) {
    case Qt::LeftButton:
    {
      // if not attached to a parent item, then don't begin resizing.
      if (parentItem()) {
        clicked = true;
        emit prim::Emitter::instance()->sig_resizeBegin();
      }
      break;
    }
    default:
    {
      prim::Item::mousePressEvent(e);
      break;
    }
  }
}

// void prim::PolygonHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
// {
//   if (clicked) {
//     setPoint(e->pos());
//     update();
//     e->accept();
//   }
// }

void prim::PolygonHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
  if (clicked) {
    update();
    if (parentItem())
      emit prim::Emitter::instance()->sig_resizeFinalizePoly(static_cast<prim::Item *>(parentItem()));
  }
  clicked = false;
}


void prim::PolygonHandle::constructStatics() //needs to be changed to look at electrode settings instead.
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  handle_dim = gui_settings->get<qreal>("polygonhandle/handle_dim");
}
