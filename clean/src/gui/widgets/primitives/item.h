// @file:     item.h
// @author:   Jake
// @created:  2016.11.21
// @editted:  2017.05.16  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Base classes for all graphical items with custom signals

#ifndef _PRIM_ITEMS_H_
#define _PRIM_ITEMS_H_

#include <QtWidgets>
#include <QtCore>
#include "emitter.h"
#include "src/settings/settings.h"

namespace prim{

  // forward declaration for prim::Layer
  class Layer;

  // Customized QGraphicsItem subclass. All items in the Layers must inherit
  // this class and should be distinguished by the item_type member. Both
  // boundingRect and paint must be redefined in any derived classes.
  class Item : public QGraphicsItem
  {
  public:

    // every derived class should be assigned an enumerated label in order
    // to distinguish them in functions which accept Item objects. Derived
    // classes can be declared and implemented elsewhere as long as they are
    // defined before use
    enum ItemType{Aggregate, DBDot, LatticeDot, GhostDot, Text, Electrode};

    // constructor, layer = 0 should indicate temporary objects that do not
    // belong to any particular layer
    Item(ItemType type, Layer *lay=0, QGraphicsItem *parent=0);

    // destructor
    ~Item(){}

    // abstract member functions for derived classes
    virtual QRectF boundingRect() const = 0;
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) = 0;

    // true if the item or its parent has been seleted, recursive to highest level parent
    bool upSelected();

    // securing the item type and layer as private isn't worth the copy
    // constructor calls for accessors, make public

    ItemType item_type;   // the ItemType of the Item
    Layer *layer;            // the layer of the Item

    // static class variables
    static qreal scale_factor;  // pixels/angstrom scaling factor

  protected:

    // optional overridable mousePressEvent interrupt
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;
  };

} // end prim namespace

#endif
