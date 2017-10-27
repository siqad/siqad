// @file:     electrode.h
// @author:   Nathan
// @created:  2017.10.27
// @editted:  2017.10.27 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     Electrode objects for creation, deletion, and moving of electrodes.

#ifndef _GUI_PR_ELECTRODE_H_
#define _GUI_PR_ELECTRODE_H_


#include "item.h"

namespace prim{

  // forard declarations
  class Layer;

  // Specific Item derived class for showing the possible dangling bond
  // location on the surface lattice. For now, this class has very similar
  // characteristics to the DBDot but will be kept a separate class to allow for
  // future distinction in properties and avoid overbloating the functionality of
  // either class.
  class Electrode: public Item
  {
  public:

    // constructor, create an electrode given two points
    Electrode(QPoint p1, QPoint p2);
    // Electrode(QPoint p1, int width, int height); //or a QPoint point and an integer width and an integer height

    // destructor
    ~Electrode(){}

    // accessors

    QPoint getLoc() const {return p1;}   // get the pixel location of p1.

    // inherited abstract method implementations

    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;

    Item *deepCopy() const;

  private:

    // construct static variables
    void constructStatics();


    // VARIABLES

    QPoint p1;                // location of first point in pixels (REQUIRED)
    QPoint p2;                // location of second point in pixels

    // static class parameters for painting

    static QColor edge_col;   // boundary edge color
    static QColor fill_col;   // dot fill color (same for all lattice dots)

  };

} // end prim namespace

#endif
