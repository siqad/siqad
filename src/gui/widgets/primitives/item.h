/** @file:     item.h
 *  @author:   Jake
 *  @created:  2016.11.21
 *  @editted:  2017.06.07  - Jake
 *  @license:  GNU LGPL v3
 *
 *  @brief:     Base classes for all graphical items with custom signals
 */

#ifndef _PRIM_ITEMS_H_
#define _PRIM_ITEMS_H_

#include <QtWidgets>
#include <QtCore>
#include <QMetaEnum>

#include "emitter.h"
#include "global.h"
#include "settings/settings.h"
#include "gui/property_map.h"

namespace prim{

  // forward declaration for prim::Layer
  class Layer;

  //! Customized QGraphicsItem subclass. All items in the Layers must inherit
  //! this class and should be distinguished by the item_type member. Both
  //! boundingRect and paint must be redefined in any derived classes.
  class Item : public QGraphicsItem
  {
  public:

    //! Every derived class should be assigned an enumerated label in order
    //! to distinguish them in functions which accept Item objects. Derived
    //! classes can be declared and implemented elsewhere as long as they are
    //! defined before use. Make sure that when adding more ItemTypes,
    //! LastItemType stays at the end of the enum list.
    enum ItemType{Aggregate, DBDot, DBDotPreview, LatticeDotPreview, Ghost, GhostDot,
                  Text, Electrode, GhostBox, AFMArea, AFMPath, AFMNode, AFMSeg,
                  PotPlot, ResizeFrame, ResizeHandle, TextLabel,
                  GhostPolygon,
                  ScreenshotClipArea, ScaleBar, ResizeRotateFrame, ResizeRotateHandle, LastItemType};

    //! constructor, layer = 0 should indicate temporary objects that do not
    //! belong to any particular layer
    Item(ItemType type, int lay_id=-1, QGraphicsItem *parent=0);

    //! destructor
    ~Item(){}

    //! update layer_id
    void setLayerID(int lay_id) {layer_id = lay_id;}

    // abstract member functions for derived classes
    virtual QRectF boundingRect() const override = 0;
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override = 0;

    //! create a deep copy of the Item for the clipboard. Deep-copied items should
    //! have no parent or scene and need only to have the information necessary
    //! to create a new copy somewhere in the scene
    virtual Item *deepCopy() const {return 0;}

    //! true if the item or its parent has been seleted, recursive to highest level parent
    bool upSelected();

    //! returns the item type as a QString
    const QString getQStringItemType() {return getQStringItemType(item_type);}
    static const QString getQStringItemType(ItemType);

    static ItemType getEnumItemType(QString type);

    //! set hovered state
    void setHovered(bool flag) {hovered = flag;}
    //! get hovered state
    bool isHovered() {return hovered;}
    //! true if the item or its parent has been hovered, recursive to highest level parent
    bool upHovered();

    //! set whether the item is resizable
    void setResizable(bool flag) {resizable = flag;}
    bool isResizable() {return resizable;}

    //! Move the item by given delta. Normally just calls Qt's moveBy function but certain
    //! classes may override this for custom behavior.
    virtual void moveItemBy(qreal dx, qreal dy) {moveBy(dx, dy);}

    //! Retreve the class default property map of this item
    virtual gui::PropertyMap *classPropertyMap() {return 0;}
    virtual gui::PropertyMap *classPropertyMap() const {return 0;}

    //! Get a PropertyMap with the default and local properties combined
    gui::PropertyMap properties();

    gui::PropertyMap properties() const;
    void propMapFromXml(QXmlStreamReader *rs);
    //! Get property from the property map
    gui::Property getProperty(const QString &key) const;

    //! Set a property in this item's property map
    void setProperty(const QString &key, QVariant var) {local_props[key]=var;}

    //! Change the color of the item. initially does nothing
    virtual void setColor(QColor color __attribute__((unused))) {}

    //! Change the rotation of the item. initially does nothing
    virtual void setRotation(qreal angle_in __attribute__((unused))) {}

    //Reutrns the current fill color, initially does nothing.
    virtual QColor getCurrentFillColor(){return QColor();}
    // securing the item type and layer as private isn't worth the copy
    // constructor calls for accessors, make public

    ItemType item_type;       // the ItemType of the Item
    int layer_id;             // the layer id of the Item

    // static class variables
    static qreal scale_factor;            // pixels/angstrom scaling factor
    static qreal scale_factor_nm;         // pixels/nm scaling factor
    static gui::ToolType tool_type;       // current tool type of the GUI
    static gui::DisplayMode display_mode; // current display mode of the GUI

    static void init();

    // SAVE LOAD
    virtual void saveItems(QXmlStreamWriter *) const {}
    virtual void loadFromFile(QXmlStreamReader *) {} // TODO instead of using this function, switch to using constructor

    //! Handy struct for storing various color variables for items when under
    //! different states.
    struct StateColors {
      QColor normal;
      QColor hovered;
      QColor selected;
      QColor publish;
    };

    //! Get the color corresponding to the current state of the item.
    QColor getCurrentStateColor(const StateColors &state_colors);
    virtual void performAction(QAction*){};
    virtual QList<QAction*> contextMenuActions(){return QList<QAction*>();}

  protected:

    bool hovered; //!< manipulated through setHovered(bool) and hovered()

    // optional overridable mousePressEvent interrupt
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *) override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *) override {}
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *) override {}
  private:


    bool resizable=false;

    // properties of this item
    // Default properties of each class are static variables of each class
    QMap<QString, QVariant> local_props;  //! Properties altered from default

  };
}; // end prim namespace
// Q_DECLARE_METATYPE(prim::Item)
// Q_DECLARE_METATYPE(prim::Item*)
#endif
