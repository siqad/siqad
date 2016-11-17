#ifndef _GUI_PR_LAYER_H_
#define _GUI_PR_LAYER_H_

#include <QList>
#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#include <QString>

namespace prim{

// Base class for design layers
class Layer
{
public:

  // constructor
  Layer();
  Layer(const QString &name);

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
  bool isVisible();


  // calls setActive(act) for all QGraphicsItems in the Layer
  void setActive(bool act);

  // return true if the Layer is active.
  bool isActive();

  const QString getName();


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
