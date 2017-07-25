// @file:     design_panel.h
// @author:   Jake
// @created:  2016.11.02
// @editted:  2017.06.07  - Jake
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

    enum ToolType{NoneTool, SelectTool, DragTool, DBGenTool, MeasureTool};

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

  signals:
    void sig_toolChange(ToolType tool);

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

    // copy/paste
    QList<prim::Item*> clipboard;  // cached deep copy of a set of items for pasting


    QStack<prim::Layer*> layers;  // stack of all layers, order immutable
    prim::Layer *top_layer;       // new items added to this layer

    // flags, change later to bit flags
    bool clicked;   // mouse left button is clicked
    bool ghosting;  // currently dragging a ghost
    bool moving;    // moving an existing group

    // snapping
    qreal snap_diameter;            // size of region to search for snap points
    prim::LatticeDot *snap_target;  // current snap target, LatticeDot
    QPointF snap_cache;             // cursor position of last snap update

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



    // RUBBER BAND

    QRubberBand *rb=0;  // rubber band object
    QPoint rb_start; // starting point of rubber band (scene)
    QPoint rb_cache; // cached to compare mouse movement (view)
    QList<QGraphicsItem*> rb_shift_selected; // list of previously selected items, for shift select

    // update rubberband during mouse movement
    void rubberBandUpdate(QPoint);

    // hide rubberband and reset flags
    void rubberBandEnd();

    

    // GHOSTING

    // create a Ghost for the current selection or clipboard
    void createGhost(bool paste);

    // clear the current Ghost
    void clearGhost();

    // snap the ghost to the nearest possible lattice position. Returns true if
    // the snap_target was update (need to change the ghost location).
    bool snapGhost(QPointF scene_pos, QPointF &offset);

    // initialize an item move
    void initMove();

    // set the selectability of the lattice dots for the given Item
    void setLatticeDotSelectability(prim::Item *item, bool flag);

    // deep copy the current selection to the clipboard
    void copySelection();

    // dbgen Location Indicator
    void snapDB(QPointF scene_pos);



    // UNDO/REDO FUNCTIONALITY

    // fundamental undo/redo command classes, keep memory requirement small

    class CreateDB;         // create a dangling bond at a given lattice dot
    class FormAggregate;    // form an aggregate from a list of Items

    class CreateLayer;      // create a new layer
    class DeleteLayer;      // delete an existing layer

    class MoveItem;         // move a single Item

    // functions including undo/redo behaviour

    // create dangling bonds in the surface at all selected lattice dots
    void createDBs();

    // delete all selected items
    void deleteSelection();

    // create an aggregate from the selected surface Items
    void formAggregate();

    // split all selected aggregates without deleting the contained items
    void splitAggregates();

    // destroy an aggregate with all contained item
    void destroyAggregate(prim::Aggregate *agg);

    // paste the current Ghost, returns True if successful
    bool pasteAtGhost();

    // helper functions for pasting specific items
    void pasteItem(prim::Ghost *ghost, prim::Item *item);
    void pasteDBDot(prim::Ghost *ghost, prim::DBDot *db);
    void pasteAggregate(prim::Ghost *ghost, prim::Aggregate *agg);


    // move the selected items to the current Ghost, returns True if successful
    bool moveToGhost(bool kill=false);

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

    bool invert;      // swaps create/delete on redo/undo

    DesignPanel *dp;  // DesignPanel pointer
    int layer_index;  // index of layer in dp->layers stack

    // internals
    int index;              // index of DBDot item in the layer item stack
    prim::LatticeDot *ldot; // Lattice dot beneath dangling bond
  };


  class DesignPanel::FormAggregate : public QUndoCommand
  {
  public:
    // group selected items into an aggregate
    FormAggregate(QList<prim::Item *> &items, DesignPanel *dp, QUndoCommand *parent=0);

    // split given aggregate into first layer of child items, offset keeps stack
    // indexing consistent when multiple aggregates are split at the same time
    FormAggregate(prim::Aggregate *agg, int offset, DesignPanel *dp, QUndoCommand *parent=0);

    // split the aggregate
    virtual void undo();

    // re-create the aggregate
    virtual void redo();

  private:

    void form();
    void split();

    bool invert;

    DesignPanel *dp;        // pointer to parent DesignPanel
    int layer_index;        // index of layer in Layer stack

    // internals
    QVector<int> item_inds; // list of indices of items in the Layer item stack
    int agg_index;          // index of agg in Layer item stack

  };


  class DesignPanel::MoveItem : public QUndoCommand
  {
  public:
    MoveItem(prim::Item *item, const QPointF &offset, DesignPanel *dp, QUndoCommand *parent=0);

    // move the Item back (by the negative of the offset)
    virtual void undo();

    // move the Item by the offset
    virtual void redo();

  private:

    // move the item either by offset or -offset
    void move(bool invert=false);

    // handler for moving an Item by a given delta
    void moveItem(prim::Item *item, const QPointF &delta);

    // move a DBDot by the given delta to a new LatticeDot
    void moveDBDot(prim::DBDot *dot, const QPointF &delta);

    // move an Aggregate by the given amount
    void moveAggregate(prim::Aggregate *agg, const QPointF &delta);

    DesignPanel *dp;

    QPointF offset;   // amount by which to move the Item
    int layer_index;  // index of layer containing the Item
    int item_index;   // index of item in Layer imte stack
  };


} // end gui namespace

#endif
