// @file:     layer.h
// @author:   Jake
// @created:  2016.11.15
// @editted:  2017.06.07  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Container class for different layers of the design. The surface
//            and subtrate lattice are special cases.


// NOTE: may want to distinguish between different types of layers using enum


#ifndef _GUI_PR_LAYER_H_
#define _GUI_PR_LAYER_H_


#include <QtWidgets>
#include <QtCore>
#include <QMetaEnum>
#include "item.h"


namespace prim{

  // Base Class for design layers, layers do not delete their items when destroyed
  class Layer : public QObject
  {
    Q_OBJECT  // enables tr without using QObject::tr

  public:

    enum LayerType{Lattice, DB, Electrode, AFMTip};
    Q_ENUM(LayerType)


    // constructor, create a Layer with the given name. If no name is given,
    // use the default naming scheme with layer_count.
    Layer(const QString &nm = QString(), const LayerType cnt_type=DB, const float z_height=0, int lay_id=-1, QObject *parent=0);
    Layer(QXmlStreamReader *stream);

    // destructor
    // ~Layer();

    // RESET
    // reset layers for new document
    static void resetLayers();

    // accessors
    
    // set layer index and update layer_id of contained items
    void setLayerIndex(int lay_id);

    // set / get the z-height of the layer
    void setZHeight(const float z_height) {zheight = z_height;}
    float getZHeight() {return zheight;}

    // add a new Item to the current layer. If the Item is already in the layer,
    // do nothing.
    void addItem(prim::Item *item, int index=-1);

    // attempt to remove the given Item from the layer. Returns true if the Item
    // is found and removed, false otherwise.
    bool removeItem(prim::Item *item);

    // pop the Item given by the index. Returns 0 if invalid index.
    prim::Item *takeItem(int ind=-1);

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

    // set the Layer content type
    void setContentType(LayerType type) {content_type = type;}

    // get the Layer content type, like "electrode", "dbdots", etc.
    LayerType getContentType() {return content_type;}
    const QString getContentTypeString() const {return QString(QMetaEnum::fromType<LayerType>().valueToKey(content_type));}

    // if i is within bounds, return a pointer to the indexed item in the Layer
    // item stack; otherwise, return 0
    prim::Item *getItem(int i) { return i >= 0 && i<items.size() ? items.at(i) : 0;}

    // get the Layer's items, needs to be a copy rather than a reference for Layer removal
    QStack<prim::Item*> &getItems() {return items;}

    // SAVE LOAD
    virtual void saveLayer(QXmlStreamWriter *) const;
    virtual void saveItems(QXmlStreamWriter *) const;
    virtual void loadItems(QXmlStreamReader *, QGraphicsScene *);

  public slots:
    void visibilityCheckBoxChanged(int check_state);
    void visibilityPushButtonChanged(bool checked) {setVisible(checked);}
    void editabilityPushButtonChanged(bool checked) {setActive(checked);}

  private:

    int layer_id;   // layer index in design panel's layers stack
    static uint layer_count;  // number of created Layer() objects, does not decrement
    float zheight;  // layer distance from surface

    QString name;   // arbitrary layer name, layers can be selected by name
    LayerType content_type;

    // list of items in order of insertion, should probably be a linked list
    QStack<prim::Item*> items;

    // flags
    bool visible; // layer is shown. If false, active should aso be false
    bool active;  // layer is edittable
  };

} // end prim namespace

#endif
