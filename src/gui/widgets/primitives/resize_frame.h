// @file:     resize_frame.h
// @author:   Samuel
// @created:  2018-03-18
// @editted:  2018-03-18 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Resize handles for resizable objects

#ifndef _PRIM_RESIZE_FRAME_H_
#define _PRIM_RESIZE_FRAME_H_

#include "item.h"

namespace prim{

  // Forward declarations
  class ResizeFrame;
  class ResizeHandle;

  //! A prim::Item that inherits this class will get resizing abilities.
  class ResizableRect : public Item
  {
  public:
    //! Constructor that takes in the rectangle dimensions in scene coordinates.
    ResizableRect(QRectF scene_rect, ItemType type, int lay_id=-1,
                  QGraphicsItem *parent=0);

    //! Move the top left and bottom right corners of the rectangle by the given
    //! deltas.
    virtual void resize(qreal dx1, qreal dy1, qreal dx2, qreal dy2, bool update_handles=false) override;

    //! Pre-resize actions - save the original position and dimensions.
    void preResize();

    // Public variables
    QRectF scene_rect;        // the rectangle dimensions in scene coordinates

  protected:

    //! Show resize frame when focused
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

  private:
    // Variables
    prim::ResizeFrame *resize_frame=0;  // the resize frame for this resizble rect
    QRectF scene_rect_cache;            // the rectangle dimensions before resize
    QPointF pos_cache;                  // the top left point before resize
  };

  //! A rectangular frame containing a few square handles for users to drag to
  //! resize graphics items. The frame sees the target item as its parent item.
  class ResizeFrame : public Item
  {
  public:

    //! Positions of resize handles which allows users to resize the target item
    //! by dragging.
    enum HandlePosition{TopLeft, Top, TopRight, Right, BottomRight, Bottom,
        BottomLeft, Left};

    //! Constructor which takes the pointer to the target item that this frame
    //! will resize.
    ResizeFrame(prim::Item *resize_target=0);

    //! Empty destructor.
    ~ResizeFrame() {};

    //! Set the target resize item.
    void setResizeTarget(prim::Item *new_target);
    prim::Item *resizeTarget() const {return resize_target;}

    //! Retrieve the handle at the indicated location
    prim::ResizeHandle *handle(HandlePosition pos) {return resize_handles.at(pos);}

    //! Update the resize target with the resize handle's new position, and
    //! update the positions of the rest of the handles.
    void resizeTargetToHandle(const HandlePosition &pos, const QPointF &delta);

    //! Update the position of all handles.
    void updateHandlePositions();

    // Graphics

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
    prim::Item *resize_target=0;

    //! Static list of handle positions for easy iteration through all positions
    static QList<HandlePosition> handle_positions;

    //! List of resize handles, the index is the HandlePosition.
    QList<prim::ResizeHandle*> resize_handles;

    // Graphics
    static qreal border_width;

  }; // end of ResizeFrame class



  //! The ResizeHandle class provides square boxes which allows users to
  //! resize target item by dragging.
  class ResizeHandle : public Item
  {
  public:
    //! Initialize the handle with the given handle position.
    ResizeHandle(prim::ResizeFrame::HandlePosition handle_pos,
        QGraphicsItem *parent);

    //! Update the position of this handle when a new target has been set or
    //! when the object is resized.
    void updatePosition();

    //! Bounding rect of the handle for users to grab.
    virtual QRectF boundingRect() const override;

    //! Paint a square indicating where users should grab for resize.
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem*, QWidget*) override;

  protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override;

  private:
    //! Initialize static class variables.
    void prepareStatics();

    prim::ResizeFrame::HandlePosition handle_position; //! The position of this handle.

    static qreal handle_dim;
    static prim::Item::StateColors handle_col;

    bool clicked;
    QPointF step_pos;   // cursor location at the last mouse move event
  }; // end of ResizeHandle class

} // end of prim namespace



#endif
