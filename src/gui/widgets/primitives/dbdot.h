// @file:     dbdot.h
// @author:   Jake
// @created:  2016.11.15
// @editted:  2017.06.07  - Jake
// @license:  GNU LGPL v3
//
// @desc:     DBDot Widget for functionality of dangling bonds

#ifndef _GUI_PR_DBDOT_H_
#define _GUI_PR_DBDOT_H_


#include <QtWidgets>

#include "item.h"
#include "latdot.h"



namespace prim{

  // Specific Item derived class for a dangling bond on the lattice. Each
  // dangling bond should correspond to a source lattice dot. For
  // generality, each DBDot has its own physical location that will typically
  // be the same as the source LatticeDot.

  class DBDot: public prim::Item
  {
  public:

    // constructor
    DBDot(int lay_id, prim::LatticeDot *src=0);
    DBDot(QXmlStreamReader *);
    void initDBDot(int lay_id, prim::LatticeDot *src=0);

    // destructor
    ~DBDot(){}

    // accessors
    QPointF getPhysLoc() const {return phys_loc;}

    void setSource(prim::LatticeDot *src);
    prim::LatticeDot *getSource() const {return source;}

    void setFill(float fill){fill_fact = fill;}
    void setFillCol(QColor col){fill_col = col;}

    // inherited abstract method implementations

    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;

    prim::Item *deepCopy() const;

    // SAVE LOAD
    virtual void saveItems(QXmlStreamWriter *) const;

  private:

    // construct static variables
    void constructStatics();

    // VARIABLES

    QPointF phys_loc;         // physical location of dot in angstroms
    prim::LatticeDot *source; // lattice site beneath dot

    qreal fill_fact;  // area proportional of dot filled
    QColor fill_col;  // color of fill

    // static class parameters for painting

    static qreal diameter;      // dot diameter in angstroms
    static qreal edge_width;    // proportional width of dot boundary edge

    static QColor edge_col;     // edge colour, unselected
    static QColor selected_col; // edge colour, selected
  };

} // end prim namespace



#endif
