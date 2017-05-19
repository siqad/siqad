// @file:     layer.h
// @author:   Jake
// @created:  2016.11.15
// @editted:  2017.05.10  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Container class for different layers of the design. The surface
//            and subtrate lattice are special cases.


// NOTE: may want to distinguish between different types of layers using enum


#ifndef _GUI_PR_LAYER_H_
#define _GUI_PR_LAYER_H_


#include <QtWidgets>
#include <QtCore>
#include "item.h"


namespace prim{

  // Base Class for design layers
  class Layer : public QObject
  {
    Q_OBJECT  // enables tr without using QObject::tr

  public:

    // constructor, create a Layer with the given name. If no name is given,
    // use the default naming scheme with layer_count.
    Layer(const QString &nm = QString(), QObject *parent=0);

    // destructor
    // ~Layer();

    // accessors

    // add a new Item to the current layer. If the Item is already in the layer,
    // do nothing.
    void addItem(prim::Item *item, int index=-1);

    // attempt to remove the given Item from the layer. Returns true if the Item
    // is found and remove, false otherwise.
    bool removeItem(prim::Item *item);

    // update the layer visibility, calls setVisible(vis) for all Items in the
    // layer. If vis==visibility, do nothing.
    void setVisible(bool vis);

    // check if the layer should be visible
    bool isVisible() const {return visible;}

    // update the layer activity, calls setActive(act) for all Items in the layer
    void setActive(bool act);

    // check if the layer should be active
    bool isActive() const {return active;}

    // get the Layer name, possibly change return to const QString&
    const QString& getName() const {return name;}

    // if i is within bounds, return a pointer to the indexed item in the Layer
    // item stack; otherwise, return 0
    prim::Item *getItem(int i) { return i >= 0 && i<items.size() ? items.at(i) : 0;}

    // get the Layer's items
    QStack<prim::Item*>& getItems() {return items;}

  private:

    static uint layer_count;  // number of created Layer() objects, does not decrement

    QString name;   // arbitrary layer name, layers can be selected by name

    // list of items in order of insertion, should probably be a linked list
    QStack<prim::Item*> items;

    // flags
    bool visible; // layer is shown. If false, active should aso be false
    bool active;  // layer is edittable
  };

} // end prim namespace

#endif
