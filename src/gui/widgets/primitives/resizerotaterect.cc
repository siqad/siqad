// @file:     ResizeRotateRect.cc
// @author:   Samuel
// @created:  2018-03-19
// @editted:  2018-03-19 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Resize handles for resizable objects

#include "resizerotaterect.h"
// #include "afmarea.h"

namespace prim{

// Initialise statics
qreal ResizeRotateFrame::border_width = -1;
QList<ResizeRotateFrame::HandlePosition> ResizeRotateFrame::handle_positions;
qreal ResizeRotateHandle::handle_dim = -1;
prim::Item::StateColors rotate_handle_col;


// Resizable Rectangle base class
ResizeRotateRect::ResizeRotateRect(ItemType type, const QRectF &scene_rect, int lay_id,
                             QGraphicsItem *parent)
  : Item(type, lay_id, parent)
{
  setResizable(true);
  setSceneRect(scene_rect);
  setTransform(new QTransform());
}

void ResizeRotateRect::resize(qreal dx1, qreal dy1, qreal dx2, qreal dy2, bool update_handles)
{
  prepareGeometryChange();

  // update dimensions
  scene_rect.setTopLeft(scene_rect.topLeft()+QPointF(dx1,dy1));
  scene_rect.setBottomRight(scene_rect.bottomRight()+QPointF(dx2,dy2));
  setPos(scene_rect.topLeft());
  update();

  // update the frame
  if (update_handles && resize_frame)
    resize_frame->updateHandlePositions();
}

void ResizeRotateRect::preResize()
{
  scene_rect_cache = scene_rect;
  pos_cache = pos();
}

void ResizeRotateRect::moveItemBy(qreal dx, qreal dy)
{
  QRectF rect = scene_rect;
  rect.moveTopLeft(rect.topLeft()+QPointF(dx,dy));
  setSceneRect(rect);
}

void ResizeRotateRect::setSceneRect(const QRectF &rect) {
  scene_rect = rect;
  setPos(scene_rect.topLeft());
  update();
}

QVariant ResizeRotateRect::itemChange(GraphicsItemChange change, const QVariant &value)
{
  if (change == QGraphicsItem::ItemSelectedChange) {
    if (value == true) {
      if (!resize_frame) {
        resize_frame = new prim::ResizeRotateFrame(this);
      }
      resize_frame->setVisible(true);
    } else {
      if (resize_frame) {
        resize_frame->setVisible(false);
      }
    }
  }

  return QGraphicsItem::itemChange(change, value);
}

// Resize Frame base class
ResizeRotateFrame::ResizeRotateFrame(prim::ResizeRotateRect *resize_target)
  : Item(prim::Item::ResizeRotateFrame), resize_target(resize_target)
{
  if (border_width == -1)
    prepareStatics();

  if (resize_target) {
    setParentItem(resize_target);
    // setFlag(ItemStacksBehindParent, true);
  }
  // create a set of handles
  for (HandlePosition handle_pos : handle_positions) {
    prim::ResizeRotateHandle *handle = new prim::ResizeRotateHandle(handle_pos, this);
    resize_handles.append(handle);
  }

  // Graphics
}


void ResizeRotateFrame::setResizeTarget(prim::ResizeRotateRect *new_target)
{
  resize_target = new_target;
  setParentItem(resize_target);

  if (resize_target)
    updateHandlePositions();
}


void ResizeRotateFrame::resizeTargetToHandle(const HandlePosition &pos,
    const QPointF &delta)
{
  switch (pos) {
    case TopLeft:
      resizeTarget()->resize(delta.x(), delta.y(), 0, 0);
      break;
    case Top:
      resizeTarget()->resize(0, delta.y(), 0, 0);
      break;
    case TopRight:
      resizeTarget()->resize(0, delta.y(), delta.x(), 0);
      break;
    case Right:
      resizeTarget()->resize(0, 0, delta.x(), 0);
      break;
    case BottomRight:
      resizeTarget()->resize(0, 0, delta.x(), delta.y());
      break;
    case Bottom:
      resizeTarget()->resize(0, 0, 0, delta.y());
      break;
    case BottomLeft:
      resizeTarget()->resize(delta.x(), 0, 0, delta.y());
      break;
    case Left:
      resizeTarget()->resize(delta.x(), 0, 0, 0);
      break;
    default:
      qCritical() << "Trying to access a non-existent resize handle position";
      break;
  }
  updateHandlePositions();
}


void ResizeRotateFrame::updateHandlePositions()
{
  for (prim::ResizeRotateHandle *handle : resize_handles)
    handle->updatePosition();
}


void ResizeRotateFrame::paint(QPainter *, const QStyleOptionGraphicsItem*,
    QWidget*)
{
  // TODO draw a rectangular border
}


void ResizeRotateFrame::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  switch(e->buttons()) {
    default:
      prim::Item::mousePressEvent(e);
      break;
  }
}

// Initialize static class variables
void ResizeRotateFrame::prepareStatics()
{
  handle_positions.append(QList<HandlePosition>({TopLeft, Top, TopRight, Right,
      BottomRight, Bottom, BottomLeft, Left}));
  // TODO graphics
}





// ResizeRotateHandle class

ResizeRotateHandle::ResizeRotateHandle(prim::ResizeRotateFrame::HandlePosition handle_pos,
    QGraphicsItem *parent)
  : Item(prim::Item::ResizeRotateHandle, -1, parent), handle_position(handle_pos)
{
  if (handle_dim == -1)
    prepareStatics();
  updatePosition();

  switch (handle_position) {
    case prim::ResizeRotateFrame::TopLeft:
    case prim::ResizeRotateFrame::BottomRight:
      setCursor(Qt::SizeFDiagCursor);
      break;
    case prim::ResizeRotateFrame::TopRight:
    case prim::ResizeRotateFrame::BottomLeft:
      setCursor(Qt::SizeBDiagCursor);
      break;
    case prim::ResizeRotateFrame::Top:
    case prim::ResizeRotateFrame::Bottom:
      setCursor(Qt::SizeVerCursor);
      break;
    case prim::ResizeRotateFrame::Left:
    case prim::ResizeRotateFrame::Right:
      setCursor(Qt::SizeHorCursor);
      break;
    default:
      qCritical() << "Trying to access a non-existent resize handle position";
      break;
  }

  // Graphics
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setFlag(QGraphicsItem::ItemIsMovable, true);
}

void ResizeRotateHandle::updatePosition()
{
  QRectF p_rect = parentItem()->boundingRect();
  switch (handle_position) {
    case prim::ResizeRotateFrame::TopLeft:
      setPos(p_rect.topLeft());
      break;
    case prim::ResizeRotateFrame::Top:
      setPos(QPointF((p_rect.left()+p_rect.right())/2, p_rect.top()));
      break;
    case prim::ResizeRotateFrame::TopRight:
      setPos(p_rect.topRight());
      break;
    case prim::ResizeRotateFrame::Right:
      setPos(QPointF(p_rect.right(), (p_rect.top()+p_rect.bottom())/2));
      break;
    case prim::ResizeRotateFrame::BottomRight:
      setPos(p_rect.bottomRight());
      break;
    case prim::ResizeRotateFrame::Bottom:
      setPos(QPointF((p_rect.left()+p_rect.right())/2, p_rect.bottom()));
      break;
    case prim::ResizeRotateFrame::BottomLeft:
      setPos(p_rect.bottomLeft());
      break;
    case prim::ResizeRotateFrame::Left:
      setPos(QPointF(p_rect.left(), (p_rect.top()+p_rect.bottom())/2));
      break;
    default:
      qCritical() << "Trying to access a non-existent resize handle position";
      break;
  }
  update();
}

QRectF ResizeRotateHandle::boundingRect() const
{
  return QRectF(-.5*handle_dim, -.5*handle_dim, handle_dim, handle_dim);
}

void ResizeRotateHandle::paint(QPainter *painter,
    const QStyleOptionGraphicsItem *, QWidget *)
{
  QRectF rect = boundingRect();
  // TODO use static parameters for pen
  painter->setPen(QPen(QColor(0,0,0), 1));
  painter->setBrush(QColor(255,255,255));
  painter->drawRect(rect);
}

void ResizeRotateHandle::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  switch(e->buttons()) {
    case Qt::LeftButton:
    {
      prim::ResizeRotateRect *target = static_cast<prim::ResizeRotateFrame*>(parentItem())->
          resizeTarget();
      if (target) {
        target->preResize();

        clicked = true;
        step_pos = e->scenePos();

        // emit a signal informing design panel of the new mode
        emit prim::Emitter::instance()->sig_resizeBegin();
        e->accept();
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

void ResizeRotateHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
  if (clicked) {
    QPointF step_delta = e->scenePos() - step_pos;
    setPos(pos()+step_delta);
    static_cast<prim::ResizeRotateFrame*>(parentItem())->
        resizeTargetToHandle(handle_position, step_delta);
    step_pos = e->scenePos();
    e->accept();
  }
}

void ResizeRotateHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
  if (clicked) {
    prim::ResizeRotateRect *target = static_cast<prim::ResizeRotateFrame*>(parentItem())->
        resizeTarget();
    emit prim::Emitter::instance()->sig_resizeFinalizeRect(target,
        target->sceneRectCached(), target->sceneRect());
  }
  clicked = false;
}

void ResizeRotateHandle::prepareStatics()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  handle_dim = gui_settings->get<qreal>("resizablerect/handle_dim");
}

} // end of prim namespace
