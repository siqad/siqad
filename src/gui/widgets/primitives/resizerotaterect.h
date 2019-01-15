// @file:     ResizeRotateRect.h
// @author:   Samuel
// @created:  2018-03-18
// @editted:  2018-03-18 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Resize handles for resizable objects

#ifndef _PRIM_RESIZE_ROTATE_FRAME_H_
#define _PRIM_RESIZE_ROTATE_FRAME_H_

#include<QTransform>
#include<QtMath>
#include "item.h"

namespace prim{

  // Forward declarations
  class ResizeRotateFrame;
  class ResizeRotateHandle;

  //! A prim::Item that inherits this class will get resizing abilities.
  class ResizeRotateRect : public Item
  {
  public:
    //! Constructor that takes in the rectangle dimensions in scene coordinates.
    ResizeRotateRect(ItemType type, const QRectF &scene_rect=QRectF(), int lay_id=-1,
                  QGraphicsItem *parent=0);

    //! Move the top left and bottom right corners of the rectangle by the given
    //! deltas. Override this function if a custom resize behavior is needed.
    virtual void resize(qreal dx1, qreal dy1, qreal dx2, qreal dy2, bool update_handles=false);

    //! Pre-resize actions - save the original position and dimensions.
    void preResize();

    //! Move the top left of the rectangle by the given delta.
    virtual void moveItemBy(qreal dx, qreal dy) override;

    //! Set rectangle that defines this item's dimensions in scene coordinates.
    void setSceneRect(const QRectF &rect);

    //! Return the rectangle that defines this item's dimensions in scene coordinates.
    QRectF sceneRect() const {return scene_rect;}

    //! Return the item's dimensions before resizing in scene coordinates.
    QRectF sceneRectCached() const {return scene_rect_cache;}

    //! Return the item's transform
    QTransform* getTransform() const {return transform;}

    void setTransform(QTransform* t) {transform = t;}

    prim::ResizeRotateFrame* getResizeFrame();

  protected:

    //! Show resize frame when focused
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

  private:
    // Variables
    prim::ResizeRotateFrame *resize_frame=0;  // the resize frame for this resizble rect
    QTransform *transform=0;             // the transform applied to the ResizeRotateRect
    QRectF scene_rect;        // the rectangle dimensions in scene coordinates
    QRectF scene_rect_cache;            // the rectangle dimensions before resize
    QPointF pos_cache;                  // the top left point before resize
  };

  //! A rectangular frame containing a few square handles for users to drag to
  //! resize graphics items. The frame sees the target item as its parent item.
  class ResizeRotateFrame : public Item
  {
  public:

    //! Positions of resize handles which allows users to resize the target item
    //! by dragging.
    enum HandlePosition{TopLeft, Top, TopRight, Right, BottomRight, Bottom,
        BottomLeft, Left};

    //! Constructor which takes the pointer to the target item that this frame
    //! will resize.
    ResizeRotateFrame(prim::ResizeRotateRect *resize_target=0);

    //! Empty destructor.
    ~ResizeRotateFrame() {};

    //! Set the target resize item.
    void setResizeTarget(prim::ResizeRotateRect *new_target);
    prim::ResizeRotateRect *resizeTarget() const {return resize_target;}

    //! Retrieve the handle at the indicated location
    prim::ResizeRotateHandle *handle(HandlePosition pos) {return resize_handles.at(pos);}

    //! Update the resize target with the resize handle's new position, and
    //! update the positions of the rest of the handles.
    void resizeTargetToHandle(const HandlePosition &pos, const QPointF &delta);

    //! Update the position of all handles.
    void updateHandlePositions();

    //! Fixes the location of the rect after resizing due to offsets from the rotational transformation.
    void fixOffset(HandlePosition pos, QPointF delta_c);

    // Graphics
    //! Will freeze the handles that need to be fixed during resizing.
    void freezeHandles(HandlePosition pos);

    //! returns a QPointF for use as a unit vector, relevant to the handle chosen.
    QPointF getUnitPoint(HandlePosition pos, qreal angle);

    //! returns the angle of the rotated rect, bounded between 0 and 180 degrees
    qreal getAngleDegrees();

    //! returns the angle of the rotated rect, bounded between 0 and pi.
    qreal getAngleRadians(){return qDegreesToRadians(getAngleDegrees());}

    //! Bounding rect for graphics calculations, just takes the resize_target's.
    virtual QRectF boundingRect() const override {return resize_target->boundingRect();}

    //! Paint function.
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem*, QWidget*) override;

  protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *) override;

  private:

    //! Initialize static class variables.
    void prepareStatics();

    // resize frame variables
    prim::ResizeRotateRect *resize_target=0;

    //! Static list of handle positions for easy iteration through all positions
    static QList<HandlePosition> handle_positions;

    //! List of resize handles, the index is the HandlePosition.
    QList<prim::ResizeRotateHandle*> resize_handles;

    // Graphics
    static qreal border_width;

  }; // end of ResizeRotateFrame class



  //! The ResizeRotateHandle class provides square boxes which allows users to
  //! resize target item by dragging.
  class ResizeRotateHandle : public Item
  {
  public:
    //! Initialize the handle with the given handle position.
    ResizeRotateHandle(prim::ResizeRotateFrame::HandlePosition handle_pos,
        QGraphicsItem *parent);

    //! Update the position of this handle when a new target has been set or
    //! when the object is resized.
    void updatePosition();

    //! Bounding rect of the handle for users to grab.
    virtual QRectF boundingRect() const override;

    void setFreeze(bool freeze){frozen = freeze;}

    bool isFrozen(){return frozen;}
    //! Paint a square indicating where users should grab for resize.
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem*, QWidget*) override;

  protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override;

  private:
    //! Initialize static class variables.
    void prepareStatics();

    prim::ResizeRotateFrame::HandlePosition handle_position; //! The position of this handle.

    static qreal handle_dim;
    static prim::Item::StateColors rotate_handle_col;

    bool frozen = false;
    bool clicked;
    QPointF step_pos;   // cursor location at the last mouse move event
  }; // end of ResizeRotateHandle class

} // end of prim namespace



#endif
