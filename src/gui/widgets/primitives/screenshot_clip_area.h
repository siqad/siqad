/** @file:     screenshot_clip_area.h
 *  @author:   Samuel
 *  @created:  2019.01.04
 *  @editted:  2019.01.04  - Samuel
 *  @license:  GNU LGPL v3
 *
 *  @brief:    Screenshot clip area preview.
 */

#ifndef _GUI_PR_SCREENSHOT_CLIP_AREA_H_
#define _GUI_PR_SCREENSHOT_CLIP_AREA_H_


#include <QtWidgets>
#include "item.h"

namespace prim{

  class ScreenshotClipArea: public prim::Item
  {
  public:

    ScreenshotClipArea();

    // destructor
    ~ScreenshotClipArea(){}

    // accessors

  protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *e) override;

  private:

    // construct static variables
    void constructStatics();

    // VARIABLES
    // TODO physical QRect

    // static class parameters for painting

    static prim::Item::StateColors fill_col;
    static prim::Item::StateColors edge_col;
    static qreal edge_width;    // proportional width of boundary edge

  };

} // end prim namespace



#endif
