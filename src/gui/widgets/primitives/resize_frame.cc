// @file:     resize_frame.cc
// @author:   Samuel
// @created:  2018-03-19
// @editted:  2018-03-19 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Resize handles for resizable objects

#include "resize_frame.h"
#include "afmarea.h"

namespace prim{

// Initialise statics
qreal ResizeFrame::border_width = -1;
QList<ResizeFrame::HandlePosition> ResizeFrame::handle_positions;
qreal ResizeHandle::handle_dim = -1;
prim::Item::StateColors handle_col;

// Constructor
ResizeFrame::ResizeFrame(prim::Item *resize_target)
  : Item(prim::Item::ResizeFrame), resize_target(resize_target)
{
  if (border_width == -1)
    prepareStatics();

  if (resize_target)
    setParentItem(resize_target);

  // create a set of handles
  for (HandlePosition handle_pos : handle_positions) {
    prim::ResizeHandle *handle = new prim::ResizeHandle(handle_pos, this);
    resize_handles.append(handle);
  }

  // Graphics
  setFlag(QGraphicsItem::ItemIsSelectable, false);
  setFlag(QGraphicsItem::ItemIsFocusable, false);
}


void ResizeFrame::setResizeTarget(prim::Item *new_target)
{
  resize_target = new_target;
  setParentItem(resize_target);

  if (resize_target)
    updateHandlePositions();
}


void ResizeFrame::resizeTargetToHandle(const HandlePosition &pos, const QPointF &delta)
{
  // TODO change AFMArea to generic rect item
  prim::AFMArea *target = static_cast<prim::AFMArea*>(resizeTarget());
  switch (pos) {
    case TopLeft:
      target->resize(delta.x(), delta.y(), 0, 0);
      break;
    case Top:
      target->resize(0, delta.y(), 0, 0);
      break;
    case TopRight:
      target->resize(0, delta.y(), delta.x(), 0);
      break;
    case Right:
      target->resize(0, 0, delta.x(), 0);
      break;
    case BottomRight:
      target->resize(0, 0, delta.x(), delta.y());
      break;
    case Bottom:
      target->resize(0, 0, 0, delta.y());
      break;
    case BottomLeft:
      target->resize(delta.x(), 0, 0, delta.y());
      break;
    case Left:
      target->resize(delta.x(), 0, 0, 0);
      break;
    default:
      qCritical() << "Trying to access a non-existent resize handle position";
      break;
  }
  updateHandlePositions();
}


void ResizeFrame::updateHandlePositions()
{
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

ResizeHandle::ResizeHandle(prim::ResizeFrame::HandlePosition handle_pos,
    QGraphicsItem *parent)
  : Item(prim::Item::ResizeHandle, -1, parent), handle_position(handle_pos)
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
  setFlag(QGraphicsItem::ItemIsMovable, true);
}

void ResizeHandle::updatePosition()
{
  QRectF p_rect = parentItem()->boundingRect();
  switch (handle_position) {
    case prim::ResizeFrame::TopLeft:
      setPos(p_rect.topLeft());
      break;
    case prim::ResizeFrame::Top:
      setPos(QPointF((p_rect.left()+p_rect.right())/2, p_rect.top()));
      break;
    case prim::ResizeFrame::TopRight:
      setPos(p_rect.topRight());
      break;
    case prim::ResizeFrame::Right:
      setPos(QPointF(p_rect.right(), (p_rect.top()+p_rect.bottom())/2));
      break;
    case prim::ResizeFrame::BottomRight:
      setPos(p_rect.bottomRight());
      break;
    case prim::ResizeFrame::Bottom:
      setPos(QPointF((p_rect.left()+p_rect.right())/2, p_rect.bottom()));
      break;
    case prim::ResizeFrame::BottomLeft:
      setPos(p_rect.bottomLeft());
      break;
    case prim::ResizeFrame::Left:
      setPos(QPointF(p_rect.left(), (p_rect.top()+p_rect.bottom())/2));
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
    case Qt::LeftButton:
    {
      clicked = true;
      step_pos = e->scenePos();
      static_cast<prim::ResizeFrame*>(parentItem())->resizeTarget()->beginResize();
      e->accept();
      break;
    }
    default:
    {
      prim::Item::mousePressEvent(e);
      break;
    }
  }
}

void ResizeHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
  if (clicked) {
    QPointF step_delta = e->scenePos() - step_pos;
    setPos(pos()+step_delta);
    static_cast<prim::ResizeFrame*>(parentItem())->resizeTargetToHandle(handle_position, step_delta);
    step_pos = e->scenePos();
    e->accept();
  }
}

void ResizeHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
  if (clicked)
    static_cast<prim::ResizeFrame*>(parentItem())->resizeTarget()->finalizeResize();
  clicked = false;
}

void ResizeHandle::prepareStatics()
{
  // TODO settings.cc
  handle_dim = 10;
}

} // end of prim namespace
