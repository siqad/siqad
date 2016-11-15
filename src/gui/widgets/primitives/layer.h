#ifndef _GUI_PR_LAYER_H_
#define _GUI_PR_LAYER_H_

#include <QList>
#include <QGraphicsItem>
#include <QGraphicsItemGroup>

namespace prim{

// Base class for design layers
class Layer
{
public:

  // constructor
  Layer();

  // destructor
  ~Layer();

  // functionality

  // add a new QGraphicsItem pointer to the current Layer. If the pointer is
  // already found in the items list nothing is done.
  void add_item(QGraphicsItem *item);

  // attempt to remove the given QGraphicsItem pointer from the current Layer.
  // If the pointer is not found, nothing is done.
  void remove_item(QGraphicsItem *item);

  // calls setVisible(vis) for all QGraphicsItems in the Layer
  void set_visible(bool vis);

  // returns true if the Layer is visible
  bool is_visible();


  // calls setActive(act) for all QGraphicsItems in the Layer
  void set_active(bool act);

  // return true if the Layer is active.
  bool is_active();


private:

  // list of items, grouping should be handled by the design_widget
  QList<QGraphicsItem*> items;

  // flags
  bool visible;
  bool active;
};


}   // end prim namespace

#endif
