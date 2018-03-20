// @file:     resize_frame.cc
// @author:   Samuel
// @created:  2018-03-19
// @editted:  2018-03-19 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Resize handles for resizable objects

#include "resize_frame.h"

namespace prim{

// Initialise statics
qreal ResizeFrame::border_width = -1;
QList<ResizeFrame::HandlePosition> ResizeFrame::handle_positions;
qreal ResizeHandle::handle_dim = -1;
prim::Item::StateColors handle_col;

// Constructor
ResizeFrame::ResizeFrame(int lay_id, prim::Item *resize_target)
  : Item(prim::Item::ResizeFrame, lay_id), resize_target(resize_target)
{
  if (border_width == -1)
    prepareStatics();

  //setPos(resize_target->pos());

  // create a set of handles
  for (HandlePosition handle_pos : handle_positions) {
    prim::ResizeHandle *handle = new prim::ResizeHandle(lay_id, handle_pos,
        this);
    resize_handles.append(handle);
  }

  // Graphics
  setFlag(QGraphicsItem::ItemIsSelectable, false);
  setFlag(QGraphicsItem::ItemIsFocusable, false);
}


void ResizeFrame::setResizeTarget(prim::Item *new_target)
{
  resize_target = new_target;

  setPos(new_target->pos());

  // each handle will determine its new position using the bounding rect of the
  // new target
  for (prim::ResizeHandle *handle : resize_handles)
    handle->updatePosition();
}


void ResizeFrame::paint(QPainter *painter, const QStyleOptionGraphicsItem*,
    QWidget*)
{
  // TODO draw a rectangular border
}


void ResizeFrame::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  switch(e->buttons()) {
    default:
      prim::Item::mousePressEvent(e);
      break;
  }
}

// Initialize static class variables
void ResizeFrame::prepareStatics()
{
  handle_positions.append(QList<HandlePosition>({TopLeft, Top, TopRight, Right,
      BottomRight, Bottom, BottomLeft, Left}));

  // TODO graphics
}





// ResizeHandle class

ResizeHandle::ResizeHandle(int lay_id,
    prim::ResizeFrame::HandlePosition handle_pos, QGraphicsItem *parent)
  : Item(prim::Item::ResizeHandle, lay_id, parent), handle_position(handle_pos)
{
  if (handle_dim == -1)
    prepareStatics();

  updatePosition();

  switch (handle_position) {
    case prim::ResizeFrame::TopLeft:
    case prim::ResizeFrame::BottomRight:
      setCursor(Qt::SizeFDiagCursor);
      break;
    case prim::ResizeFrame::TopRight:
    case prim::ResizeFrame::BottomLeft:
      setCursor(Qt::SizeBDiagCursor);
      break;
    case prim::ResizeFrame::Top:
    case prim::ResizeFrame::Bottom:
      setCursor(Qt::SizeVerCursor);
      break;
    case prim::ResizeFrame::Left:
    case prim::ResizeFrame::Right:
      setCursor(Qt::SizeHorCursor);
      break;
    default:
      qCritical() << "Trying to access a non-existent resize handle position";
      break;
  }

  // Graphics
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setFlag(QGraphicsItem::ItemIsFocusable, true);
}

void ResizeHandle::updatePosition()
{
  QPointF f_pos = mapFromScene(static_cast<prim::ResizeFrame*>(parentItem())->
      resizeTarget()->pos());
  QRectF p_rect = parentItem()->boundingRect();
  switch (handle_position) {
    case prim::ResizeFrame::TopLeft:
      setPos(f_pos+p_rect.topLeft());
      break;
    case prim::ResizeFrame::Top:
      setPos(f_pos+QPointF((p_rect.left()+p_rect.right())/2, p_rect.top()));
      break;
    case prim::ResizeFrame::TopRight:
      setPos(f_pos+p_rect.topRight());
      break;
    case prim::ResizeFrame::Right:
      setPos(f_pos+QPointF(p_rect.right(), (p_rect.top()+p_rect.bottom())/2));
      break;
    case prim::ResizeFrame::BottomRight:
      setPos(f_pos+p_rect.bottomRight());
      break;
    case prim::ResizeFrame::Bottom:
      setPos(f_pos+QPointF((p_rect.left()+p_rect.right())/2, p_rect.bottom()));
      break;
    case prim::ResizeFrame::BottomLeft:
      setPos(f_pos+p_rect.bottomLeft());
      break;
    case prim::ResizeFrame::Left:
      setPos(f_pos+QPointF(p_rect.left(), (p_rect.top()+p_rect.bottom())/2));
      break;
    default:
      qCritical() << "Trying to access a non-existent resize handle position";
      break;
  }
  update();
}

QRectF ResizeHandle::boundingRect() const
{
  return QRectF(-.5*handle_dim, -.5*handle_dim, handle_dim, handle_dim);
}

void ResizeHandle::paint(QPainter *painter,
    const QStyleOptionGraphicsItem *, QWidget *)
{
  QRectF rect = boundingRect();
  // TODO use static parameters for pen
  painter->setPen(QPen(QColor(0,0,0), 1));
  painter->setBrush(QColor(255,255,255));
  painter->drawRect(rect);
}

void ResizeHandle::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  switch(e->buttons()) {
    default:
      prim::Item::mousePressEvent(e);
      break;
  }
}

void ResizeHandle::prepareStatics()
{
  // TODO settings.cc
  handle_dim = 10;
}

} // end of prim namespace
