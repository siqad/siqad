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
#include "emitter.h"


namespace prim{


class DBDot: public prim::MyItem
{

public:

  // constructor
  DBDot(QPointF p_loc, int layer, DBDot *src=0);

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

  QPointF phys_loc; // Physical location of dot in angstroms.
  DBDot *source;    // Lattice site beneath dot

  QColor edge_col;
  QColor selected_col;

  qreal fill_fact; // area proportion of dot filled
  QColor fill_col;

  // immutable parameters
  qreal scale_fact; // pixels per angstrom for dot locations
  qreal diameter;   // diameter of dot in pixels
  qreal edge_width; // edge pen width in pixels
};


} // end prim namespace


#endif
