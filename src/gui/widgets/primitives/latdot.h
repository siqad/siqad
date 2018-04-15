/** @file:     latdot.h
 *  @author:   Jake
 *  @created:  2017.05.03
 *  @editted:  2017.06.07  - Jake
 *  @license:  GNU LGPL v3
 *
 *  @desc:     Base graphical item for possible dangling bond locations on the
 *             surface lattice.
 */

#ifndef _GUI_PR_LATDOT_H_
#define _GUI_PR_LATDOT_H_


#include "item.h"

namespace prim{

  // forward declarations
  class Layer;
  class DBDot;

  //! Specific Item derived class for showing the possible dangling bond
  //! location on the surface lattice. For now, this class has very similar
  //! characteristics to the DBDot but will be kept a separate class to allow for
  //! future distinction in properties and avoid overbloating the functionality of
  //! either class.
  class LatticeDot: public Item
  {
  public:

    //! constructor, create a lattice dot at the given location in physical units
    LatticeDot(int layer_id, QPointF p_loc);

    //! destructor
    ~LatticeDot(){}

    // accessors

    QPointF getPhysLoc() const {return phys_loc;}   //!< get the dots physical location
    prim::DBDot *getDBDot() const {return dbdot;}   //!< get the created dangling bond
    void setDBDot(prim::DBDot *dot=0);              //!< set the created dangling bond

    // inherited abstract method implementations

    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;

    Item *deepCopy() const override;

  protected:

    // double click event is added to fix the bug where latdot becomes unselectable
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE {e->ignore();}

  private:

    // construct static variables
    void constructStatics();


    // VARIABLES

    QPointF phys_loc;   // physical location of the dot in angstroms
    prim::DBDot *dbdot; // dangling bond at the lattice dot

    // static class parameters for painting

    static qreal diameter;    // dot diameter in angstroms
    static qreal edge_width;  // proportional width of dot boundary edge
    static qreal publish_scale;

    static prim::Item::StateColors edge_col;   // boundary edge color
    static prim::Item::StateColors fill_col;   // dot fill color (same for all lattice dots)

    static qreal in_fill;                       // fill factor for inner circle
    static prim::Item::StateColors in_fill_col; // colour of inner circle
  };

} // end prim namespace

#endif
