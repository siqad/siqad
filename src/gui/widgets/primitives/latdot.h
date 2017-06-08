// @file:     latdot.h
// @author:   Jake
// @created:  2017.05.03
// @editted:  2017.05.03  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Base graphical item for possible dangling bond locations on the
//            surface lattice.


#ifndef _GUI_PR_LATDOT_H_
#define _GUI_PR_LATDOT_H_


#include "item.h"

namespace prim{

  // forard declarations
  class Layer;
  class DBDot;

  // Specific Item derived class for showing the possible dangling bond
  // location on the surface lattice. For now, this class has very similar
  // characteristics to the DBDot but will be kept a separate class to allow for
  // future distinction in properties and avoid overbloating the functionality of
  // either class.
  class LatticeDot: public Item
  {
  public:

    // constructor, create a lattice dot at the given location in physical units
    LatticeDot(prim::Layer *layer, QPointF p_loc);

    // destructor
    ~LatticeDot(){}

    // accessors

    QPointF getPhysLoc() const {return phys_loc;}   // get the dots physical location
    prim::DBDot *getDBDot() const {return dbdot;}   // get the created dangling bond
    void setDBDot(prim::DBDot *dot=0);              // set the created dangling bond

    // inherited abstract method implementations

    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;

    Item *deepCopy() const;

  private:

    // construct static variables
    void constructStatics();


    // VARIABLES

    QPointF phys_loc;   // physical location of the dot in angstroms
    prim::DBDot *dbdot; // dangling bond at the lattice dot

    // static class parameters for painting

    static qreal diameter;    // dot diameter in angstroms
    static qreal edge_width;  // proportional width of dot boundary edge

    static QColor edge_col;   // boundary edge color
    static QColor fill_col;   // dot fill color (same for all lattice dots)
  };

} // end prim namespace

#endif
