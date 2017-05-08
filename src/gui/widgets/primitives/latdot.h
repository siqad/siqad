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


#include "items.h"
#include "emitter.h"

namespace prim{


// Specific MyItem derived class for showing the possible dangling bond
// location on the surface lattice. For now, this class has very similar
// characteristics to the DBDot but will be kept a separate class to allow for
// future distinction in properties and avoid overbloating the functionality of
// either class.

class LatticeDot: public prim::MyItem
{
public:

  // constructor, create a lattice dot at the given location in physical units
  LatticeDot(QPointF p_loc);

  // destructor
  ~LatticeDot();

  // accessors

  QPointF getPhysLoc(){return phys_loc;}

  // inherited abstract method implementations

  QRectF boundingRect() const;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);

private:

  // construct the statics
  void constructStatics();


  // VARIABLES

  QPointF phys_loc;   // physical location of the dot in angstroms

  // static class parameters for painting

  static qreal diameter;    // dot diameter in angstroms
  static qreal edge_width;  // proportional width of dot boudary edge

  static QColor edge_col;   // boundary edge color
  static QColor fill_col;   // dot fill color (same for all lattice dots)

};

} // end prim namespace



#endif
