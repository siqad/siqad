// @file:     ghost.h
// @author:   Jake
// @created:  2016.11.24
// @editted:  2017.05.01  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Classes that give simplified graphical representations of sets
//            of object. Use for indicating projected locations after moving
//            or copying objects on the surface.

#ifndef _PRIM_GHOST_H_
#define _PRIM_GHOST_H_

#include <QGraphicsItemGroup>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QList>
#include <QHash>

#include <QRectF>
#include <QPointF>
#include <QColor>

#include <QPainter>

#include "src/settings/settings.h"
#include "items.h"
#include "dbdot.h"

namespace prim{

class GhostDot : public prim::MyItem
{
public:

  GhostDot(QGraphicsItemGroup *parent, QGraphicsItem *item, QColor *col);

  QRectF boundingRect() const;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*);

private:

  qreal diameter;
  QColor *pcol;
};

class Ghost : public QGraphicsItemGroup
{
public:

  QHash<prim::DBDot*, bool> valid_hash; // hash table for valid snap points

  // constructor
  Ghost(QGraphicsScene *scene=0);

  // destructor
  ~Ghost();

  // initializers

  // create an image of the given list of items
  void prepare(const QList<QGraphicsItem*> &items);
  void prepare(QGraphicsItem *item);

  // finds the lattice site beneath each dot. If there is no lattice site,
  // adds a NULL pointer
  QList<prim::DBDot*> getTargets();

  // get pointers to the DBDots corresponding to the dots
  QList<QGraphicsItem*> getSource() const;

  // transformation methods

  void flip(bool horiz);
  void rotate(bool cw);

  // accessors

  // get the location of the first item in the Ghost (arb.)
  QPointF getAnchor() const;

  // update the dot coloring and the valid flag for the ghost
  void updateValid();
  void setValid(bool flag);

  // check if the ghost is valid after a given offset
  bool checkValid(QPointF offset = QPointF());

  // return the lattice dbdot beneath each ghost dot. If a ghost dot is not above
  // a lattice site, append a null pointer.
  QList<prim::DBDot*> getLattice(QPointF offset = QPointF());

private:

  QList<QGraphicsItem*> source;   // target DBDot for each GhostDot
  QList<GhostDot*> dots;          // list of GhostDots

  QColor col;   // current dot color
  bool valid;   // true if all dots are above available lattice sites.

  void createGhostDot(QGraphicsItem *item);

  // recursive method for extracting locations of all items
  void prepareItem(QGraphicsItem *item);

  void cleanGhost();
  void zeroGhost();


  // check whether a given item is a dot in the lattice
  bool inLattice(QGraphicsItem *item);

};

} // end prim namespace

#endif
