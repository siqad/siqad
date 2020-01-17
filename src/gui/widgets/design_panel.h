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
#include <QImage>

#include "global.h"

#include "afm_panel.h"
#include "property_editor.h"
#include "managers/layer_manager.h"
#include "managers/item_manager.h"
#include "managers/screenshot_manager.h"
#include "color_dialog.h"
#include "rotate_dialog.h"

#include "primitives/layer.h"
#include "primitives/lattice.h"
#include "primitives/items.h"
#include "primitives/emitter.h"
#include "components/sim_job.h"

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

    QWidget *layerManagerSideWidget() {return layman->sideWidget();}
    QWidget *itemManagerWidget() {return itman;} //ItemManager fits the side bar at the moment.

    //! add a new Item using a command from the dialog panel.
    bool commandCreateItem(QString item_type, QString layer_id, QStringList item_args);

    //! remove an Item using a command from the dialog panel.
    bool commandRemoveItem(QString item_type, QStringList brackets, QStringList numericals);

    //! switch statement to properly delete items
    void commandRemoveHandler(prim::Item *item);

    //! Move an item on the design panel using a command from the dialog panel.
    bool commandMoveItem(QString item_type, QStringList brackets, QStringList numericals);

    //! Find the appropriate offset for commandMove, given the cleaned item arguments.
    QPointF findMoveOffset(QStringList args);

    //! add a new Item to the Layer at the given index of the stack. If layer_index==-1,
    //! add the new item to the top_layer. If ind != -1, inserts the Item into the given
    //! location of the Layer Item stack.
    void addItem(prim::Item *item, int layer_index=-1, int ind=-1);

    //! Remove the given item from the layer at the given index if possible.
    //! If retain item is set to true, then the item itself would not be deleted.
    void removeItem(prim::Item *item, int layer_index, bool retain_item=false);

    //! Remove the given Item from the given Layer if possible.
    void removeItem(prim::Item *item, prim::Layer* layer, bool retain_item=false);

    //! Add a new Item to the graphics scene without adding it to a layer. This
    //! either means the Item is already owned by another class and only needs
    //! to be shown graphically, or the Item is merely a temporary graphics
    //! item for purely indicative purposes.
    void addItemToScene(prim::Item *item);

    //! Remove item from scene without deleting the item pointer. The caller has
    //! to handle the cleanup if so desired.
    void removeItemFromScene(prim::Item *item);

    //! return a list of selected prim::Items
    QList<prim::Item*> selectedItems();

    //! get a list of all the surface dangling bonds
    QList<prim::DBDot*> getSurfaceDBs() const;

    //! Return a list of DBDot pointers at the provided physical locations. If
    //! any of the locations are not valid DB sites, an exception is thrown.
    QList<prim::DBDot*> getDBsAtLocs(QList<QPointF> phys_locs);

    //! resets the drawing layer and builds a lattice from the given <lattice>.ini
    //! file. If no file is given, the default lattice is used
    void buildLattice(const QString &fname=QString());
    void setSceneMinSize();

    //! Check if given QPointF falls within a lattice dot
    bool isLatticeDot(QPointF scene_pos);

    //! update the tool type
    void setTool(gui::ToolType tool);

    //! update the fill values for the surface dangling bonds, no check for
    //! array size/contents.
    void setFills(float *fills);

    //! set the undo stack as clean at the current index
    void stateSet() {undo_stack->setClean();}

    //! check if the contents of the DesignPanel have changed
    bool stateChanged() const {return !undo_stack->isClean();}

    //! take a screenshot of the design at the specified QRect in scene coord
    void screenshot(QPainter *painter, const QRect &region=QRect());

    //! Return the current display mode.
    DisplayMode displayMode() {return display_mode;}

    //! Set the current display mode. DisplayMode is defined in global.h.
    void setDisplayMode(DisplayMode mode);

    //! get afm_panel pointer
    AFMPanel *afmPanel() {return afm_panel;}

    // SAVE

    // flag if actions are performed after last saved
    int autosave_ind=0;
    int save_ind=0;

    //! Save layers and items into the given write stream.
    void writeToXmlStream(QXmlStreamWriter *, DesignInclusionArea);


    // LOAD

    //! Load layers and items from the given read stream.
    void loadFromFile(QXmlStreamReader *);

    //! Load GUI flags.
    void loadGUIFlags(QXmlStreamReader *);

    //! Load layers.
    void loadLayers(QXmlStreamReader *, QList<int> &layer_order_id);

    //! Load layer properties.
    void loadLayerProps(QXmlStreamReader *, QList<int> &layer_order_id);

    //! Load design (items contained within layers).
    void loadDesign(QXmlStreamReader *, QList<int> &layer_order_id);


    // SIMULATION RESULT DISPLAY

    //! Display the simulation result from SimAnneal
    void displaySimResults(comp::SimJob *job, int dist_int, bool avg_degen);

    //! Clear the simulation result from SimAnneal
    void clearSimResults();

    void clearPlots();

    //! Display the simulation result from PoisSolver
    void displayPotentialPlot(QString pot_plot_path, QRectF graph_container, QString pot_anim_path);

    //! Show the color dialog, adding the target items into the list of items to recolor.
    void showColorDialog(QList<prim::Item*> target_items);

    //!Show rotation dialog
    void showRotateDialog(QList<prim::Item*> target_items);

  public slots:
    // items
    void deselectAll();
    //! A selected item has been clicked on.
    void selectClicked(prim::Item *item);

    //! Show the property of an item.
    void showItemProperty(prim::Item *item) {property_editor->showForms(QList<prim::Item*>({item}));}

    //! Begin resizing an item.
    void resizeBegin();

    // simulation
    void simVisualizeDockVisibilityChanged(bool visible);

    // gui
    void rotateCw();
    void rotateCcw();

    //! change the color of selected items
    void changeItemColors(QColor);

    //! change the rotation of items.
    void setItemRotations(double rot);

    //! Move item to given lattice coordinates. Mainly for Item Emitter to instruct
    //! movements, use setPos directly otherwise.
    void moveDBToLatticeCoord(prim::Item *, int, int, int);

    void physLoc2LatticeCoord(QPointF physloc, int &n, int &m, int &l);

    //! Emitted when the undo stack clean stage has changed.
    void emitUndoStackCleanChanged(bool c) {emit sig_undoStackCleanChanged(c);}

    //! Update background to match current display mode and zoom level.
    void updateBackground();

    //! Edit text label
    void editTextLabel(prim::Item *text_lab, const QString &new_text);

    //! update movie
    void updateSimMovie();

  signals:
    void sig_itemAdded(); //notify ItemManager that item was added.
    void sig_itemRemoved(prim::Item* item); //notify ItemManager that item was removed.
    void sig_toolChangeRequest(gui::ToolType tool);  // request ApplicationGUI to change tool
    void sig_toolChanged(gui::ToolType tool); // notify other components of the
                                              // change in tool
    void sig_preDPResetCleanUp();
    void sig_postDPReset();
    void sig_undoStackCleanChanged(bool); // emitted when undo_stack emits cleanChanged(bool)

    //! Request ApplicationGUI to update the layer manager widget being used.
    void sig_setLayerManagerWidget(QWidget*);

    //! Request ApplicationGUI to update the item manager widget being used.
    void sig_setItemManagerWidget(QWidget*);

    //! Request ApplicationGUI to show simulation setup dialog.
    void sig_showSimulationSetup();

    //! Request SimManager to quickly run a simulation without showing the dialog.
    void sig_quickRunSimulation();

    //! Request ApplicationGUI to take a screenshot of the design panel bounded
    //! by the given scene_rect. If the rect is null, take a screenshot of the
    //! entire scene.
    void sig_screenshot(const QString &target_img_path, const QRectF &scene_rect, bool always_overwrite);

    //! Tell ApplicationGUI to cancel the current screenshot.
    void sig_cancelScreenshot();

    //! Emit the current cursor scene coordinates.
    void sig_cursorPhysLoc(QPointF cursor_pos);

    //! Emit the current zoom level.
    void sig_zoom(float zoom);

    //! Emit the current total DB count.
    void sig_totalDBCount(int db_count);

    //! Emit the bounding rect of the currently selected items.
    void sig_selBoundingRect(QRectF b_rect);

    //! Emit the currently selected items. Empty for no selected items.
    void sig_selectedItems(QList<prim::Item*> items);

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

    void dragEnterEvent(QDragEnterEvent *e) Q_DECL_OVERRIDE;

  private slots:
    void undoAction();
    void redoAction();
    void cutAction();
    void copyAction();
    void pasteAction();
    void dummyAction();
    void deleteAction();

  private:

    QGraphicsScene *scene;    // scene for the QGraphicsView
    QRectF min_scene_rect;    // minimum size of the scene rect
    gui::ToolType tool_type;  // current cursor tool type
    gui::DisplayMode display_mode=DesignMode; // current display mode
    QUndoStack *undo_stack;   // undo stack

    // contained widgets
    gui::LayerManager *layman=nullptr;
    gui::PropertyEditor *property_editor=nullptr;
    gui::ItemManager *itman=nullptr;
    gui::ScreenshotManager *screenman=nullptr;
    ColorDialog *color_dialog = 0;    //Color dialog widget
    RotateDialog *rotate_dialog = 0;    //rotate dialog widget


    // background color presets
    static QColor background_col;         // normal background color
    static QColor background_col_publish; // background color in publishing mode
    static qreal zoom_visibility_threshold;

    // Common actions used in the design panel
    QAction *action_undo;       // reverse in the undo stack
    QAction *action_redo;       // advance in the undo stack
    QAction *action_cut;        // cut selected items
    QAction *action_copy;       // copy selected items
    QAction *action_paste;      // paste items in clipboard
    QAction *action_delete;     // delete selected items

    QAction *action_form_agg;   // form DBDot aggregates
    QAction *action_split_agg;  // splite selected aggregate
    QAction *action_dup;        // duplicate selected objects

    // children panels
    AFMPanel *afm_panel;
    // TODO layer manager

    // copy/paste
    QList<prim::Item*> clipboard;   // cached deep copy of a set of items for pasting
    QList<prim::Item*> cache;       // general purpose cache

    prim::Lattice *lattice=0;       // lattice for reference

    // flags, change later to bit flags
    bool clicked;   // mouse left button is clicked
    bool ghosting;  // currently dragging a ghost
    bool moving;    // moving an existing group
    bool pasting;   // evoked some kind of pasting
    bool resizing;  // currently resizing an item

    // DB previews
    QList<prim::DBDotPreview*> db_previews;

    // snapping
    qreal snap_diameter;            // size of region to search for snap points
    prim::LatticeCoord snap_coord;  // current snap target, lattice coordinate
    prim::LatticeCoord coord_start; // lattice coordinate of mouse click location
    QPointF snap_cache;             // cursor position of last snap update

    // AFM ghost
    prim::AFMNode *ghost_afm_node=0;
    prim::AFMSeg *ghost_afm_seg=0;

    // mouse functionality
    QPoint press_scene_pos;   // mouse position on click (view coord)
    QPoint prev_pan_pos;      // mouse position on last panning update (view coord)
    QPoint wheel_deg;         // accumulated degrees of "rotation" for mouse scrolls

    // screenshot
    QRect prev_screenshot_area; // store the previous screenshot area for reuse

    // sim visualization
    QList<prim::DBDot*> db_dots_result;
    QList<prim::Item*> sim_results_items;  // holding temporary items

    // INTERNAL METHODS

    // initialize actions
    void initActions();

    // construct static variables on first init
    void constructStatics();

    // array generation of selected items
    void duplicateSelection();

    // perform scene zoom based on wheel rotation
    void wheelZoom(QWheelEvent *e, bool boost);

    // perform scene pan based on wheel rotation, swap x and y if shift is pressed
    void wheelPan(bool shift_scroll, bool boost);

    // assert bounds on zooming
    void boundZoom(qreal &ds);

    // scroll the viewport to the correct location with the provided delta in scene
    // coordinates, taking the transformation (zoom and rotate) into account. Used
    // when rotating the view or anchoring during zoom.
    void scrollDelta(QPointF delta);

    // RUBBER BAND

    QRubberBand *rb=0;    // rubber band object
    QPoint rb_start;      // starting point of rubber band (scene)
    QPoint rb_cache;      // cached to compare mouse movement (view)
    QRect rb_scene_rect;  // rubberband selection area in scene coordinates
    QList<QGraphicsItem*> rb_shift_selected; // list of previously selected items, for shift select

    //! Update rubberband during mouse movement
    void rubberBandUpdate(QPoint);

    //! Select objects contained in the rubberband.
    void rubberBandSelect();

    //! Clear rubberband content and reset flags.
    void rubberBandClear();

    // SCREENSHOT

    QRect screenshot_rect;


    // GHOSTclass UndoCommand;e a Ghost for the current selection or clipboard
    // create a ghost. scene_pos is only needed if not pasting
    void createGhost(bool paste, int count=1);

    // clear the current Ghost
    void clearGhost();

    // snap the ghost to the nearest possible lattice position. Returns true if
    // the snap_target was update (need to change the ghost location).
    bool snapGhost(QPointF scene_pos, prim::LatticeCoord &offset);

    // initialize an item move
    void initMove();

    // set the selectability of the lattice dots for the given Item
    void setLatticeSiteOccupancy(prim::Item *item, bool flag);

    // deep copy the current selection to the clipboard
    void copySelection();

    //! Create graphical previews for provided DB coordinates (always destroys
    //! existing previews).
    void createDBPreviews(QList<prim::LatticeCoord> coords);

    //! Add DB graphical previews without destroying existing ones.
    void appendDBPreviews(QList<prim::LatticeCoord> coords);

    //! Destroy DB graphical previews.
    void destroyDBPreviews();

    // return the scene position of the nearest prim::Item with the specified item types.
    // returns a null pointer if no eligible item falls within the search range.
    prim::Item *filteredSnapTarget(QPointF scene_pos, QList<prim::Item::ItemType>
        &target_types, qreal search_box_width);



    // UNDO/class UndoCommand;redo base class

    // fundamental undo/redo command classes, keep memory requirement small

    class CreateItem;       // create any prim::Item that doesn't require extra checks
    class ResizeItem;       // resize a ResizableRect

    class CreateDB;         // create a dangling bond at a given lattice dot
    class FormAggregate;    // form an aggregate from a list of Items

    class CreateLayer;      // create a new layer
    class DeleteLayer;      // delete an existing layer

    class MoveItem;         // move a single Item

    class CreatePotPlot;  // create an electrode at the given points

    class CreateAFMPath;    // create an empty AFMPath that should later contain AFMNodes
    class CreateAFMNode;    // create AFMNodes that should be children of AFMPath

    class CreateTextLabel;  // create a text label
    class EditTextLabel;

    class RotateItem;       // resize a ResizableRect

    class ChangeColor;       // change color of item

    // functions including undo/redo behaviour

    //! If no lat_coord is provided, create DBs at all DB preview locations
    //! stored in the db_previews list. Otherwise, create a DB at the provided
    //! coord (intended for SQCommand).
    void createDBs(prim::LatticeCoord lat_coord = prim::LatticeCoord());

    //! Create electrode with rubberband area, assumes the given rect is already
    //! in scene coordinates.
    void createElectrode(QRect scene_rect);


    //create potential plot on panel
    void createPotPlot(QString pot_plot_path, QRectF graph_container, QString pot_anim_path);

    //! Create AFM area with rubberband selected area, assumes the given rect is
    //! already in scene coordinates.
    void createAFMArea(QRect scene_rect);

    // create AFM node in focused path after focused node
    void createAFMNode();

    //! Create a text label
    void createTextLabel(const QRect &scene_rect);

    //! Edit a text label
    void editTextLabel(prim::TextLabel *text_lab, const QString &new_text);

    void resizeItemRect(prim::Item *item, const QRectF &orig_rect,
        const QRectF &new_rect);

    // destroy AFM path and included nodes
    void destroyAFMPath(prim::AFMPath *afm_path);

    // delete all selected items
    void deleteSelection();

    // create an aggregate from the provided list or from selected surface Items
    void formAggregate(QList<prim::Item*> items=QList<prim::Item*>());

    // split all selected aggregates without deleting the contained items
    void splitAggregates();

    // destroy an aggregate with all contained item
    void destroyAggregate(prim::Aggregate *agg);

    // paste the current Ghost, returns True if successful
    bool pasteAtGhost();

    // helper functions for pasting specific items to indexd ghost set
    void pasteItem(prim::Ghost *ghost, int n, prim::Item *item);
    void pasteDBDot(prim::Ghost *ghost, int n, prim::DBDot *db);
    void pasteAggregate(prim::Ghost *ghost, int n, prim::Aggregate *agg);
    void pasteElectrode(prim::Ghost *ghost, int n, prim::Electrode *elec);
    void pasteAFMArea(prim::Ghost *ghost, int n, prim::AFMArea *afm_area);

    // move the selected items to the current Ghost, returns True if successful
    bool moveToGhost(bool kill=false);
  };



  // Details for QUndoCommand derived classes

  class DesignPanel::CreateDB : public QUndoCommand
  {
  public:
    //! Create a dangling bond at the given lattice dot, set invert if deleting
    //! DB. Set src_db if copying DB.
    CreateDB(prim::LatticeCoord l_coord, int layer_index, DesignPanel *dp,
        prim::DBDot *cp_src=0, bool invert=false, QUndoCommand *parent=0);

    // destroy the dangling bond and update the lattice dot
    virtual void undo();

    // re-create the dangling bond
    virtual void redo();

  private:

    void create();    // create the dangling bond
    void destroy();   // destroy the dangling bond

    bool invert;      // swaps create/delete on redo/undo

    prim::LatticeCoord lat_coord;
    prim::DBDot *db_at_loc=0;
    prim::DBDot *cp_src;

    DesignPanel *dp;  // DesignPanel pointer
    int layer_index;  // index of layer in dp->layers stack

    // internals
    int index;        // index of DBDot item in the layer item stack
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


  class DesignPanel::CreatePotPlot : public QUndoCommand
  {
  public:
    // create an plot at the given points
    CreatePotPlot(gui::DesignPanel *dp, QString pot_plot_path, QRectF graph_container, QString pot_anim_path,
      prim::PotPlot *pp = 0, bool invert=false, QUndoCommand *parent=0);

  private:

    // destroy the dangling bond and update the lattice dot
    virtual void undo();
    // re-create the dangling bond
    virtual void redo();

    void create();  // create the dangling bond
    void destroy(); // destroy the dangling bond

    DesignPanel *dp;  // DesignPanel pointer
    QImage potential_plot;
    QString pot_plot_path;
    QRectF graph_container;
    QString pot_anim_path;
    prim::PotPlot* pp;
    bool invert;
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


  class DesignPanel::CreateTextLabel : public QUndoCommand
  {
  public:
    //! Create a text label
    CreateTextLabel(int layer_index, DesignPanel *dp, const QRectF &scene_rect,
                    const QString &text, prim::TextLabel *text_lab=0,
                    bool invert=false, QUndoCommand *parent=0);

    virtual void undo();
    virtual void redo();

  private:
    void create();
    void destroy();

    DesignPanel *dp;
    bool invert;
    int layer_index;    // index of layer in layer manager
    int item_index;     // index of item in layer
    QRectF scene_rect;  // rectangle that the label takes up in scene coords
    QString text;       // text contained in the label
  };

  class DesignPanel::EditTextLabel : public QUndoCommand
  {
  public:
    //! Specify text label to edit
    EditTextLabel(int layer_index, DesignPanel *dp, const QString &new_text,
                  prim::TextLabel *text_lab, bool invert=false,
                  QUndoCommand *parent=0);

    virtual void undo();
    virtual void redo();

  private:
    DesignPanel *dp;
    bool invert;
    int layer_index;    // index of layer in layer manager
    int item_index;     // index of item in layer
    QString text_orig;  // original text
    QString text_new;   // new text
  };

  //! Generic undoable item creation
  class DesignPanel::CreateItem : public QUndoCommand
  {
  public:
    CreateItem(int layer_index, DesignPanel *dp, prim::Item *item,
               bool invert=false, QUndoCommand *parent=0);

    ~CreateItem();

    virtual void undo();
    virtual void redo();

  private:
    void create();
    void destroy();

    DesignPanel *dp;
    bool invert;
    bool in_scene;
    int layer_index;  // index of layer in layer manager
    int item_index;   // index of item in layer
    prim::Item *item; // pointer to item, this pointer should always be valid for recreation
  };

  //! Resize a ResizableRect
  class DesignPanel::ResizeItem : public QUndoCommand
  {
  public:
    //! Set manual to true if the resize was done manually, which means the rect
    //! already has the correct dimensions.
    ResizeItem(int layer_index, DesignPanel *dp, int item_index,
               const QRectF &orig_rect, const QRectF &new_rect,
               bool manual=false, bool invert=false, QUndoCommand *parent=0);

    virtual void undo();
    virtual void redo();

  private:
    DesignPanel *dp;
    bool invert;
    bool manual;
    int layer_index;
    int item_index;
    QRectF orig_rect;
    QRectF new_rect;
    QPointF top_left_delta;
    QPointF bottom_right_delta;
  };

  //! Rotate a ResizeRotateRect
  class DesignPanel::RotateItem : public QUndoCommand
  {
  public:
    //! Set manual to true if the resize was done manually, which means the rect
    //! already has the correct dimensions.
    RotateItem(int layer_index, DesignPanel *dp, int item_index,
               double init_ang, double fin_ang, bool invert=false, QUndoCommand *parent=0);

    virtual void undo();
    virtual void redo();

  private:
    DesignPanel *dp;
    bool invert;
    int layer_index;
    int item_index;
    double init_ang;
    double fin_ang;
  };

  //! Change the colour of an item.
  class DesignPanel::ChangeColor : public QUndoCommand
  {
  public:
    //! Set manual to true if the resize was done manually, which means the rect
    //! already has the correct dimensions.
    ChangeColor(int layer_index, DesignPanel *dp, int item_index,
               QColor init_col, QColor fin_col, bool invert=false, QUndoCommand *parent=0);

    virtual void undo();
    virtual void redo();

  private:
    DesignPanel *dp;
    bool invert;
    int layer_index;
    int item_index;
    QColor init_col;
    QColor fin_col;
  };



} // end gui namespace

#endif
