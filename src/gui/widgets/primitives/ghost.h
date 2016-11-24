#ifndef _PRIM_GHOST_H_
#define _PRIM_GHOST_H_

#include <QGraphicsItemGroup>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QList>

#include <QRectF>
#include <QPointF>
#include <QColor>

#include <QPainter>

#include "src/settings/settings.h"
#include "items.h"
#include "dbdot.h"

namespace prim{

class GhostDot : public QGraphicsItem
{
public:

  GhostDot(QGraphicsItemGroup *parent, QGraphicsItem *item);

  QRectF boundingRect() const;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *options, QWidget *widget);

private:

  qreal diameter;
  QColor col;
};

class Ghost : public QGraphicsItemGroup
{
public:

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

private:

  QList<QGraphicsItem*> source;   // target DBDot for each GhostDot
  QList<GhostDot*> dots;          // list of GhostDots

  void createGhostDot(QGraphicsItem *item);

  // recursive method for extracting locations of all items
  void prepareItem(QGraphicsItem *item);

  void cleanGhost();
  void zeroGhost();


  bool inLattice(QGraphicsItem *item);

};

} // end prim namespace

#endif
