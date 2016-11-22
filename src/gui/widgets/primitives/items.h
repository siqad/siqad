#ifndef _PRIM_ITEMS_H_
#define _PRIM_ITEMS_H_

// BASE CLASSES FOR ITEM AND ITEMGROUP TYPES WITH CUSTOM SIGNALS

#include <QGraphicsItem>
#include <QGraphicsItemGroup>

#include <QGraphicsSceneMouseEvent>


namespace prim{


class MyItem : public QGraphicsItem
{
public:

  // constructor
  MyItem(QGraphicsItem *parent=0) : QGraphicsItem(parent){}

  // destructor
  ~MyItem(){}

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
  Aggregate(QGraphicsItem *parent=0) : QGraphicsItemGroup(parent) {}

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
