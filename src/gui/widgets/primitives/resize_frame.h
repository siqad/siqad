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

  // Forward declaration of ResizeHandle class
  class ResizeHandle;

  //! A rectangular frame containing a few square handles for users to drag to
  //! resize graphics items. Always created in the "Control" layer which
  //! displays items intended purely for user control, never exported nor saved.
  class ResizeFrame : public Item
  {
  public:

    //! Positions of resize handles which allows users to resize the target item
    //! by dragging.
    enum HandlePosition{TopLeft, Top, TopRight, Right, BottomRight, Bottom,
        BottomLeft, Left};

    //! Constructor which takes the "Control" layer id and a pointer to the
    //! target item that this frame will resize.
    ResizeFrame(int lay_id, prim::Item *resize_target=0);

    //! Empty destructor.
    ~ResizeFrame() {};

    //! Set the target resize item.
    void setResizeTarget(prim::Item *new_target);
    prim::Item *resizeTarget() const {return resize_target;}

    //! Retrieve the handle at the indicated location
    prim::ResizeHandle *handle(HandlePosition pos) {return resize_handles.at(pos);}

    // Graphics

    //! Bounding rect for graphics calculations, just takes the resize_target's.
    virtual QRectF boundingRect() const {return resize_target->boundingRect();}

    //! Paint function.
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem*, QWidget*);

    //! This object is never copied.
    virtual Item *deepCopy() const {};

  protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *) Q_DECL_OVERRIDE;

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
    ResizeHandle(int lay_id, prim::ResizeFrame::HandlePosition handle_pos,
        QGraphicsItem *parent);

    //! Update the position of this handle when a new target has been set or
    //! when the object is resized.
    void updatePosition();

    //! Bounding rect of the handle for users to grab.
    virtual QRectF boundingRect() const;

    //! Paint a square indicating where users should grab for resize.
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem*, QWidget*);

    //! This object is never copied.
    virtual Item *deepCopy() const {};

  protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *) Q_DECL_OVERRIDE;

  private:
    //! Initialize static class variables.
    void prepareStatics();
    
    prim::ResizeFrame::HandlePosition handle_position; //! The position of this handle.

    static qreal handle_dim;
    static prim::Item::StateColors handle_col;
  }; // end of ResizeHandle class

} // end of prim namespace



#endif
