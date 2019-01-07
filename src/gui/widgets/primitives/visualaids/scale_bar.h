/** @file:     scale_bar.h
 *  @author:   Samuel
 *  @created:  2019.01.06
 *  @editted:  2019.01.06  - Samuel
 *  @license:  GNU LGPL v3
 *
 *  @brief:    Scale bar.
 */

#ifndef _GUI_PR_SCALE_BAR_H_
#define _GUI_PR_SCALE_BAR_H_


#include <QtWidgets>
#include "../item.h"
#include "../../../../global.h"

namespace prim{

  class ScaleBar: public prim::Item
  {
  public:

    // NOTE at this time, the scale bar is only meant for screenshots. If future
    // needs change and this has to be added to layers, make the appropriate
    // additions in the constructor.

    //! Construct a scale bar with the given length and unit. If the length is
    //! less than 0 then the scale bar won't be drawn. The position defines the
    //! location of the top left corner of the scale bar.
    //! No layer is needed since this is will appear in the scene directly.
    ScaleBar(int t_layer_id, float t_bar_length=-1, 
             gui::Unit::DistanceUnit t_bar_unit=gui::Unit::nm,
             QPointF scene_pos=QPointF());

    //! Destructor.
    ~ScaleBar() {};

    // accessors

    //! Set the scale bar length and unit.
    void setScaleBar(float t_bar_length, gui::Unit::DistanceUnit t_bar_unit);

    //! Set the scene position.
    void setScenePos(const QPointF &scene_pos);

    //! Overridden paint function.
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;

  protected:

    virtual QRectF boundingRect() const override;


  private:

    //! Construct static variables.
    void constructStatics();

    // VARIABLES
    float bar_length;
    gui::Unit::DistanceUnit bar_unit;
    float bar_length_px;

    // static class parameters for painting
    static qreal bar_thickness;   // bar width in angstrom (in the future, make this independent of zoom)
    static qreal text_height; // text height in angstrom
    static QColor bar_col;    // color of the scale bar and text

  };

} // end prim namespace



#endif
