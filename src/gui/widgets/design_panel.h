/** @file:     design_panel.h
 *  @author:   Jake
 *  @created:  2016.11.02
 *  @editted:  2017.06.07  - Jake
 *  @license:  GNU LGPL v3
 *
 *  @brief:     Top level widget for the design panel. Contains all layers and
 *            functionality for reating, selecting, moving, etc. object in the
 *            design.
 */

#ifndef _GUI_DESIGN_PANEL_H_
#define _GUI_DESIGN_PANEL_H_

#include <QtWidgets>
#include <QtCore>
#include <QDialog>

#include "../../global.h"

#include "afm_panel.h"

#include "primitives/layer.h"
#include "primitives/lattice.h"
#include "primitives/items.h"
#include "primitives/emitter.h"
#include "primitives/sim_job.h"

namespace gui{

  //! Highest level of the design window visualization. Contains all
  //! functionality for creating and viewing dangling bonds.
  class DesignPanel : public QGraphicsView
  {
    Q_OBJECT

  public:

    //enum ToolType{NoneTool, SelectTool, DragTool, DBGenTool, MeasureTool, ElectrodeTool, AFMPathTool};
    //enum DisplayMode{DesignMode, SimDisplayMode};


    class UndoCommand;

    //! constructor
    DesignPanel(QWidget *parent=0);

    //! destructor
    ~DesignPanel();

    // clear and reset
    void initDesignPanel();              //!< used on first init or after reset
    void clearDesignPanel(bool reset=false);  //!< used on exit or before reset
    void resetDesignPanel();             //!< call for reset

    // ACCESSORS

    //! add a new Item to the Layer at the given index of the stack. If layer_index==-1,
    //! add the new item to the top_layer. If ind != -1, inserts the Item into the given
    //! location of the Layer Item stack.
    void addItem(prim::Item *item, int layer_index=-1, int ind=-1);

    //! remove the given Item from the given Layer if possible
    void removeItem(prim::Item *item, prim::Layer* layer);

    //! add a new Item to the graphics scene. This either means the Item is already owned
    //! by another class and only needs to be shown graphically, or the Item is merely
    //! a temporary graphics item for purely indicative purposes.
    void addItemToScene(prim::Item *item);

    //! remove item from scene without deleting the item pointer. The caller has
    //! to handle the cleanup if so desired.
    void removeItemFromScene(prim::Item *item);

    //! return a list of selected prim::Items
    QList<prim::Item*> selectedItems();

    //! add a new layer with the given name. If no name is given, a default scheme
    //! is used. Checks if the layer already exists.
    void addLayer(const QString &name = QString(), const prim::Layer::LayerType cnt_type=prim::Layer::DB, const float zoffset = 0, const float zheight = 0);

    //! attempt to remove a layer, by name
    void removeLayer(const QString &name);
    //! attempt to remove a layer, by index
    void removeLayer(int n);

    //! retreieve entire layer stack
    //TODO might not be needed, delete if true
    QStack<prim::Layer*>* getLayers() {return &layers;}

    //! returns a pointer to the requested layer if it exists, else 0
    prim::Layer* getLayer(const QString &name) const;
    //! returns a pointer to the requested layer if it exists, else 0
    prim::Layer* getLayer(int n) const;

    //! get the top_layer index
    int getLayerIndex(prim::Layer *layer=0) const;

    //! get a list of all the surface dangling bonds
    QList<prim::DBDot *> getSurfaceDBs() const;

    //! change the top layer to the requested layer
    void setLayer(const QString &name);
    //! change the top layer to the requested layer
    void setLayer(int n);

    //! resets the drawing layer and builds a lattice from the given <lattice>.ini
    //! file. If no file is given, the default lattice is used
    void buildLattice(const QString &fname=QString());
    void setScenePadding();

    //! update the tool type
    void setTool(gui::ToolType tool);

    //! update the fill values for the surface dangling bonds, no check for
    //! array size/contents.
    void setFills(float *fills);

    //! set the undo stack as clean at the current index
    void stateSet() {undo_stack->setClean();}

    //! check if the contents of the DesignPanel have changed
    bool stateChanged() const {return !undo_stack->isClean();}

    //! return the current display mode
    DisplayMode displayMode() {return display_mode;}
    void setDisplayMode(DisplayMode mode);

    //! get afm_panel pointer
    AFMPanel *afmPanel() {return afm_panel;}

    // SAVE

    // flag if actions are performed after last saved
    int autosave_ind=0;
    int save_ind=0;

    void saveToFile(QXmlStreamWriter *stream, bool for_sim=false);
    void loadFromFile(QXmlStreamReader *);


    // SIMULATION RESULT DISPLAY
    //! Display the simulation result from SimAnneal
    void displaySimResults(prim::SimJob *job, int dist_int);
    //! Clear the simulation result from SimAnneal
    void clearSimResults();
    //! Display the simulation result from PoisSolver
    void displayPotentialPlot(QPixmap potential_plot, QRectF graph_container);


  public slots:
    void selectClicked(prim::Item *item);
    void simVisualizeDockVisibilityChanged(bool visible);

    void resizeBegin();

    void addItemToSceneRequest(prim::Item *item) {addItemToScene(item);}
    void removeItemFromSceneRequest(prim::Item *item) {removeItemFromScene(item);}

    void rotateCw();
    void rotateCcw();

    void emitUndoStackCleanChanged(bool c) {emit sig_undoStackCleanChanged(c);}

  signals:
    void sig_toolChangeRequest(gui::ToolType tool);  // request ApplicationGUI to change tool
    void sig_toolChanged(gui::ToolType tool);  // request ApplicationGUI to change tool
    void sig_resetDesignPanel();
    void sig_undoStackCleanChanged(bool); // emitted when undo_stack emits cleanChanged(bool)

  protected:

    void contextMenuEvent(QContextMenuEvent *e) override;

    // interrupts

    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *e) Q_DECL_OVERRIDE;

    void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;

    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *e) Q_DECL_OVERRIDE;

  private slots:
    void undoAction();
    void redoAction();
    void cutAction();
    void copyAction();
    void pasteAction();
    void deleteAction();
    void electrodeSetPotentialAction();
    void toggleDBElecAction();

  private:

    QGraphicsScene *scene;    // scene for the QGraphicsView
    gui::ToolType tool_type;       // current cursor tool type
    gui::DisplayMode display_mode; // current display mode
    QUndoStack *undo_stack;   // undo stack

    // children panels
    AFMPanel *afm_panel;
    // TODO layer manager

    // copy/paste
    QList<prim::Item*> clipboard;  // cached deep copy of a set of items for pasting

    QStack<prim::Layer*> layers;  // stack of all layers, order immutable
    prim::Layer *top_layer;       // new items added to this layer
    prim::Layer *electrode_layer; // add electrodes to this layer
    prim::Layer *afm_layer;       // add afm paths to this layer TODO request layers from Layer Manager instead of keeping pointers like these
    prim::Layer *plot_layer;      // add potential plots to this layer

    // flags, change later to bit flags
    bool clicked;   // mouse left button is clicked
    bool ghosting;  // currently dragging a ghost
    bool moving;    // moving an existing group
    bool pasting;   // evoked some kind of pasting
    bool resizing;  // currently resizing an item

    // snapping
    qreal snap_diameter;            // size of region to search for snap points
    prim::LatticeDot *snap_target;  // current snap target, LatticeDot
    QPointF snap_cache;             // cursor position of last snap update

    // AFM ghost
    prim::AFMNode *ghost_afm_node=0;
    prim::AFMSeg *ghost_afm_seg=0;

    // mouse functionality
    QPoint mouse_pos_old;     // old mouse position in pixels on click
    QPoint mouse_pos_cached;  // parameter for caching relevant mouse positions on click, in pixels
    QPoint wheel_deg;         // accumulated degrees of "rotation" for mouse scrolls

    // sim visualization
    QList<prim::DBDot*> db_dots_result;

    // INTERNAL METHODS

    // perform scene zoom based on wheel rotation
    void wheelZoom(QWheelEvent *e, bool boost);

    // perform scene pan based on wheel rotation
    void wheelPan(bool boost);

    // assert bounds on zooming
    void boundZoom(qreal &ds);

    // scroll the viewport to the correct location with the provided delta in scene
    // coordinates, taking the transformation (zoom and rotate) into account. Used
    // when rotating the view or anchoring during zoom.
    void scrollDelta(QPointF delta);

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



    // GHOSTclass UndoCommand;e a Ghost for the current selection or clipboard
    // create a ghost. scene_pos is only needed if not pasting
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
    void snapDBPreview(QPointF scene_pos);

    // return the scene position of the nearest prim::Item with the specified item types.
    // returns a null pointer if no eligible item falls within the search range.
    prim::Item *filteredSnapTarget(QPointF scene_pos, QList<prim::Item::ItemType>
        &target_types, qreal search_box_width);



    // UNDO/class UndoCommand;redo base class

    // fundamental undo/redo command classes, keep memory requirement small

    class CreateDB;         // create a dangling bond at a given lattice dot
    class FormAggregate;    // form an aggregate from a list of Items

    class CreateLayer;      // create a new layer
    class DeleteLayer;      // delete an existing layer

    class MoveItem;         // move a single Item

    class CreateElectrode;  // create an electrode at the given points

    class CreatePotPlot;  // create an electrode at the given points

    class CreateAFMArea;    // create an AFM area

    class CreateAFMPath;    // create an empty AFMPath that should later contain AFMNodes
    class CreateAFMNode;    // create AFMNodes that should be children of AFMPath

    class ResizeAFMArea;    // resize an AFM Area

    // functions including undo/redo behaviour

    // create dangling bonds in the surface at all selected lattice dots
    void createDBs();

    void createElectrodes(QPoint point1);

    //create potential plot on panel
    void createPotPlot(QPixmap potential_plot, QRectF graph_container);

    // create AFM area with rubberband selected area
    void createAFMArea(QPoint point1);

    // create AFM node in focused path after focused node
    void createAFMNode();

    void resizeItem(prim::Item *item, const QRectF &orig_rect,
        const QRectF &new_rect);

    // resize AFM Area
    void resizeAFMArea(prim::AFMArea *afm_area, const QRectF &orig_rect,
        const QRectF &new_rect);

    // destroy AFM path and included nodes
    void destroyAFMPath(prim::AFMPath *afm_path);

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
    void pasteElectrode(prim::Ghost *ghost, prim::Electrode *elec);
    void pasteAFMArea(prim::Ghost *ghost, prim::AFMArea *afm_area);

    // move the selected items to the current Ghost, returns True if successful
    bool moveToGhost(bool kill=false);

    void createActions();
    QAction *action_undo;
    QAction *action_redo;
    QAction *action_cut;
    QAction *action_copy;
    QAction *action_paste;
    QAction *action_delete;
    QAction *action_set_potential;
    QAction *action_toggle_db_elec;
  };



  // Details for QUndoCommand derived classes

  class DesignPanel::CreateDB : public QUndoCommand
  {
  public:
    // create a dangling bond at the given lattice dot, set invert if deleting DB
    CreateDB(prim::LatticeDot *ldot, int layer_index, DesignPanel *dp, prim::DBDot *src_db=0,
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
    int elec;         // elec content of the db

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
    void moveElectrode(prim::Electrode *electrode, const QPointF &delta);
    void moveAFMArea(prim::AFMArea *afm_area, const QPointF &delta);

    DesignPanel *dp;

    QPointF offset;   // amount by which to move the Item
    int layer_index;  // index of layer containing the Item
    int item_index;   // index of item in Layer imte stack
  };


  class DesignPanel::CreateElectrode : public QUndoCommand
  {
  public:
    // create an electrode at the given points
    CreateElectrode(int layer_index, gui::DesignPanel *dp, QPointF point1, QPointF point2, prim::Electrode *elec = 0, bool invert=false, QUndoCommand *parent=0);

  private:

    // destroy the dangling bond and update the lattice dot
    virtual void undo();
    // re-create the dangling bond
    virtual void redo();

    void create();  // create the dangling bond
    void destroy(); // destroy the dangling bond

    DesignPanel *dp;  // DesignPanel pointer
    int layer_index;  // index of layer in dp->layers stack

    QPointF point1;
    QPointF point2;

    bool invert;

    // internals
    int index;              // index of electrode item in the layer item stack

  };

  class DesignPanel::CreatePotPlot : public QUndoCommand
  {
  public:
    // create an electrode at the given points
    CreatePotPlot(int layer_index, gui::DesignPanel *dp, QPixmap potential_plot, QRectF graph_container,
      prim::PotPlot *pp = 0, bool invert=false, QUndoCommand *parent=0);

  private:

    // destroy the dangling bond and update the lattice dot
    virtual void undo();
    // re-create the dangling bond
    virtual void redo();

    void create();  // create the dangling bond
    void destroy(); // destroy the dangling bond

    DesignPanel *dp;  // DesignPanel pointer
    int layer_index;  // index of layer in dp->layers stack

    QPixmap potential_plot;
    QRectF graph_container;

    bool invert;

    // internals
    int index;              // index of electrode item in the layer item stack
  };


  class DesignPanel::CreateAFMArea : public QUndoCommand
  {
  public:
    //! Create an AFMArea at the given points
    CreateAFMArea(int layer_index, gui::DesignPanel *dp, QPointF point1,
        QPointF point2, prim::AFMArea *afm_area=0, bool invert=false,
        QUndoCommand *parent=0);

  private:
    //! Destroy the AFMArea
    virtual void undo();
    //! Re-create the AFMArea
    virtual void redo();

    void create();    //! Create the AFMArea.
    void destroy();   //! Destroy the AFMArea.

    DesignPanel *dp;  //! Pointer to the DesignPanel
    int layer_index;  //! Index of layer in dp->layers stack

    QPointF point1;
    QPointF point2;

    bool invert;

    int index;        //! Index of this item in the layer item stack.
  };


  class DesignPanel::CreateAFMPath : public QUndoCommand
  {
  public:
    // create an empty AFMPath
    CreateAFMPath(int layer_index, DesignPanel *dp, prim::AFMPath *afm_path=0,
                    bool invert=false, QUndoCommand *parent=0);

    // destroy the AFMPath, which is not necessarily empty
    virtual void undo();

    // re-create the AFMPath
    virtual void redo();

  private:
    void create();    // create the AFMPath
    void destroy();   // destroy the AFMPath

    bool invert;      // swaps create/delete on redo/undo

    DesignPanel *dp;  // DesignPanel pointer
    int layer_index;  // index of layer in dp->layers stack

    int index;        // index of this path in the items stack
  };


  class DesignPanel::CreateAFMNode : public QUndoCommand
  {
  public:
    // create an AFMNode with the given AFMPath and index in path
    CreateAFMNode(int layer_index, DesignPanel *dp, QPointF scenepos, float z_offset,
          int afm_index, int index_in_path=-1, bool invert=false,
          QUndoCommand *parent=0);

    // remove the AFMNode from its Path and destroy the node
    virtual void undo();

    // re-create the node
    virtual void redo();

  private:
    void create();    // create the AFMNode
    void destroy();   // destroy the AFMNode

    bool invert;      // swaps create/delete on redo/undo

    int layer_index;
    DesignPanel *dp;
    int afm_index;    // the Path's index in its layer
    int node_index;   // the Node's index in the path
    QPointF scenepos;
    float z_offset;
  };

  class DesignPanel::ResizeAFMArea : public QUndoCommand
  {
  public:
    // resize the AFM area from the original positions to the new positions
    ResizeAFMArea(int layer_index, DesignPanel *dp,
        const QRectF &orig_rect, const QRectF &new_rect,
        int afm_area_index, bool invert=false, QUndoCommand *parent=0);

    // resize from new to original positions
    virtual void undo();

    // resize from original to new positions
    virtual void redo();

  private:
    bool invert;

    int layer_index;
    DesignPanel *dp;
    int afm_area_index; // the AFM Area's index in its layer
    QPointF top_left_delta;
    QPointF bot_right_delta;
    QRectF orig_rect;
    QRectF new_rect;
  };


} // end gui namespace

#endif
