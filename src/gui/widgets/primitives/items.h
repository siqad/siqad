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

  enum ItemType{DotType, Text};

  // constructor
  MyItem(Emitter *em, QGraphicsItem *parent=0)
    : QGraphicsItem(parent), emitter(em), item_type(DotType) {}

  // destructor
  ~MyItem(){}

  ItemType item_type;

protected:

  void mousePressEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;
  //void mouseMoveEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;
  //void mouseReleaseEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;

  prim::Emitter *emitter;

private:
};




class Aggregate : public QGraphicsItemGroup
{
public:

  // constructor
  Aggregate(Emitter *em, QGraphicsItem *parent=0)
    : QGraphicsItemGroup(parent), emitter(em) {}

  // destructor
  ~Aggregate(){}

protected:

  void mousePressEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;
  //void mouseMoveEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;
  //void mouseReleaseEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;

  prim::Emitter *emitter;

private:

};


} // end prim namespace

#endif
