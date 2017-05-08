// @file:     dbdot.h
// @author:   Jake
// @created:  2016.11.15
// @editted:  2017.05.01  - Jake
// @license:  GNU LGPL v3
//
// @desc:     DBDot Widget for functionality of dangling bonds

#ifndef _GUI_PR_DBDOT_H_
#define _GUI_PR_DBDOT_H_

#include <QGraphicsItem>
#include <QObject>

#include <QPointF>
#include <QPainter>

#include <QGraphicsSceneMouseEvent>

#include "items.h"
#include "latdot.h"
#include "emitter.h"


namespace prim{


class DBDot: public prim::MyItem
{

public:

  // constructor
  DBDot(QPointF p_loc, int layer, prim::LatticeDot *src=0);

  // destructor
  ~DBDot();

  // ACCESSORS
  bool inLattice(){return layer==0;}
  QPointF getPhysLoc(){return phys_loc;}
  DBDot *getSource(){return source;}

  void setFill(float fill){fill_fact = fill;}

  // construct a deep copy of the dot untied to a lattice src dot
  DBDot *clone() const;

  // geometry
  QRectF boundingRect() const;

  // painting
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);

private:

  QPointF phys_loc;         // Physical location of dot in angstroms.
  prim::LatticeDot *source; // Lattice site beneath dot


  // instance specific painting parameters

  qreal fill_fact;  // area proportion of dot filled
  QColor fill_col;  // color of fill

  // static class parameters for painting

  static qreal diameter;   // diameter of dot in pixels
  static qreal edge_width; // edge pen width in pixels

  static QColor edge_col;     // edge colour when unselected
  static QColor selected_col; // edge colour when selected
};


} // end prim namespace


#endif
