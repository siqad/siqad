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
#include "emitter.h"
#include "src/global.h"
#include "src/settings/settings.h"

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
    //! defined before use
    enum ItemType{Aggregate, DBDot, LatticeDot, Ghost, GhostDot, Text,
        Electrode, GhostBox, AFMArea, AFMPath, AFMNode, AFMSeg, PotPlot,
        ResizeFrame, ResizeHandle};

    //! constructor, layer = 0 should indicate temporary objects that do not
    //! belong to any particular layer
    Item(ItemType type, int lay_id=-1, QGraphicsItem *parent=0);

    //! destructor
    ~Item(){}

    //! update layer_id
    void setLayerIndex(int lay_id) {layer_id = lay_id;}

    // abstract member functions for derived classes
    virtual QRectF boundingRect() const = 0;
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) = 0;

    //! create a deep copy of the Item for the clipboard. Deep-copied items should
    //! have no parent or scene and need only to have the information necessary
    //! to create a new copy somewhere in the scene
    virtual Item *deepCopy() const = 0;

    //! true if the item or its parent has been seleted, recursive to highest level parent
    bool upSelected();

    //! set hovered state
    void setHovered(bool flag) {hovered = flag;}
    //! get hovered state
    bool isHovered() {return hovered;}
    //! true if the item or its parent has been hovered, recursive to highest level parent
    bool upHovered();

    //! set whether the item is resizable
    void setResizable(bool flag) {resizable = flag;}
    bool isResizable() {return resizable;}

    //! If the item is resizable, implement the resize function. The first two
    //! parameters (dx1, dy1) correspond to the delta for the top left corner,
    //! the next two parameters (dx2, dy2) correspond to the bottom right.
    //! Don't forget update the item position with setPos. update_handles
    //! indicate whether the resize frame handle positions should be updated,
    //! set to true if calling from QUndoStack.
    virtual void resize(qreal dx1, qreal dy1, qreal dx2, qreal dy2,
        bool update_handles=false)
      {Q_UNUSED(dx1); Q_UNUSED(dy1); Q_UNUSED(dx2); Q_UNUSED(dy2);
        Q_UNUSED(update_handles);}

    //! Note down the bounding rect of the item before resize.
    void setBoundingRectPreResize(const QRectF &rect)
      {bounding_rect_pre_resize = rect;}

    //! The bounding rect of the item before the resize.
    QRectF boundingRectPreResize() {return bounding_rect_pre_resize;}


    // securing the item type and layer as private isn't worth the copy
    // constructor calls for accessors, make public

    ItemType item_type;       // the ItemType of the Item
    int layer_id;             // the layer id of the Item

    // static class variables
    static qreal scale_factor;            // pixels/angstrom scaling factor
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
      QColor high_contrast;
    };

    //! Get the color corresponding to the current state of the item.
    QColor getCurrentStateColor(const StateColors &state_colors);

  protected:

    bool hovered; //!< manipulated through setHovered(bool) and hovered()

    // optional overridable mousePressEvent interrupt
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *) Q_DECL_OVERRIDE;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *) Q_DECL_OVERRIDE {}
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *) Q_DECL_OVERRIDE {}

  private:


    bool resizable=false;
    QRectF bounding_rect_pre_resize;  // used for resizable items

  };

} // end prim namespace

#endif
