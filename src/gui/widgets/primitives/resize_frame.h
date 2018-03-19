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

  //! A rectangular frame containing a few square handles for users to drag to
  //! resize graphics items. Always created in the "Control" layer which
  //! displays items intended purely for user control, never exported nor saved.
  class ResizeFrame : public Item
  {
  public:

    ResizeFrame(int lay_id, prim::Item *resize_target);
    ~ResizeFrame() {};

    class ResizeHandle
    {
      static qreal handle_dim;
      static prim::StateColors handle_col;
    };

  protected:


  private:
  };

} // end of prim namespace



#endif
