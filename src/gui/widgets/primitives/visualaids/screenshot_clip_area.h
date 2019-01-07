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
#include "../item.h"

namespace prim{

  class ScreenshotClipArea: public prim::Item
  {
  public:

    //! Construct a screenshot clip area with the given QRectF in physical
    //! coordinates.
    ScreenshotClipArea(int t_layer_id, QRectF t_scene_rect=QRectF());

    // TODO XML version for future implementation of saving and loading screenshot
    // clip areas.

    //! Destructor.
    ~ScreenshotClipArea() {};


    // accessors

    //! Set the screenshot clip area in scene coordinates.
    void setSceneRect(QRectF t_scene_rect);

    //! Return the scene_rect.
    QRectF sceneRect() {return scene_rect;}

    //! Overridden paint function. Unlike most other items, the screenshot clip
    //! area preview's border does not overlap with the rect area.
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;

  protected:

    //! Return the scene rect plus border.
    virtual QRectF boundingRect() const override;

    //virtual void mousePressEvent(QGraphicsSceneMouseEvent *e) override;


  private:

    //! Construct static variables.
    void constructStatics();

    // VARIABLES
    QRectF scene_rect;

    // static class parameters for painting
    static qreal edge_width;    // edge width in angstrom (in the future, make this independent of zoom)
    //static prim::Item::StateColors fill_col;
    static QColor edge_col;

  };

} // end prim namespace



#endif
