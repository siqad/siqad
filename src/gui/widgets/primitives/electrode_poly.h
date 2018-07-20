/** @file:     electrode_poly.h
 *  @author:   Nathan
 *  @created:  2017.10.27
 *  @editted:  2018.01.17 - Nathan
 *  @license:  GNU LGPL v3
 *
 *  @brief:     Function prototypes for the ElectrodePoly object.
 */

#ifndef _GUI_PR_ELECTRODE_POLY_H_
#define _GUI_PR_ELECTRODE_POLY_H_


#include <QtWidgets>
#include "resizablerect.h"

namespace prim{

  class ElectrodePoly: public Item
  {
  public:
    ElectrodePoly(const QPolygonF, const QRectF, int lay_id);
    void test();
    QPolygonF getPolygon() const {return poly;}
    void setSceneRect(const QRectF &scene_rect_in){scene_rect = scene_rect_in;}
    QRectF sceneRect() const {return scene_rect;}

    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;
    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;
    virtual Item *deepCopy() const override;

  private:
    void constructStatics();
    void initElectrodePoly(int lay_id);

    QRectF scene_rect;
    QPolygonF poly;
    static qreal edge_width;  // proportional width of dot boundary edge
    static QColor fill_col;   // dot fill color (same for all lattice dots)
    static QColor edge_col;     // edge colour, unselected
    static QColor selected_col; // edge colour, selected
  };

} //end prim namespace

#endif
