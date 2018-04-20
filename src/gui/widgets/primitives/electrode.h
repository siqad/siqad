/** @file:     electrode.h
 *  @author:   Nathan
 *  @created:  2017.10.27
 *  @editted:  2018.01.17 - Nathan
 *  @license:  GNU LGPL v3
 *
 *  @brief:     Function prototypes for the Electrode object.
 */

#ifndef _GUI_PR_ELECTRODE_H_
#define _GUI_PR_ELECTRODE_H_

#include <QtWidgets>
#include "item.h"

namespace prim{

  // forward declarations
  class Layer;

  class Electrode: public prim::Item
  {
  public:
    //! constructor, creates an electrode given two points.
    Electrode(int lay_id, QPointF point1, QPointF point2);
    //! constructor, creates an electrode from the design file.
    Electrode(QXmlStreamReader *ls, QGraphicsScene *scene);
    //! destructor
    ~Electrode(){}
    //! Clock types will vary over time. Fix types are static.
    enum ElectrodeType{Clock, Fix};
    ElectrodeType electrode_type;
    // Initializer for initial attribute values.
    void initElectrode(int lay_id, QPointF point1_in, QPointF point2_in, double potential_in=0, int electrode_type_in=0);

    //! sets the electrode potential to givenPotential.
    void setPotential(double givenPotential);
    //! gets the electrode potential to givenPotential.
    double getPotential(void) const {return potential;}

    // accessors
    QPointF getPoint1(void){return point1;}
    QPointF getPoint2(void){return point2;}
    QPointF getTopLeft(void){return top_left;}
    qreal getTopDepth(void){return top_depth;}
    qreal getWidth(void) const {return std::max(point1.x(), point2.x()) - std::min(point1.x(), point2.x());}
    qreal getHeight(void) const {return std::max(point1.y(), point2.y()) - std::min(point1.y(), point2.y());}
    qreal getDepth(void){return elec_depth;}

    //! Updates the electrode with its new location. Call this after moving the electrode.
    void updatePoints(QPointF);

    // inherited abstract method implementations
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;
    Item *deepCopy() const override;

    // saving to design
    virtual void saveItems(QXmlStreamWriter *) const override;

  protected:

    // Mouse events
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;

  private:
    // construct static variables
    void constructStatics();

    // VARIABLES
    QPointF point1;
    QPointF point2;
    QPointF top_left; //top left point, since the two points given could be any two opposite points
    double potential = 0;
    qreal elec_depth;
    qreal top_depth;

    static qreal edge_width;  // proportional width of dot boundary edge
    static QColor fill_col;   // dot fill color (same for all lattice dots)
    static QColor edge_col;     // edge colour, unselected
    static QColor selected_col; // edge colour, selected
  };

} // end prim namespace

#endif
