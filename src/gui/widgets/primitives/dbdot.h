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
  DBDot(prim::Emitter *em, QPointF p_loc, bool lattice=false, DBDot *src=0);

  // destructor
  ~DBDot();

  // ACCESSORS
  bool inLattice(){return lattice;}
  QPointF getPhysLoc(){return phys_loc;}
  DBDot *getSource(){return source;}

  void setFill(float fill){fill_fact = fill;}

  // construct a deep copy of the dot
  DBDot *clone() const;

  // geometry
  QRectF boundingRect() const;

  // painting
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);


protected:



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

  bool lattice;
};


} // end prim namespace


#endif
