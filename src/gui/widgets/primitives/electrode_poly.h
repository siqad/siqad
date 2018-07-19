/** @file:     electrode_poly.h
 *  @author:   Nathan
 *  @created:  2017.10.27
 *  @editted:  2018.01.17 - Nathan
 *  @license:  GNU LGPL v3
 *
 *  @brief:     Function prototypes for the ElectrodePoly object.
 */

#include <QtWidgets>
#include "resizablerect.h"

namespace prim{

  class ElectrodePoly: public Item
  {
  public:
    ElectrodePoly(const QPolygonF, const QRectF);
    void test();
    QPolygonF getPolygon(){return poly;}
    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;
    QRectF sceneRect(){return scene_rect;}
    void setSceneRect(QRectF scene_rect_in){scene_rect = scene_rect_in;}

  private:
    void constructStatics();
    void initElectrodePoly();

    QRectF scene_rect;
    QPolygonF poly;
    static qreal edge_width;  // proportional width of dot boundary edge
    static QColor fill_col;   // dot fill color (same for all lattice dots)
    static QColor edge_col;     // edge colour, unselected
    static QColor selected_col; // edge colour, selected
  };

} //end prim namespace
