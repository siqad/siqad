// @file:     design_panel.h
// @author:   Jake
// @created:  2016.11.02
// @editted:  2017.05.11  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Top level widget for the design panel. Contains all layers and
//            functionality for reating, selecting, moving, etc. object in the
//            design.

#ifndef _GUI_DESIGN_PANEL_H_
#define _GUI_DESIGN_PANEL_H_

#include <QtWidgets>
#include <QtCore>

#include "primitives/layer.h"
#include "primitives/lattice.h"
#include "primitives/items.h"
#include "primitives/emitter.h"

namespace gui{

  // Highest level of the design window visualization. Contains all
  // functionality for creating and viewing dangling bonds.
  class DesignPanel : public QGraphicsView
  {
    Q_OBJECT

  public:

    enum ToolType{None, SelectTool, DragTool, DBGenTool, MeasureTool};

    // constructor
    DesignPanel(QWidget *parent=0);

    // destructor
    ~DesignPanel();

    // ACCESSORS

    // add a new Item to the Layer at the given index of the stack. If layer_index==-1,
    // add the new item to the top_layer. If ind != -1, inserts the Item into the given
    // location of the Layer Item stack.
    void addItem(prim::Item *item, int layer_index=-1, int ind=-1);

    // remove the given Item from the given Layer if possible
    void removeItem(prim::Item *item, prim::Layer* layer);

    // add a new layer with the given name. If no name is given, a default scheme
    // is used. Checks if the layer already exists.
    void addLayer(const QString &name = QString());

    // attempt to remove a layer, either by name or index
    void removeLayer(const QString &name);
    void removeLayer(int n);

    // returns a pointer to the requested layer if it exists, else 0
    prim::Layer* getLayer(const QString &name) const;
    prim::Layer* getLayer(int n) const;

    // get the top_layer index
    int getLayerIndex(prim::Layer *layer=0) const;

    // get a list of all the surface dangling bonds
    QList<prim::DBDot *> getSurfaceDBs() const;

    // change the top layer to the requested layer
    void setLayer(const QString &name);
    void setLayer(int n);

    // resets the drawing layer and builds a lattice from the given <lattice>.ini
    // file. If no file is given, the default lattice is used
    void buildLattice(const QString &fname=QString());

    // update the tool type
    void setTool(ToolType tool);

    // update the fill values for the surface dangling bonds, no check for
    // array size/contents.
    void setFills(float *fills);

  public slots:

    void selectClicked(prim::Item *item);

  protected:

    // interrupts

    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *e) Q_DECL_OVERRIDE;

    void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;

    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *e) Q_DECL_OVERRIDE;

  private:

    QGraphicsScene *scene;  // scene for the QGraphicsView
    ToolType tool_type;     // current cursor tool type
    QUndoStack *undo_stack; // undo stack

    // copy/paste and item dragging

    QStack<prim::Layer*> layers;  // stack of all layers, order immutable
    prim::Layer *top_layer;       // new items added to this layer

    // flags, change later to bit flags
    bool clicked;   // mouse left button is clicked
    bool ghosting;  // currently dragging a ghost
    bool moving;    // moving an existing group

    // snapping
    qreal snap_diameter;
    prim::LatticeDot *snap_target;

    // mouse functionality
    QPoint mouse_pos_old;     // old mouse position in pixels
    QPoint mouse_pos_cached;  // parameter for caching relevant mouse positions, in pixels
    QPoint wheel_deg;         // accumulated degrees of "rotation" for mouse scrolls


    // INTERNAL METHODS

    // perform scene zoom based on wheel rotation
    void wheelZoom(QWheelEvent *e, bool boost);

    // perform scene pan based on wheel rotation
    void wheelPan(bool boost);

    // assert bounds on zooming
    void boundZoom(qreal &ds);

    // filter selected items
    void filterSelection(bool select_flag);

    // fundamental undo/redo command classes, keep memory requirement small

    class CreateDB;         // create a dangling bond at a given lattice dot
    class CreateAggregate;  // create an aggregate from a list of Items

    class DeleteItem;       // delete a given Item

    class CreateLayer;      // create a new layer
    class DeleteLayer;      // delete an existing layer

    // create dangling bonds in the surface at all selected lattice dots
    void createDBs();

    // delete all selection items
    void deleteSelection();

    // create an aggregate from the selected surface Items
    prim::Aggregate *createAggregate(int layer, QList<prim::Item*> items);

    // split an aggregate without deleting the contained items
    void splitAggregate(prim::Aggregate *aggregate);

    // destroy an aggregate with all contained item
    void destroyAggregate(prim::Aggregate *aggregate);

  };




  // Details for QUndoCommand derived classes

  class DesignPanel::CreateDB : public QUndoCommand
  {
  public:
    // create a dangling bond at the given lattice dot, set invert if deleting DB
    CreateDB(prim::LatticeDot *ldot, int layer_index, DesignPanel *dp,
                              bool invert=false, QUndoCommand *parent=0);

    // destroy the dangling bond and update the lattice dot
    virtual void undo();

    // re-create the dangling bond
    virtual void redo();

  private:

    void create();  // create the dangling bond
    void destroy(); // destroy the dangling bond

    // destroy the dangling bond and update the lattice dot
    void cleanDB();

    bool invert;      // swaps create/delete on redo/undo

    DesignPanel *dp;  // DesignPanel pointer
    int layer_index;  // index of layer in dp->layers stack

    // internals
    int index;              // index of DBDot item in the layer item stack
    prim::LatticeDot *ldot; // Lattice dot beneath dangling bond
  };


  // NOTE: How would we recreate an aggregate if the contained items were destroyed
  // and then recreated? Can't rely on the pointers being the stored items list is
  // useless.
  class DesignPanel::CreateAggregate : public QUndoCommand
  {
  public:
    // group selected items into an aggregate
    CreateAggregate(QList<prim::Item *> &items, DesignPanel *dp, QUndoCommand *parent=0);

    // split the aggregate
    virtual void undo();

    // re-create the aggregate
    virtual void redo();

  private:

    DesignPanel *dp;
    int layer_index;

    // internals
    QVector<int> item_inds;
    prim::Aggregate *agg;

  };

  //NOTE: need to make sure items are destroyed and re-create without changin their
  // order on the Item stacks.
  class DesignPanel::DeleteItem : public QUndoCommand
  {
  public:
    DeleteItem(prim::Item *item, QUndoCommand *parent=0);

    // re-create all the deleted items
    virtual void undo();

    // delete all the specified items
    virtual void redo();

  private:
    prim::Item *item;
  };

} // end gui namespace

#endif
