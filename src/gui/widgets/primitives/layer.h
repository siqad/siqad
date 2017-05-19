// @file:     layer.h
// @author:   Jake
// @created:  2016.11.15
// @editted:  2017.05.01  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Container class for different layers of the design. The surface
//            and subtrate lattice are special cases.

#ifndef _GUI_PR_LAYER_H_
#define _GUI_PR_LAYER_H_

#include <QtWidgets>
#include <QtCore>

namespace prim{

// Base class for design layers
class Layer : public QObject
{
  Q_OBJECT

public:

  // constructor
  Layer(QObject *parent=0);
  Layer(const QString &name, QObject *parent=0);

  // destructor
  ~Layer();

  // functionality

  // add a new QGraphicsItem pointer to the current Layer. If the pointer is
  // already found in the items list nothing is done.
  void addItem(QGraphicsItem *item);

  // attempt to remove the given QGraphicsItem pointer from the current Layer.
  // If the pointer is not found, nothing is done.
  void removeItem(QGraphicsItem *item);

  // calls setVisible(vis) for all QGraphicsItems in the Layer
  void setVisible(bool vis);

  // returns true if the Layer is visible
  bool isVisible(){return visible;}


  // calls setActive(act) for all QGraphicsItems in the Layer
  void setActive(bool act);

  // return true if the Layer is active.
  bool isActive(){return active;}

  const QString getName(){return name;}
  QList<QGraphicsItem*> getItems(){return items;}

private:

  static uint layer_cnt;

  QString name;

  // list of items, grouping should be handled by the design_widget
  QList<QGraphicsItem*> items;

  // flags
  bool visible;
  bool active;
};


}   // end prim namespace

#endif
