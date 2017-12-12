// @file:     electrode.h
// @author:   Nathan
// @created:  2017.10.27
// @editted:  2017.10.27 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     Electrode objects for creation, deletion, and moving of electrodes.

#ifndef _GUI_PR_ELECTRODE_H_
#define _GUI_PR_ELECTRODE_H_

#include <QtWidgets>
#include "item.h"

namespace prim{

  // forard declarations
  class Layer;

  class Electrode: public prim::Item
  {
  public:

    // constructor, create an electrode given two points
    Electrode(int lay_id, QPointF point1, QPointF point2);
    Electrode(QXmlStreamReader *ls, QGraphicsScene *scene);
    void initElectrode(int lay_id, QPointF point1_in, QPointF point2_in);


    // destructor
    ~Electrode(){}

    //Electrodes will have two types, clocked (that will change over time) and fixed
    enum ElectrodeType{Clock, Fix};

    ElectrodeType electrode_type;

    //initializer
    void initElectrode(int lay_id, QPointF point1_in, QPointF point2_in, double potential_in=0, int electrode_type_in=0);

    //setters
    void setPotential(double givenPotential);

    // accessors
    QPointF getPoint1(void){ return point1;}
    QPointF getPoint2(void){ return point2;}
    QPointF getTopLeft(void){return top_left;}
    qreal getWidth(void){return elec_width;}
    qreal getHeight(void){return elec_height;}
    double getPotential(void) const {return potential;}
    // inherited abstract method implementations

    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;

    Item *deepCopy() const;

    // SAVE LOAD
    virtual void saveItems(QXmlStreamWriter *) const;
  //
  protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;

  private:

    // construct static variables
    void constructStatics();

    // VARIABLES
    QPointF point1;
    QPointF point2;
    QPointF top_left;

    double potential = 0;
    qreal elec_width;
    qreal elec_height;

    static qreal edge_width;  // proportional width of dot boundary edge
    static QColor fill_col;   // dot fill color (same for all lattice dots)
    static QColor edge_col;     // edge colour, unselected
    static QColor selected_col; // edge colour, selected
  };

} // end prim namespace

#endif
//////
