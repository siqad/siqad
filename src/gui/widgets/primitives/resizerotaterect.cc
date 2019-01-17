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
QColor ResizeRotateHandle::fill_col;
QColor ResizeRotateHandle::selected_col; // edge colour, selected


// Resizable Rectangle base class
ResizeRotateRect::ResizeRotateRect(ItemType type, const QRectF &scene_rect, int lay_id,
                             QGraphicsItem *parent)
  : Item(type, lay_id, parent)
{
  setResizable(true);
  setSceneRect(scene_rect);
}

void ResizeRotateRect::resize(qreal dx1, qreal dy1, qreal dx2, qreal dy2, bool update_handles)
{
  prepareGeometryChange();

  // update dimensions
  scene_rect.setTopLeft(scene_rect.topLeft()+QPointF(dx1,dy1));
  scene_rect.setBottomRight(scene_rect.bottomRight()+QPointF(dx2,dy2));
  setPos(scene_rect.topLeft());
  updatePolygon();
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

void ResizeRotateRect::setTransform(QTransform t)
{
  prepareGeometryChange();
  transform = t;
  update();
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

void ResizeRotateRect::updatePolygon()
{
  QRectF rect = sceneRect();
  rect.moveTo(0,0);
  QPolygonF poly(rect);
  polygon_cache = getTransform().map(poly);
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

void ResizeRotateRect::setAngleDegrees(qreal angle_in)
{
  angle = angle_in;
  //reduce the angle to between 0 and 180. Bounds OK for rectangular.
  while (angle >= 180)
    angle -= 180;
  while (angle < 0)
    angle += 180;
}

void ResizeRotateRect::setRotation(qreal angle_in)
{
  prepareGeometryChange();
  setAngleDegrees(angle_in); //set the angle variable
  QTransform t; //update the transform.
  t.translate(sceneRect().width()*0.5, sceneRect().height()*0.5);
  t.rotate(getAngleDegrees());
  t.translate(-sceneRect().width()*0.5, -sceneRect().height()*0.5);
  setTransform(t);
  updatePolygon(); //update polygon cache
  if (resize_frame)
    resize_frame->updateHandlePositions();
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

QPointF ResizeRotateFrame::getUnitPoint(HandlePosition pos)
{
  QTransform t;
  t.rotate(resizeTarget()->getAngleDegrees());
  QPointF unit;
  switch (pos) {
    case TopLeft:
      unit = QPointF();
      break;
    case Top:
      unit = QPointF(0,1);
      break;
    case TopRight:
      unit = QPointF();
      break;
    case Right:
      unit = QPointF(1,0);
      break;
    case BottomRight:
      unit = QPointF();
      break;
    case Bottom:
      unit = QPointF(0,1);
      break;
    case BottomLeft:
      unit = QPointF();
      break;
    case Left:
      unit = QPointF(1,0);
      break;
    default:
      qCritical() << "Trying to access a non-existent resize handle position";
      break;
  }
  return t.map(unit);
}

void ResizeRotateFrame::resizeTargetToHandle(const HandlePosition &pos,
    const QPointF &delta)
{
  //find the angle of the rect
  QPointF unit = getUnitPoint(pos);
  //calculate the dot product
  qreal dot = QPointF::dotProduct(unit, delta/delta.manhattanLength());
  //check if nan
  if (dot != dot)
    dot = 0;
  qreal cos_scale = qCos(resizeTarget()->getAngleRadians());
  qreal sin_scale = qSin(resizeTarget()->getAngleRadians());
  QPointF old_anchor, new_anchor;
  QPointF top, right;
  switch (pos) {
    case TopLeft:
      old_anchor = resize_handles[BottomRight]->scenePos();
      resizeTarget()->resize(cos_scale*delta.x()+sin_scale*delta.y(), -sin_scale*delta.x()+cos_scale*delta.y(), 0, 0);
      break;
    case Top:
      old_anchor = resize_handles[Bottom]->scenePos();
      resizeTarget()->resize(0, dot*delta.manhattanLength(), 0, 0);
      break;
    case TopRight:
      old_anchor = resize_handles[BottomLeft]->scenePos();
      resizeTarget()->resize(0, cos_scale*delta.y()-sin_scale*delta.x(), cos_scale*delta.x()+sin_scale*delta.y(), 0);
      break;
    case Right:
      old_anchor = resize_handles[Left]->scenePos();
      resizeTarget()->resize(-dot*delta.manhattanLength(), 0, 0, 0);
      break;
    case BottomRight:
      old_anchor = resize_handles[TopLeft]->scenePos();
      resizeTarget()->resize(0, 0, cos_scale*delta.x()+sin_scale*delta.y(), cos_scale*delta.y()-sin_scale*delta.x());
      break;
    case Bottom:
      old_anchor = resize_handles[Top]->scenePos();
      resizeTarget()->resize(0, -dot*delta.manhattanLength(), 0, 0);
      break;
    case BottomLeft:
      old_anchor = resize_handles[TopRight]->scenePos();
      resizeTarget()->resize(cos_scale*delta.x()+sin_scale*delta.y(), 0, 0, cos_scale*delta.y()-sin_scale*delta.x());
      break;
    case Left:
      old_anchor = resize_handles[BottomRight]->scenePos();
      resizeTarget()->resize(dot*delta.manhattanLength(), 0, 0, 0);
      break;
    default:
      qCritical() << "Trying to access a non-existent resize handle position";
      break;
  }
  updateHandlePositions();
  fixOffset(pos, old_anchor);
}

void ResizeRotateFrame::fixOffset(HandlePosition pos, QPointF old_anchor)
{
  QPointF new_anchor;
  QTransform t;
  t.rotate(resizeTarget()->getAngleDegrees());
  QRectF rect = resizeTarget()->sceneRect();
  switch (pos) {
    case TopLeft:
      new_anchor = resize_handles[BottomRight]->scenePos();
      rect.translate(old_anchor - new_anchor);
      break;
    case Top:
      new_anchor = resize_handles[Bottom]->scenePos();
      rect.translate(old_anchor - new_anchor);
      break;
    case TopRight:
      new_anchor = resize_handles[BottomLeft]->scenePos();
      rect.translate(old_anchor - new_anchor);
      break;
    case Right:
      new_anchor = resize_handles[Left]->scenePos();
      rect.translate(old_anchor - new_anchor);
      break;
    case BottomRight:
      new_anchor = resize_handles[TopLeft]->scenePos();
      rect.translate(old_anchor - new_anchor);
      break;
    case Bottom:
      new_anchor = resize_handles[Top]->scenePos();
      rect.translate(old_anchor - new_anchor);
      break;
    case BottomLeft:
      new_anchor = resize_handles[TopRight]->scenePos();
      rect.translate(old_anchor - new_anchor);
      break;
    case Left:
      new_anchor = resize_handles[BottomRight]->scenePos();
      rect.translate(old_anchor - new_anchor);
      break;
    default:
      qCritical() << "Trying to access a non-existent resize handle position";
      break;
  }
  resizeTarget()->setSceneRect(rect);
}


void ResizeRotateFrame::updateHandlePositions()
{
  for (prim::ResizeRotateHandle *handle : resize_handles){
    handle->updatePosition();
  }
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
      // setCursor(Qt::SizeFDiagCursor);
      break;
    case prim::ResizeRotateFrame::TopRight:
    case prim::ResizeRotateFrame::BottomLeft:
      // setCursor(Qt::SizeBDiagCursor);
      break;
    case prim::ResizeRotateFrame::Top:
    case prim::ResizeRotateFrame::Bottom:
      // setCursor(Qt::SizeVerCursor);
      break;
    case prim::ResizeRotateFrame::Left:
    case prim::ResizeRotateFrame::Right:
      // setCursor(Qt::SizeHorCursor);
      break;
    default:
      qCritical() << "Trying to access a non-existent resize handle position";
      break;
  }

  // Graphics
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setFlag(QGraphicsItem::ItemIsMovable, true);
  setAcceptHoverEvents(true);
}


void ResizeRotateHandle::updatePosition()
{
  QTransform t = static_cast<prim::ResizeRotateFrame*>(parentItem())->resizeTarget()->getTransform();
  QRectF p_rect = static_cast<prim::ResizeRotateFrame*>(parentItem())->resizeTarget()->sceneRect();
  p_rect.moveTo(0,0);
  switch (handle_position) {
    case prim::ResizeRotateFrame::TopLeft:
      setPos(t.map(p_rect.topLeft()));
      break;
    case prim::ResizeRotateFrame::Top:
      setPos(t.map(QPointF((p_rect.left()+p_rect.right())/2, p_rect.top())));
      break;
    case prim::ResizeRotateFrame::TopRight:
      setPos(t.map(p_rect.topRight()));
      break;
    case prim::ResizeRotateFrame::Right:
      setPos(t.map(QPointF(p_rect.right(), (p_rect.top()+p_rect.bottom())/2)));
      break;
    case prim::ResizeRotateFrame::BottomRight:
      setPos(t.map(p_rect.bottomRight()));
      break;
    case prim::ResizeRotateFrame::Bottom:
      setPos(t.map(QPointF((p_rect.left()+p_rect.right())/2, p_rect.bottom())));
      break;
    case prim::ResizeRotateFrame::BottomLeft:
      setPos(t.map(p_rect.bottomLeft()));
      break;
    case prim::ResizeRotateFrame::Left:
      setPos(t.map(QPointF(p_rect.left(), (p_rect.top()+p_rect.bottom())/2)));
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
  painter->setBrush(fill_col);
  if(isHovered())
    painter->setBrush(selected_col);
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

void ResizeRotateHandle::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
  setHovered(true);
  update();
}

void ResizeRotateHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
  setHovered(false);
  update();
}

void ResizeRotateHandle::prepareStatics()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  handle_dim = gui_settings->get<qreal>("resizerotaterect/handle_dim");
  selected_col= gui_settings->get<QColor>("resizerotaterect/selected_col");
  fill_col= gui_settings->get<QColor>("resizerotaterect/fill_col");

}

} // end of prim namespace
