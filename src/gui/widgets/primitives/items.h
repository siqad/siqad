#ifndef _PRIM_ITEMS_H_
#define _PRIM_ITEMS_H_

// BASE CLASSES FOR ITEM AND ITEMGROUP TYPES WITH CUSTOM SIGNALS

#include <QGraphicsItem>
#include <QGraphicsItemGroup>

#include <QGraphicsSceneMouseEvent>

#include "emitter.h"


namespace prim{

class MyItem : public QGraphicsItem
{
public:

  enum ItemType{DBDotType, GhostDotType, Text};

  // constructor
  MyItem(ItemType type = DBDotType, int lay = -1, QGraphicsItem *parent=0)
    : QGraphicsItem(parent), item_type(type), layer(lay) {}

  // destructor
  ~MyItem(){}

  ItemType item_type;   // describes the type of the item.
  int layer;           // current layer containing the item.

protected:

  void mousePressEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;
  //void mouseMoveEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;
  //void mouseReleaseEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;

private:

};


class Aggregate : public QGraphicsItemGroup
{
public:

  // constructor
  Aggregate(QGraphicsItem *parent=0)
    : QGraphicsItemGroup(parent) {}

  // destructor
  ~Aggregate(){}

protected:

  void mousePressEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;
  //void mouseMoveEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;
  //void mouseReleaseEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;

private:

};


} // end prim namespace

#endif
