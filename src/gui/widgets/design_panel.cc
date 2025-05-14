// @file:     design_panel.cc
// @author:   Jake
// @created:  2016.11.02
// @editted:  2017.07.11  - Jake
// @license:  GNU LGPL v3
//
// @desc:     DesignPanel implementation

#include "design_panel.h"
#include "settings/settings.h"

#include <algorithm>

QColor gui::DesignPanel::background_col;
QColor gui::DesignPanel::background_col_publish;
qreal gui::DesignPanel::zoom_visibility_threshold;

// constructor
gui::DesignPanel::DesignPanel(QWidget *parent)
  : QGraphicsView(parent)
{
  // set-up scale_factor in prim::Item
  prim::Item::init();

  // construct static variables at first launch
  if (!background_col.isValid())
    constructStatics();

  // initialize design panel
  initDesignPanel("");

  // initialize actions
  initActions();

  connect(prim::Emitter::instance(), &prim::Emitter::sig_selectClicked,
          this, &gui::DesignPanel::selectClicked);
  connect(prim::Emitter::instance(), &prim::Emitter::sig_showProperty,
          this, &gui::DesignPanel::showItemProperty);
  connect(prim::Emitter::instance(), &prim::Emitter::sig_addItemToScene,
          this, &gui::DesignPanel::addItemToScene);
  connect(prim::Emitter::instance(), &prim::Emitter::sig_removeItemFromScene,
          this, &gui::DesignPanel::removeItemFromScene);
  connect(prim::Emitter::instance(), &prim::Emitter::sig_resizeBegin,
          this, &gui::DesignPanel::resizeBegin);
  connect(prim::Emitter::instance(), &prim::Emitter::sig_resizeFinalizeRect,
          this, &gui::DesignPanel::resizeItemRect);
  connect(prim::Emitter::instance(), &prim::Emitter::sig_moveDBToLatticeCoord,
          this, &gui::DesignPanel::moveDBToLatticeCoord);
  connect(prim::Emitter::instance(), &prim::Emitter::sig_physLoc2LatticeCoord,
          this, &gui::DesignPanel::physLoc2LatticeCoord);
  connect(prim::Emitter::instance(), &prim::Emitter::sig_latticeCoord2PhysLoc,
          this, &gui::DesignPanel::latticeCoord2PhysLoc);
  connect(prim::Emitter::instance(), &prim::Emitter::sig_setLatticeVisibility,
          this, &gui::DesignPanel::updateBackground);
  connect(prim::Emitter::instance(), &prim::Emitter::sig_editTextLabel,
          this, QOverload<prim::Item*, const QString &>::of(&gui::DesignPanel::editTextLabel));
  connect(prim::Emitter::instance(), &prim::Emitter::sig_rotate,
          this, &gui::DesignPanel::showRotateDialog);
  connect(prim::Emitter::instance(), &prim::Emitter::sig_color_change,
          this, &gui::DesignPanel::showColorDialog);
}

// destructor
gui::DesignPanel::~DesignPanel()
{
  clearPlots();
  clearDesignPanel(false);
}

// initialise design panel on first init or after reset
void gui::DesignPanel::initDesignPanel(QString lattice_file_path, bool init_layers) {
  undo_stack = new QUndoStack();
  connect(undo_stack, &QUndoStack::cleanChanged,
          this, &DesignPanel::emitUndoStackCleanChanged);

  // initialize contained widgets
  layman = new LayerManager(this);
  property_editor = new PropertyEditor(this);
  itman = new ItemManager(this, layman);

  color_dialog = new ColorDialog(this);

  rotate_dialog = new RotateDialog(this);

  settings::AppSettings *app_settings = settings::AppSettings::instance();

  scene = new QGraphicsScene(this);
  setScene(scene);
  setMouseTracking(true);

  setAcceptDrops(true);

  // setup flags
  clicked = ghosting = moving = pasting = resizing = false;

  // initialising parameters
  snap_diameter = app_settings->get<qreal>("snap/diameter")*prim::Item::scale_factor;
  qDebug() << tr("SD: %1").arg(snap_diameter);
  snap_coord = prim::LatticeCoord();

  tool_type = gui::ToolType::NoneTool;     // now setTool will update the tool

  sim_results_items.clear();

  // set view behaviour
  setTransformationAnchor(QGraphicsView::NoAnchor);
  setResizeAnchor(QGraphicsView::AnchorViewCenter);
  resetTransform(); // resets QTransform, which undoes the zoom
  scale(0.1, 0.1);

  setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

  // color scheme
  QColor col;
  setFrameShadow(QFrame::Raised);

  setCacheMode(QGraphicsView::CacheBackground);

  // make lattice and surface layer
  buildLattice(lattice_file_path);
  initOverlays();
  if (init_layers) {
    initLayers();
  }
  setSceneMinSize();

  // initialise the Ghost and set the scene
  prim::Ghost::instance()->setScene(scene);

  // initialise scroll bar position and policies
  verticalScrollBar()->setValue((verticalScrollBar()->minimum()+verticalScrollBar()->maximum())/2);
  horizontalScrollBar()->setValue((horizontalScrollBar()->minimum()+horizontalScrollBar()->maximum())/2);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);


  // construct widgets

  // initialize widgets which depend on other things to be initialized first
  
  screenman = new ScreenshotManager(layman->indexOf(layman->getLayer("Screenshot Overlay")), this);

  // ScreenshotManager signals
  connect(screenman, &gui::ScreenshotManager::sig_takeScreenshot,
      [this](const QString &target_img_path, const QRectF &scene_rect, bool always_overwrite) {
        emit sig_screenshot(target_img_path, scene_rect, always_overwrite);
      }
      );
  connect(screenman, &gui::ScreenshotManager::sig_clipSelectionTool,
      [this]() {emit sig_toolChangeRequest(gui::ScreenshotAreaTool);});
  connect(screenman, &gui::ScreenshotManager::sig_addVisualAidToDP,
      [this](prim::Item *t_item) {
        addItem(t_item, layman->getLayer("Screenshot Overlay")->layerID());
      });
  connect(screenman, &gui::ScreenshotManager::sig_removeVisualAidFromDP,
      [this](prim::Item *t_item) {
        removeItem(t_item, layman->getLayer("Screenshot Overlay")->layerID(), true);
      });
  connect(screenman, &gui::ScreenshotManager::sig_scaleBarAnchorTool,
      [this]() {sig_toolChangeRequest(gui::ScaleBarAnchorTool);});
  connect(screenman, &gui::ScreenshotManager::sig_closeEventTriggered,
      [this]() {emit sig_cancelScreenshot();});

  connect(itman, &gui::ItemManager::sig_deselect,
          this, &gui::DesignPanel::deselectAll);
  connect(itman, &gui::ItemManager::sig_delete_selected,
          this, &gui::DesignPanel::deleteAction);

  connect(color_dialog, &QColorDialog::colorSelected,
          this, &gui::DesignPanel::changeItemColors);

  connect(rotate_dialog, &QInputDialog::doubleValueSelected,
          this, &gui::DesignPanel::setItemRotations);

  emit sig_setItemManagerWidget(itman);


  // set display mode
  setDisplayMode(DesignMode);

  // initialize scene rect for the current viewport
  updateSceneRect();
}

void gui::DesignPanel::deselectAll()
{
  scene->clearSelection();
  // qDebug() << "Deselecting All";
}

// clear design panel
void gui::DesignPanel::clearDesignPanel(bool reset)
{
  // destroy DB previews
  destroyDBPreviews();

  // delete child widgets
  delete screenman;
  delete property_editor;
  delete layman;
  delete itman;
  delete color_dialog;
  delete rotate_dialog;

  screenman=nullptr;
  property_editor=nullptr;
  layman=nullptr;
  itman=nullptr;
  lattice=nullptr;

  // delete layers and contained items
  if(reset) prim::Layer::resetLayers(); // reset layer counter


  // delete all graphical items from the scene
  scene->clear();
  delete scene;

  // purge the clipboard
  for(prim::Item *item : clipboard)
    delete item;
  clipboard.clear();

  // disconnect this signal first otherwise segfaults
  disconnect(undo_stack, &QUndoStack::cleanChanged,
           this, &DesignPanel::emitUndoStackCleanChanged);
  delete undo_stack;

  qDebug() << tr("Finished clearing design panel");
}

// reset
void gui::DesignPanel::resetDesignPanel(QString lattice_file_path, bool init_layers)
{
  // tell application to perform pre-reset clean-ups
  emit sig_preDPResetCleanUp();

  clearDesignPanel(true);
  initDesignPanel(lattice_file_path, init_layers);
  // REBUILD

  //let application know that design panel has been reset.
  emit sig_postDPReset();
  qDebug() << tr("Design Panel reset complete");
}


// ACCESSORS

void gui::DesignPanel::addItem(prim::Item *item, int layer_index, int ind)
{
  // check valid layer index, should not allow access to lattice layer
  if(layer_index == 0 || layer_index >= layman->layerCount()){
    qCritical() << tr("Invalid layer index");
    return;
  }

  //prim::Layer *layer = layer_index > 0 ? layers.at(layer_index) : top_layer;
  prim::Layer *layer = layer_index > 0 ? layman->getLayer(layer_index) : layman->activeLayer();
  if(ind > layer->getItems().count()){
    qCritical() << tr("Invalid item index");
    return;
  }

  // record position for screen drift correction
  QPointF old_pos(mapToScene(mapFromParent(rect().center())));

  // add Item
  layer->addItem(item, ind);
  scene->addItem(item);

  updateSceneRect();

  // correct screen shift
  QPointF new_pos(mapToScene(mapFromParent(rect().center())));
  scrollDelta(new_pos - old_pos);

  // update item manager
  itman->updateTableAdd();
}

void gui::DesignPanel::removeItem(prim::Item *item, int layer_index, bool retain_item)
{
  removeItem(item, layman->getLayer(layer_index), retain_item);
}

void gui::DesignPanel::removeItem(prim::Item *item, prim::Layer *layer, bool retain_item)
{
  // if layer contains the item, delete and remove froms scene, otherwise
  // do nothing
  if(layer->removeItem(item)){
    // record position for screen drift correction
    QPointF old_pos(mapToScene(mapFromParent(rect().center())));

    // remove the item
    scene->removeItem(item);
    if (!retain_item)
      delete item;

    updateSceneRect();

    // correct screen shift
    QPointF new_pos(mapToScene(mapFromParent(rect().center())));
    scrollDelta(new_pos - old_pos);

    emit sig_itemRemoved(item);
  }

  // update item manager
  itman->updateTableRemove(item);
}

void gui::DesignPanel::addItemToScene(prim::Item *item)
{
  scene->addItem(item);
}

void gui::DesignPanel::removeItemFromScene(prim::Item *item)
{
  scene->removeItem(item);
  // item pointer delete should be handled by the caller
}

void gui::DesignPanel::updateSceneRect(const QRectF &expand_to_include)
{
  QRectF sbr = scene->itemsBoundingRect();
  QPointF vp_tl = mapToScene(mapFromParent(rect().topLeft()));
  QPointF vp_br = mapToScene(mapFromParent(rect().bottomRight()));
  //QRectF vp = mapToScene(viewport()->rect()).boundingRect();
  //qDebug() << "Old style:" << vp;
  QRectF vp(vp_tl, vp_br);
  //qDebug() << "New style:" << vp;
  if (expand_to_include.isNull()) {
    scene->setSceneRect(min_scene_rect | sbr | vp);
  } else {
    scene->setSceneRect(min_scene_rect | sbr | vp | expand_to_include);
  }
  //scene->setSceneRect(min_scene_rect | sbr);
}

void gui::DesignPanel::fitItemsInView(const bool &include_hidden)
{
  QRectF rect;
  for (QGraphicsItem *item : scene->items()) {
    if (include_hidden || item->isVisible()) {
      rect |= item->sceneBoundingRect();
      qDebug() << rect;
    }
  }
  fitInView(rect, Qt::KeepAspectRatio);
  updateBackground();
  informZoomUpdate();
}


QList<prim::Item*> gui::DesignPanel::selectedItems()
{
  QList<prim::Item*> casted_list;
  for (QGraphicsItem *gitem : scene->selectedItems())
    casted_list.append(static_cast<prim::Item*>(gitem));
  emit sig_selectedItems(casted_list);
  return casted_list;
}


QList<prim::DBDot*> gui::DesignPanel::getAllDBs() const
{
  QList<prim::Layer*> db_layers = layman->getLayers(prim::Layer::DB);
  if (db_layers.isEmpty()) {
    qDebug() << tr("No DB layers found");
    return QList<prim::DBDot*>();
  }

  QList<prim::DBDot*> dbs;
  for (prim::Layer *lay : db_layers) {
    if (lay->role() != prim::Layer::Design)
      continue;
    dbs += static_cast<prim::DBLayer*>(lay)->getDBs();
  }
  return dbs;
}


void gui::DesignPanel::buildLattice(QString fname)
{
  if (fname.isEmpty()) {
    fname = settings::LatticeSettings::instance()->get<QString>("lattice/default_lattice_file_path");
  }
  qDebug() << tr("Build lattice from file: %1").arg(fname);

  if (layman->layerCount() != 0) {
    // if layer manager not empty, clear it
    layman->removeAllLayers();
  }

  QFile lattice_file(fname);
  QXmlStreamReader rs(&lattice_file);
  if (!lattice_file.open(QFile::ReadOnly | QFile::Text)) {
    qCritical() << tr("Cannot open lattice file at path: %1").arg(fname);
    return;
  }

  // LATTICE MUST BE LAYER 0

  // build the new lattice
  // lattice = new prim::Lattice(fname, 0);
  rs.readNextStartElement();
  lattice = new prim::Lattice(&rs);

  // add the lattice to the layers, as layer 0
  layman->addLattice(lattice);

  // the 2nd lattice is for result display
  layman->addLattice(new prim::Lattice((*lattice), 0), prim::Layer::Result);
  updateBackground();
}


void gui::DesignPanel::initLayers()
{
  bool only_add_missing_defaults = false;

  if (layman->layerCount() != 0) {
    qDebug() << tr("Layers already exist on the design panel, therefore only "
      "missing basic layers are added.");
    only_add_missing_defaults = true;
  }

  // add in the dangling bond surface
  if (!only_add_missing_defaults || layman->getLayers(prim::Layer::DB).size() == 0) {
    layman->addDBLayer(lattice, "Surface", prim::Layer::Design);
  }

  // add in the metal layer for electrodes
  if (!only_add_missing_defaults || layman->getLayers(prim::Layer::Electrode).size() == 0) {
    layman->addLayer("Metal", prim::Layer::Electrode, prim::Layer::Design, 1000, 100);
  }

  layman->populateLayerTable();
  layman->initSideWidget();
  emit sig_setLayerManagerWidget(layman->sideWidget());

  layman->setActiveLayer(layman->getLayer("Surface"));
}

void gui::DesignPanel::initOverlays()
{
  // add the screenshot misc items layer
  if (layman->getLayer("Screenshot Overlay") != 0) {
    qWarning() << "Screenshot Overlay layer already exists during overlay "
      "initialization, shouldn't be the case.";
  }
  layman->addLayer("Screenshot Overlay", prim::Layer::Misc, prim::Layer::Overlay, 0, 0);
}



void gui::DesignPanel::setSceneMinSize()
{
  // add an invisible rectangle to the scene to set a minimum scene rect
  int min_size = settings::GUISettings::instance()->get<int>("lattice/minsize");
  QPoint bot_right = min_size * (lattice->sceneLatticeVector(0) + lattice->sceneLatticeVector(1));
  min_scene_rect = QRectF(QPoint(0,0),bot_right);
  min_scene_rect.moveCenter(QPoint(0,0));
  //scene->addItem(new QGraphicsRectItem(scene_rect));
  scene->setSceneRect(min_scene_rect);
}


void gui::DesignPanel::setTool(gui::ToolType tool)
{
  // do nothing if tool has not been changed
  if(tool==tool_type)
    return;

  // reset selected items
  scene->clearSelection();

  // emit empty selected items list
  emit sig_selectedItems(QList<prim::Item*>());

  // destroy DB previews
  destroyDBPreviews();

  // inform all items of select mode
  prim::Item::tool_type = tool;

  switch(tool){
    case gui::ToolType::SelectTool:
      setDragMode(QGraphicsView::NoDrag);
      setInteractive(true);
      break;
    case gui::ToolType::DragTool:
      setDragMode(QGraphicsView::ScrollHandDrag);
      setInteractive(false);
      break;
    case gui::ToolType::DBGenTool:
      layman->setActiveLayer(layman->getMRULayer(prim::Layer::DB));
      setDragMode(QGraphicsView::NoDrag);
      setInteractive(true);
      break;
    case gui::ToolType::ElectrodeTool:
      layman->setActiveLayer(layman->getMRULayer(prim::Layer::Electrode));
      setDragMode(QGraphicsView::NoDrag);
      setInteractive(true);
      break;
    case gui::ToolType::ScreenshotAreaTool:
      setInteractive(true);
      screenman->setClipVisibility(true, true);
      break;
    case gui::ToolType::ScaleBarAnchorTool:
      setInteractive(true);
      screenman->setScaleBarVisibility(true, true);
      break;
    case gui::ToolType::LabelTool:
      setInteractive(true);
      break;
    default:
      qCritical() << tr("Invalid ToolType... should not have happened");
      return;
  }

  tool_type = tool;
  emit sig_toolChanged(tool);
}

void gui::DesignPanel::setFills(float *fills)
{
  QList<prim::DBDot *> dbs = getAllDBs();
  for (int i=0; i<dbs.count(); i++) {
    if(qAbs(fills[i])>1.)
      qWarning() << tr("Given fill invalid");
    else
      dbs.at(i)->setFill(fills[i]);
  }
}


void gui::DesignPanel::screenshot(QPainter *painter, const QRectF &region, const QRectF &outrect)
{
  // add lattice dot previews (vector graphics) instead of using the bitmap
  // include lattice background if layer is not hidden
  QList<prim::LatticeDotPreview*> latdot_previews;
  prim::Lattice *lat = static_cast<prim::Lattice*>(layman->getLayer(0, !layman->isSimLayerMode()));
  if (lat->isVisible()) {
    QList<prim::LatticeCoord> coords = lat->enclosedSites(region);
    for (prim::LatticeCoord coord : coords) {
      if (lat->isOccupied(coord))
        continue;
      prim::LatticeDotPreview *ldp = new prim::LatticeDotPreview(coord);
      ldp->setPos(lat->latticeCoord2ScenePos(coord));
      ldp->setZValue(INT_MIN);
      latdot_previews.append(ldp);
      scene->addItem(ldp);
    }
  }

  bool clip_reactivate = screenman->clipVisible();
  if (clip_reactivate)
    screenman->setClipVisibility(false, false);

  // render scene onto painter
  scene->render(painter, outrect, region);

  if (clip_reactivate)
    screenman->setClipVisibility(true, false);

  // remove lattice dot previews
  while (!latdot_previews.isEmpty()) {
    prim::LatticeDotPreview *ldp = latdot_previews.takeLast();
    scene->removeItem(ldp);
    delete ldp;
  }
}


void gui::DesignPanel::setDisplayMode(DisplayMode mode)
{
  display_mode = mode;
  prim::Item::display_mode = mode;

  screenman->prepareScreenshotMode(display_mode == ScreenshotMode);

  updateBackground();
}


// SAVE

void gui::DesignPanel::writeToXmlStream(QXmlStreamWriter *ws,
                                        DesignInclusionArea inclusion_area)
{
  // TODO implement inclusion area

  // save gui flags
  ws->writeComment("GUI Flags");
  ws->writeStartElement("gui");

  // save zoom and scroll bar position
  ws->writeTextElement("zoom", QString::number(transform().m11() + transform().m12())); // m11 of qtransform
  // save center scene position in angstroms (>= v0.2.2)
  ws->writeEmptyElement("displayed_region");
  QPointF tlpt = mapToScene(mapFromParent(rect().topLeft()));
  QPointF brpt = mapToScene(mapFromParent(rect().bottomRight()));
  ws->writeAttribute("x1", QString::number(tlpt.x() / prim::Item::scale_factor));
  ws->writeAttribute("y1", QString::number(tlpt.y() / prim::Item::scale_factor));
  ws->writeAttribute("x2", QString::number(brpt.x() / prim::Item::scale_factor));
  ws->writeAttribute("y2", QString::number(brpt.y() / prim::Item::scale_factor));
  // scroll position for legacy support (<= v0.2.1)
  ws->writeEmptyElement("scroll");
  ws->writeAttribute("x", QString::number(verticalScrollBar()->value()));
  ws->writeAttribute("y", QString::number(horizontalScrollBar()->value()));

  ws->writeEndElement();  // end of gui node

  // save layer properties
  ws->writeComment("Layer Properties");
  ws->writeComment("Layer ID is intrinsic to the layer order");
  ws->writeStartElement("layers");
  layman->saveLayers(ws);
  ws->writeEndElement();

  // save item hierarchy
  ws->writeComment("Item Hierarchy");
  ws->writeStartElement("design");
  layman->saveLayerItems(ws, inclusion_area);
  ws->writeEndElement(); // end of design node
}

void gui::DesignPanel::loadFromFile(QXmlStreamReader *rs, bool is_sim_result)
{
  if (!is_sim_result) {
    // reset the design panel state
    resetDesignPanel("", false);
  }

  QList<int> layer_order_id;
  QRectF visrect;

  // read from xml stream and hand nodes off to appropriate functions
  while (rs->readNextStartElement()) {
    QString elem_name = rs->name().toString();
    // read program flags
    if (elem_name == "program") {
      // TODO implement:
      // file_purpose: pop-up message when loading simulation or autosave, also unhide layers as needed (e.g. if autosaved in sim display mode where all designs are hidden)
      // version: no action needed so far
      // date: no action needed so far
      rs->skipCurrentElement();
    } else if(elem_name == "gui") {
      if (!is_sim_result) {
        loadGUIFlags(rs, visrect);
      } else {
        rs->skipCurrentElement();
      }
    } else if (elem_name == "layers") {
      loadLayers(rs, layer_order_id, is_sim_result);
    } else if(elem_name == "layer_prop") {
      // LEGACY support for files < v0.0.2
      // starting from v0.0.2 layer_prop should appear inside the layers level
      loadLayerProps(rs, layer_order_id, is_sim_result);
    } else if(elem_name == "design") {
      loadDesign(rs, layer_order_id, is_sim_result);
    } else {
      qDebug() << tr("Design Panel: invalid element encountered on line %1 - %2")
          .arg(rs->lineNumber()).arg(elem_name);
      rs->skipCurrentElement();
    }
  }

  // show error if any
  if(rs->hasError()){
    qCritical() << tr("XML error: ") << rs->errorString().data();
  }

  if (!is_sim_result) {
    initLayers();   // init missing layers
  }

  if (!is_sim_result) {
    updateSceneRect(visrect);

    // apply visual settings last if relying on display area (>= v0.2.2)
    if (!visrect.isNull()) {
      fitInView(visrect, Qt::KeepAspectRatio);
      qDebug() << "Display area set to top left=" << visrect.topLeft() 
        << " and bottom right=" << visrect.bottomRight();
    }

    // zoom related
    updateBackground();
    informZoomUpdate();
  }

  //make sure all layers from the loaded file are present in the advanced layer table
  if (!is_sim_result) {
    layman->populateLayerTable();
  }
}


void gui::DesignPanel::loadGUIFlags(QXmlStreamReader *rs, QRectF &visrect)
{
  qDebug() << "Loading GUI flags";
  qreal zoom=0.1, scroll_v=0, scroll_h=0;
  QPointF tlpt, brpt;
  while (rs->readNextStartElement()) {
    QString elem_name = rs->name().toString();
    if (elem_name == "zoom") {
      zoom = rs->readElementText().toDouble();
    } else if (elem_name == "scroll") {
      scroll_v = rs->attributes().value("x").toInt();
      scroll_h = rs->attributes().value("y").toInt();
      // no text is being read so the current element has to be explicitly skipped
      rs->skipCurrentElement();
    } else if (elem_name == "displayed_region") {
      tlpt.setX(rs->attributes().value("x1").toFloat());
      tlpt.setY(rs->attributes().value("y1").toFloat());
      brpt.setX(rs->attributes().value("x2").toFloat());
      brpt.setY(rs->attributes().value("y2").toFloat());
      rs->skipCurrentElement();
    } else {
      qDebug() << tr("Design Panel: invalid element encountered on line %1 - %2")
          .arg(rs->lineNumber()).arg(elem_name);
      rs->skipCurrentElement();
    }
  }
  // set scene position: use scene center point if present, else use scroll bar pos
  if (!(tlpt.isNull() && brpt.isNull())) {
    visrect = QRectF(tlpt * prim::Item::scale_factor, brpt * prim::Item::scale_factor);
  } else {
    setTransform(QTransform(zoom,0,0,zoom,0,0));
    verticalScrollBar()->setValue(scroll_v);
    horizontalScrollBar()->setValue(scroll_h);
    qDebug() << tr("Set zoom=%1, scrollbar positions v=%2, h=%3").arg(zoom).arg(scroll_v).arg(scroll_h);
  }
  updateBackground();
}


void gui::DesignPanel::loadLayers(QXmlStreamReader *rs, 
    QList<int> &layer_order_id, bool is_sim_result)
{
  qDebug() << "Loading layers";
  while (rs->readNextStartElement()) {
    if (rs->name().toString() == "layer_prop") {
      loadLayerProps(rs, layer_order_id, is_sim_result);
    }
  }
}


void gui::DesignPanel::loadLayerProps(QXmlStreamReader *rs, 
    QList<int> &layer_order_id, bool is_sim_result)
{
  QString layer_nm;
  float zoffset=0, zheight=0;
  prim::Layer::LayerType layer_type = prim::Layer::DB;
  prim::Layer::LayerRole layer_role = prim::Layer::Design;
  bool layer_visible=false, layer_active=false;

  // keep reading until end of layer_prop tag
  while (rs->readNextStartElement()) {
    QString elem_name = rs->name().toString();
    if (elem_name == "name") {
      layer_nm = rs->readElementText();
    } else if (elem_name == "type") {
      layer_type = static_cast<prim::Layer::LayerType>(
          QMetaEnum::fromType<prim::Layer::LayerType>().keyToValue(
            rs->readElementText().toStdString().c_str()));
    } else if (elem_name == "role") {
      layer_role = static_cast<prim::Layer::LayerRole>(
          QMetaEnum::fromType<prim::Layer::LayerRole>().keyToValue(
            rs->readElementText().toStdString().c_str()));
    } else if (elem_name == "zoffset") {
      zoffset = rs->readElementText().toFloat();
    } else if (elem_name == "zheight") {
      zheight = rs->readElementText().toFloat();
    } else if (elem_name == "visible") {
      layer_visible = (rs->readElementText() == "1") ? true : false;
    } else if (elem_name == "active") {
      layer_active = (rs->readElementText() == "1") ? true : false;
    } else if (elem_name == "lat_vec") {
      prim::Lattice *lat_lay = layman->getLattice(!is_sim_result);
      lat_lay->constructFromXml(rs);
    } else {
      qDebug() << tr("Design Panel: invalid element encountered on line %1 - %2")
          .arg(rs->lineNumber()).arg(elem_name);
      rs->skipCurrentElement();
    }
  }

  if (is_sim_result) {
    if (layer_role != prim::Layer::Design) {
      qDebug() << tr("Loading design from simulation problem, skipping layer %1"
          " as it is not of Design role.").arg(layer_nm);
      layer_order_id.append(-1);
      return;
    } else {
      layer_role = prim::Layer::Result;
    }
  }

  if (layer_role == prim::Layer::Overlay) {
    layer_order_id.append(-1);
    qDebug() << tr("Overlay layer role encountered, skipping this load.");
    return;
  }

  prim::Layer *lay;
  switch(layer_type){
    case prim::Layer::Lattice:
    {
      lay = layman->getLattice(!is_sim_result);
      lay->setZOffset(zoffset);
      lay->setZHeight(zheight);
      break;
    }
    case prim::Layer::DB:
    {
      prim::Lattice *lat = layman->getLattice(!is_sim_result);
      if (lat==nullptr) {
        qFatal("Lattice must not be a null pointer at the creation of a DB layer.");
      }
      lay = layman->addDBLayer(lat, layer_nm, layer_role);
      break;
    }
    default:
    {
      lay = layman->addLayer(layer_nm, layer_type, layer_role, zoffset, zheight);
      break;
    }
  }

  if (lay==nullptr) {
    qFatal("Layer initialization unsuccessful, please check the logs.");
  }

  lay->setVisible(layer_visible);
  lay->setActive(layer_active);

  layer_order_id.append(lay->layerID());
  qDebug() << tr("Loaded Layer %1 at ID %2").arg(lay->getName()).arg(lay->layerID());

  /*
  // edit layer if it exists, create new otherwise
  if (!is_sim_result) {
    qDebug() << tr("Loading layer %1 with type %2").arg(layer_nm).arg(layer_type);
    // TODO rethink this layer loading method
    prim::Layer* load_layer = layman->getLayer(layer_nm);
    if (!load_layer) {
      qDebug() << tr("Created layer %1 instead").arg(layer_nm);
      layman->addLayer(layer_nm);
      load_layer = layman->getLayer(layman->layerCount()-1);
    }
    load_layer->setContentType(layer_type);
    load_layer->setZOffset(zoffset);
    load_layer->setZHeight(zheight);
    load_layer->setVisible(layer_visible);
    load_layer->setActive(layer_active);
    layer_order_id.append(load_layer->layerID());
  } else {
    if (layer_role != prim::Layer::Design) {
      qDebug() << tr("Loading design from simulation problem, skipping layer %1"
          " as it is not of Design role.").arg(layer_nm);
      return;
    }
    prim::Layer *lay;
    switch(layer_type){
      case prim::Layer::Lattice:
        break;
      case prim::Layer::DB:
        break;
      default:
        lay = new Layer(
        break;
    }
  }
  */
}


void gui::DesignPanel::loadDesign(QXmlStreamReader *rs, QList<int> &layer_order_id,
    bool is_sim_result)
{
  qDebug() << "Loading design";
  int layer_load_order=0;
  while (rs->readNextStartElement()) {
    if (rs->name().toString() == "layer") {
      if (layer_order_id[layer_load_order] == -1) {
        layer_load_order++;
        rs->skipCurrentElement();
        continue;
      }
      // recursively populate layer with items
      //rs->readNext();
      layman->getLayer(layer_order_id[layer_load_order], !is_sim_result)->loadItems(rs, scene);
      layer_load_order++;
    } else {
      qDebug() << tr("Design Panel: invalid element encountered on line %1 - %2")
          .arg(rs->lineNumber()).arg(rs->name().toString());
      rs->skipCurrentElement();
    }
  }

  itman->updateTableAdd();
}


// SIMULATION RESULT DISPLAY

void gui::DesignPanel::enableSimVis()
{
  layman->setSimVisualizeMode(true);
}

void gui::DesignPanel::clearSimResults()
{
  layman->setSimVisualizeMode(false);

  setDisplayMode(DesignMode);

  // TODO remove the following code when all simulation related item handling
  // are moved over to SimVisualizer
  // set show_elec of all DBDots to 0
  while(!sim_results_items.isEmpty()){
    prim::Item* temp_item = sim_results_items.takeFirst();
    removeItemFromScene(temp_item);
    delete temp_item;
  }
}

void gui::DesignPanel::clearPlots()
{
  setDisplayMode(DesignMode);
  for (prim::Item* temp_item: sim_results_items) {
    if (temp_item->item_type == prim::Item::PotPlot) {
      prim::PotPlot *pp = static_cast<prim::PotPlot*>(temp_item);
      undo_stack->push(new CreatePotPlot(this,
          pp->getPotPlotPath(), pp->getGraphContainer(), pp->getAnimPath(),
          static_cast<prim::PotPlot*>(temp_item), true));
      removeItemFromScene(temp_item);
      delete temp_item;
    }
  }
}

void gui::DesignPanel::displayPotentialPlot(QString pot_plot_path, QRectF graph_container, QString pot_plot_anim)
{
  qDebug() << tr("graph_container height: ") << graph_container.height();
  qDebug() << tr("graph_container width: ") << graph_container.width();
  qDebug() << tr("graph_container topLeft: ") << graph_container.topLeft().x() << tr(", ") << graph_container.topLeft().y();
  clearPlots();
  setDisplayMode(SimDisplayMode);
  createPotPlot(pot_plot_path, graph_container, pot_plot_anim);
}

// SLOTS

void gui::DesignPanel::selectClicked(prim::Item *)
{
  // for now, if an item is clicked in selection mode, act as though the highest
  // level aggregate was clicked
  if(tool_type == gui::ToolType::SelectTool && display_mode == DesignMode)
    initMove();

}

void gui::DesignPanel::resizeBegin()
{
  qDebug() << "resizeBegin()";
  resizing = true;
}

void gui::DesignPanel::rotateCw()
{
  QPointF old_center = mapToScene(viewport()->rect().center());
  rotate(90);

  // recenter the view to the original center before the rotation
  QPointF delta = mapToScene(viewport()->rect().center()) - old_center;
  scrollDelta(delta);
}

void gui::DesignPanel::rotateCcw()
{
  QPointF old_center = mapToScene(viewport()->rect().center());
  rotate(-90);

  // recenter the view to the original center before the rotation
  QPointF delta = mapToScene(viewport()->rect().center()) - old_center;
  scrollDelta(delta);
}

void gui::DesignPanel::moveDBToLatticeCoord(prim::Item *item, int n, int m, int l)
{
  item->setPos(lattice->latticeCoord2ScenePos(prim::LatticeCoord(n,m,l)));
}

void gui::DesignPanel::physLoc2LatticeCoord(QPointF physloc, int &n, int &m, int &l)
{
  prim::LatticeCoord coord = lattice->nearestSite(physloc, false);
  n = coord.n;
  m = coord.m;
  l = coord.l;
}

void gui::DesignPanel::latticeCoord2PhysLoc(int n, int m, int l, QPointF &physloc)
{
  prim::LatticeCoord coord(n, m, l);
  physloc = lattice->latticeCoord2PhysLoc(coord);
}

void gui::DesignPanel::updateBackground()
{
  QColor col = (display_mode == gui::ScreenshotMode) ? background_col_publish : background_col;
  bool lattice_visible = true;
  prim::Lattice *lat = layman->getLattice(!layman->isSimLayerMode());

  if (qAbs(transform().m11() + transform().m12()) < zoom_visibility_threshold
      || !lat->isVisible()) {
    lattice_visible = false;
  }

  if (lattice_visible)
    scene->setBackgroundBrush(QBrush(lat->tileableLatticeImage(col, display_mode == gui::ScreenshotMode)));
  else
    scene->setBackgroundBrush(QBrush(col));
}

void gui::DesignPanel::editTextLabel(prim::Item *text_lab,
                                     const QString &new_text)
{
  editTextLabel(reinterpret_cast<prim::TextLabel*>(text_lab), new_text);
}

// INTERRUPTS

// most behaviour will be connected to mouse move/release. However, when
// ghosting (dragging items or copy/paste), show the ghost on the left button
// press
void gui::DesignPanel::mousePressEvent(QMouseEvent *e)
{
  Qt::KeyboardModifiers keymods = QApplication::keyboardModifiers();

  // set clicked flag and store current mouse position for move behaviour
  clicked = true;
  press_scene_pos = mapToScene(e->pos()).toPoint();
  prev_pan_pos = e->pos();

  // if other buttons are clicked during rubber band selection, end selection
  if(rb)
    rubberBandClear();

  switch(e->button()){
    case Qt::LeftButton:
      if (tool_type == ScaleBarAnchorTool) {
        screenman->setScaleBarAnchor(mapToScene(e->pos()));
      } else if (tool_type == ScreenshotAreaTool) {
        // use rubberband to select screenshot area
        rb_start = mapToScene(e->pos()).toPoint();
        rb_cache = e->pos();
      } else if (tool_type == SelectTool || tool_type == ElectrodeTool ||
          tool_type == LabelTool) {
        // rubber band variables
        rb_start = mapToScene(e->pos()).toPoint();
        rb_cache = e->pos();

        if (keymods & Qt::ShiftModifier) {
          // save current selection if Shift is pressed
          rb_shift_selected = scene->selectedItems();
        } else {
          // pass the press event on to the view
          QGraphicsView::mousePressEvent(e);
        }

      } else if (tool_type == DBGenTool) {
        // start the rubber band at the snap_target if it exists
        /* TODO implement when trying to do lattice dot rubber band creation
        if (tool_type == DBGenTool && snap_target)
          rb_start = snap_target->pos().toPoint();
        else*/
        rb_start = mapToScene(e->pos()).toPoint();
        rb_cache = e->pos();
        coord_start = lattice->nearestSite(mapToScene(e->pos()), true);

      } else {
        QGraphicsView::mousePressEvent(e);
      }
      break;
    case Qt::MiddleButton:
      break;
    case Qt::RightButton: //prevent deselection of items
      break;
    default:
      QGraphicsView::mousePressEvent(e);
      break;
  }
}


// action depends on the status of buttons and keys pressed. If ghosting, we want
// to show the ghost snapping to the nearest snap point to the cursor. Otherwise,
// if a button is clicked, we are either selecting or panning the window. Reserve
// the middle mouse button to always pan and right click for context menus.
void gui::DesignPanel::mouseMoveEvent(QMouseEvent *e)
{
  QPoint mouse_pos_del;
  qreal dx, dy;
  emit sig_cursorPhysLoc(mapToScene(e->pos()) / prim::Item::scale_factor_nm);

  if (ghosting) {
    // update snap
    QPointF scene_pos = mapToScene(e->pos());
    prim::LatticeCoord offset;
    if (snapGhost(scene_pos, offset)) // if there are db dots
      prim::Ghost::instance()->moveByCoord(offset, lattice);
  } else if (!clicked && tool_type == DBGenTool) {
    QPoint cursor_pos = mapToScene(e->pos()).toPoint();
    QPoint cursor_offset = cursor_pos - press_scene_pos;
    if (cursor_offset.manhattanLength() > snap_diameter) {
      // show preview location of new DB
      createDBPreviews({lattice->nearestSite(mapToScene(e->pos()), true)});
      press_scene_pos = cursor_pos;
    }
  } else if (clicked) {
    // not ghosting, mouse dragging of some sort
    switch(e->buttons()){
      case Qt::LeftButton:
        if (tool_type == SelectTool || tool_type == ElectrodeTool ||
            tool_type == ScreenshotAreaTool || tool_type == LabelTool) {
          rubberBandUpdate(e->pos());
        } else if (tool_type == DBGenTool) {
          createDBPreviews(lattice->enclosedSites(coord_start, lattice->nearestSite(mapToScene(e->pos()), true)));
        }
        // use default behaviour for left mouse button
        QGraphicsView::mouseMoveEvent(e);
        break;
      case Qt::MiddleButton:
        {
        // middle button always pans
        mouse_pos_del = e->pos()-prev_pan_pos;
        dx = mouse_pos_del.x();
        dy = mouse_pos_del.y();
        qreal xf = horizontalScrollBar()->value() - dx;
        qreal yf = verticalScrollBar()->value() - dy;
        if (xf > horizontalScrollBar()->maximum())
          horizontalScrollBar()->setMaximum(xf);
        else if (xf < horizontalScrollBar()->minimum())
          horizontalScrollBar()->setMinimum(xf);
        else if (yf > verticalScrollBar()->maximum())
          verticalScrollBar()->setMaximum(yf);
        else if (yf < verticalScrollBar()->minimum())
          verticalScrollBar()->setMinimum(yf);
        verticalScrollBar()->setValue(verticalScrollBar()->value()-dy);
        horizontalScrollBar()->setValue(horizontalScrollBar()->value()-dx);
        prev_pan_pos = e->pos();
        break;
        }
      case Qt::RightButton:
        // right button will be for context menus in future
        break;
      case Qt::NoButton:
        // do nothing if no button is pressed... display coords??
        break;
      default:
        // multi button behavour not implemented
        break;
    }
  }
  QGraphicsView::mouseMoveEvent(e);
}

void gui::DesignPanel::mouseReleaseEvent(QMouseEvent *e)
{
  // for now, default behaviour first
  QGraphicsView::mouseReleaseEvent(e);
  // QPointF scene_pos = mapToScene(e->pos());
  QTransform trans = transform();

  if (rb)
    rubberBandSelect();

  // case specific behaviour
  if (ghosting) {
    // plant ghost and end ghosting
    if (moving)
      moveToGhost();
    else
      pasteAtGhost();
    clearGhost();
  }
  else if (clicked) {
    switch(e->button()){
      case Qt::LeftButton:
        // action based on chosen tool
        switch(tool_type){
          case gui::ToolType::SelectTool:
            // call selected items so the updated item list is emited
            selectedItems();
            break;
          case gui::ToolType::DBGenTool:
            // identify free lattice sites and create dangling bonds
            createDBs();
            break;
          case gui::ToolType::ElectrodeTool:
            // get start and end locations, and create the electrode.
            createElectrode(rb_scene_rect);
            break;
          case gui::ToolType::ScreenshotAreaTool:
          {
            // set the screenshot clip area in the screenshot manager
            screenman->setClipArea(rb_scene_rect);
            break;
          }
          case gui::ToolType::LabelTool:
            // create a label with the rubberband area
            createTextLabel(rb_scene_rect);
            break;
          case gui::ToolType::DragTool:
            // pan ends
            break;
          case gui::ToolType::MeasureTool:{
            // display measurement from start to finish
            QPointF delta = mapToScene(e->pos())-press_scene_pos;
            qreal dx = delta.x()*trans.m11()*prim::Item::scale_factor;
            qreal dy = delta.y()*trans.m22()*prim::Item::scale_factor;
            qDebug() << tr("Measure: x: %1 :: y: %2").arg(dx).arg(dy);
            break;
          }
          default:
            break;
        }
        break;
      default:
        break;
    }
  }

  // end rubber band if active
  if (rb)
    rubberBandClear();

  clicked=false;
}


void gui::DesignPanel::mouseDoubleClickEvent(QMouseEvent *e)
{
  QGraphicsView::mouseDoubleClickEvent(e);
}


void gui::DesignPanel::wheelEvent(QWheelEvent *e)
{
  // allow for different scroll types. Note both x and y scrolling
  QPoint pix_del = e->pixelDelta();
  QPoint deg_del = e->angleDelta();

  // accumulate scroll value
  if(!pix_del.isNull())
    wheel_deg += pix_del;
  else if(!deg_del.isNull())
    wheel_deg += deg_del;

  // if enough scroll achieved, act and reset wheel_deg
  if(qMax(qAbs(wheel_deg.x()),qAbs(wheel_deg.y())) >= 15) {
    Qt::KeyboardModifiers keymods = QApplication::keyboardModifiers();
    if(keymods & Qt::ControlModifier)
      wheelZoom(e, keymods & Qt::AltModifier);
    else
      wheelPan(keymods & Qt::ShiftModifier, keymods & Qt::AltModifier);
  }
}


// don't typically do anything on key press.
void gui::DesignPanel::keyPressEvent(QKeyEvent *e)
{
  //if an item is selected, move the item instead of scrolling the view.
  if (!selectedItems().isEmpty()) {
    QPointF offset(0,0);
    switch(e->key()){
      case Qt::Key_Up:
        offset = QPointF(0,-50);
        break;
      case Qt::Key_Down:
        offset = QPointF(0,50);
        break;
      case Qt::Key_Left:
        offset = QPointF(-50,0);
        break;
      case Qt::Key_Right:
        offset = QPointF(50,0);
        break;
      case Qt::Key_Escape:
        deselectAll();
        QGraphicsView::keyPressEvent(e);
        return;
      default:
        QGraphicsView::keyPressEvent(e);
        return;
    }
    //only here if we break out of the if-else, meaning:
    //items were selected and an arrow key was pressed
    Qt::KeyboardModifiers keymods = QApplication::keyboardModifiers();
    if(keymods & Qt::ShiftModifier)
      offset *= 10;
    undo_stack->beginMacro(tr("moving item"));
    for (prim::Item* item : selectedItems())
      undo_stack->push(new MoveItem(static_cast<prim::Item*>(item), offset, this));
    undo_stack->endMacro();
  } else {
    QGraphicsView::keyPressEvent(e);
    return;
  }

}

// all the shortcut keys should be done here. Must release keypress event for
// use by higher level widgets
void gui::DesignPanel::keyReleaseEvent(QKeyEvent *e)
{
  if(ghosting){
    // only allowed actions are for manipulating the ghost
    switch(e->key()){
      case Qt::Key_Escape:
        // stop ghosting
        if(moving)
          moveToGhost(true);
        clearGhost();
        break;
      case Qt::Key_H:
        // TODO flip ghost horizontally
        break;
      case Qt::Key_V:
        // TODO flip ghost vertically
        break;
      case Qt::Key_R:
        // TODO rotate based on whether the control modifiers is pressed
        break;
      default:
        break;
    }
  }
  else{
    switch(e->key()){
      case Qt::Key_Escape:
        // deactivate current tool
        if (tool_type != gui::ToolType::SelectTool) {
          //qDebug() << tr("Esc pressed, drop back to select tool");
          // emit signal to be picked up by application.cc
          emit sig_toolChangeRequest(gui::ToolType::SelectTool);
        }
        break;
      default:
        QGraphicsView::keyReleaseEvent(e);
        break;
    }
  }
}

void gui::DesignPanel::dragEnterEvent(QDragEnterEvent *e)
{
  // no actions associated with dragEnterEvent, leave it up to application
  // window to parse.
  e->ignore();
}

// INTERNAL METHODS

void gui::DesignPanel::constructStatics()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  background_col = gui_settings->get<QColor>("view/bg_col");
  background_col_publish = gui_settings->get<QColor>("view/bg_col_pb");
  zoom_visibility_threshold = gui_settings->get<qreal>("latdot/zoom_vis_threshold");
}


void gui::DesignPanel::duplicateSelection()
{
  if (selectedItems().isEmpty())
    return;

  // raise prompt
  int count = QInputDialog::getInt(this, tr("Selection duplication"),
                tr("Count:"), 2, 2, 1000, 1);

  if(count >= 2){
    copySelection();
    createGhost(true, count-1);
  }
}

void gui::DesignPanel::wheelZoom(QWheelEvent *e, bool boost)
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  // base zoom factor
  qreal ds = (wheel_deg.y()>0 ? 1 : -1) * gui_settings->get<qreal>("view/zoom_factor");
  // apply boost
  if(boost)
    ds *= gui_settings->get<qreal>("view/zoom_boost");

  applyZoom(ds, e);

  // reset both scrolls (avoid repeat from |x|>=120)
  wheel_deg.setX(0);
  wheel_deg.setY(0);
}

void gui::DesignPanel::stepZoom(const bool &zoom_in)
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  qreal ds = (zoom_in ? 1 : -1) * gui_settings->get<qreal>("view/zoom_factor");
  applyZoom(ds);
}

void gui::DesignPanel::applyZoom(qreal ds, QWheelEvent *e)
{
  // assert scale limitations
  boundZoom(ds);

  // save old zoom level
  qreal old_zoom = qAbs(transform().m11() + transform().m12());

  if(ds!=0){
    // zoom under mouse, should be indep of transformationAnchor
    QPointF old_pos, new_pos;
    if (e != nullptr) {
      old_pos = mapToScene(e->position().toPoint());
    } else {
      old_pos = mapToScene(mapFromParent(rect().center()));
    }

    // pre-zoom scene rect update
    //QRectF sbr = scene->itemsBoundingRect();
    //QRectF vp = mapToScene(viewport()->rect()).boundingRect();
    QPointF vp_tl = mapToScene(mapFromParent(rect().topLeft()));
    QPointF vp_br = mapToScene(mapFromParent(rect().bottomRight()));
    QRectF vp = QRectF(vp_tl, vp_br);
    if (ds < 0)
      vp.adjust(-vp.width(), -vp.height(), vp.width(), vp.height()); // account for zoom out viewport size
    //setSceneRect(min_scene_rect | sbr | vp);
    updateSceneRect(vp);

    // perform zoom
    scale(1+ds,1+ds);

    // move to anchor
    if (e != nullptr) {
      new_pos = mapToScene(e->position().toPoint());
    } else {
      new_pos = mapToScene(mapFromParent(rect().center()));
    }
    scrollDelta(new_pos - old_pos); // scroll with anchoring

    // post-zoom scene rect update
    //vp = mapToScene(viewport()->rect()).boundingRect();
    //setSceneRect(min_scene_rect | sbr | vp);
    updateSceneRect();
  }

  // update background if lattice visibility threshold has been crossed
  qreal new_zoom = qAbs(transform().m11() + transform().m12());
  if ((old_zoom < zoom_visibility_threshold && zoom_visibility_threshold <= new_zoom)
      || (new_zoom < zoom_visibility_threshold && zoom_visibility_threshold <= old_zoom))
    updateBackground();

  informZoomUpdate();
}


void gui::DesignPanel::wheelPan(bool shift_scroll, bool boost)
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  qreal dx=0, dy=0;

  // y scrolling
  if(wheel_deg.y()>0)
    dy -= gui_settings->get<qreal>("view/wheel_pan_step");
  else if(wheel_deg.y()<0)
    dy += gui_settings->get<qreal>("view/wheel_pan_step");
  wheel_deg.setY(0);

  // x scrolling
  if(wheel_deg.x()>0)
    dx -= gui_settings->get<qreal>("view/wheel_pan_step");
  else if(wheel_deg.x()<0)
    dx += gui_settings->get<qreal>("view/wheel_pan_step");
  wheel_deg.setX(0);

  // apply boost
  if(boost){
    qreal boost_fact = gui_settings->get<qreal>("view/wheel_pan_boost");
    dx *= boost_fact;
    dy *= boost_fact;
  }

  // flip x and y appropriately if shift is pressed
  if (shift_scroll) {
    qreal temp = dx;
    dx = dy;
    dy = temp;
  }

  // if scrolling past the current max / min, extend the scrolling area
  qreal xf = horizontalScrollBar()->value() + dx;
  qreal yf = verticalScrollBar()->value() + dy;
  if (xf > horizontalScrollBar()->maximum())
    horizontalScrollBar()->setMaximum(xf);
  else if (xf < horizontalScrollBar()->minimum())
    horizontalScrollBar()->setMinimum(xf);
  else if (yf > verticalScrollBar()->maximum())
    verticalScrollBar()->setMaximum(yf);
  else if (yf < verticalScrollBar()->minimum())
    verticalScrollBar()->setMinimum(yf);

  horizontalScrollBar()->setValue(horizontalScrollBar()->value()+ dx);
  verticalScrollBar()->setValue(verticalScrollBar()->value() + dy);
}


void gui::DesignPanel::boundZoom(qreal &ds)
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  qreal m = qAbs(transform().m11()) + qAbs(transform().m12());  // m = m11 = m22

  // need zoom_min <= m11*(1+ds) <= zoom_max
  if(ds<0)
    ds = qMax(ds, gui_settings->get<qreal>("view/zoom_min")/m-1);
  else
    ds = qMin(ds, gui_settings->get<qreal>("view/zoom_max")/m-1);
}

void gui::DesignPanel::scrollDelta(QPointF delta)
{
  qreal scroll_delta_v = delta.y()*transform().m11() + delta.x()*transform().m12();
  qreal scroll_delta_h = delta.y()*transform().m21() + delta.x()*transform().m22();
  verticalScrollBar()->setValue(verticalScrollBar()->value()-scroll_delta_v);
  horizontalScrollBar()->setValue(horizontalScrollBar()->value()-scroll_delta_h);
}


void gui::DesignPanel::contextMenuEvent(QContextMenuEvent *e)
{
  if (!clipboard.isEmpty()) { //not empty, enable pasting
    action_paste->setEnabled(true);
  } else {
    action_paste->setEnabled(false);
  }
  if (!scene->selectedItems().isEmpty()) { //not empty, enable deleting
    action_delete->setEnabled(true);
  } else {
    action_delete->setEnabled(false);
  }
  QMenu *menu = new QMenu(this);
  if (itemAt(e->pos())) {
    //all items at that position
    QList<QGraphicsItem*> gitems = items(e->pos());
    //keep track of inserted types, so as to not double insert.
    QList<int> inserted_types = QList<int>();
    for (auto gitem: gitems) {
      if (!inserted_types.contains(static_cast<prim::Item*>(gitem)->item_type)){
        inserted_types.append(static_cast<prim::Item*>(gitem)->item_type);
        QList<QAction*> actions = static_cast<prim::Item*>(gitem)->contextMenuActions();
        if (!actions.isEmpty()) {
          menu->addSection(static_cast<prim::Item*>(gitem)->getQStringItemType());
          for (auto action : actions) {
            action->setProperty("item_type", static_cast<prim::Item*>(gitem)->item_type);
            action->setProperty("pos", e->pos());
            menu->addAction(action);
            //Qt::UniqueConnection prevents connecting multiple times.
            connect(action, &QAction::triggered, this, &gui::DesignPanel::dummyAction, Qt::UniqueConnection);
          }
        }
      }
    }
  }
  menu->addSection("General");
  menu->addAction(action_undo);
  menu->addAction(action_redo);
  menu->addSeparator();
  menu->addAction(action_cut);
  menu->addAction(action_copy);
  menu->addAction(action_paste);
  menu->addSeparator();
  menu->addAction(action_delete);
  menu->exec(e->globalPos());
  delete menu;
}

void gui::DesignPanel::undoAction()
{
    undo_stack->undo();
}

void gui::DesignPanel::redoAction()
{
    undo_stack->redo();
}

void gui::DesignPanel::cutAction()
{
    copySelection();
    deleteSelection();
}

void gui::DesignPanel::copyAction()
{
    copySelection();
}

void gui::DesignPanel::pasteAction()
{
    if(!clipboard.isEmpty() && display_mode == DesignMode)
      createGhost(true);
}

void gui::DesignPanel::deleteAction()
{
  if(tool_type == gui::ToolType::SelectTool && display_mode == DesignMode) {
    qDebug() << "Delete action invoked";
    deleteSelection();
  } else {
    qDebug() << "Delete action conditions not met.";
  }
}

void gui::DesignPanel::dummyAction()
{
  QPoint pos = sender()->property("pos").toPoint();
  QList<QGraphicsItem*> gitems;
  if (itemAt(pos)) {
    //if there are selected items, use those that are same type as the one clicked.
    QList<prim::Item*> items_list = selectedItems();
    if (items_list.isEmpty()) {
      QList<QGraphicsItem*> gitems = items(pos);
      //since the list was empty, just add the ones we want to the list.
      for (auto gitem: gitems)
        items_list.append(static_cast<prim::Item*>(gitem));
    }
    for (auto item: items_list) {
      //make sure the item type is correct.
      if (item->item_type == sender()->property("item_type").toInt()) {
        item->performAction(static_cast<QAction*>(sender()));
      }
    }
  }
}

void gui::DesignPanel::initActions()
{
  action_undo = new QAction(QIcon::fromTheme("edit-undo", 
        QIcon(":/ico/fb/undo.svg")), tr("&Undo"), this);
  action_redo = new QAction(QIcon::fromTheme("edit-redo", 
        QIcon(":/ico/fb/redo.svg")), tr("&Redo"), this);
  action_cut = new QAction(QIcon::fromTheme("edit-cut",
        QIcon(":/ico/fb/cut.svg")), tr("Cut"), this);
  action_copy = new QAction(QIcon::fromTheme("edit-copy",
        QIcon(":/ico/fb/copy.svg")), tr("&Copy"), this);
  action_paste = new QAction(QIcon::fromTheme("edit-paste",
        QIcon(":/ico/fb/paste.svg")), tr("&Paste"), this);
  action_delete = new QAction(QIcon::fromTheme("edit-delete",
        QIcon(":/ico/fb/delete.svg")), tr("&Delete"), this);
  action_form_agg = new QAction(tr("Form A&ggregate"), this);
  action_split_agg = new QAction(tr("Split Aggregate"), this);
  action_dup = new QAction(tr("Duplicate"), this);

  action_undo->setShortcut(tr("CTRL+Z"));
  action_redo->setShortcuts({ tr("CTRL+Y"), tr("CTRL+SHIFT+Z") });
  action_cut->setShortcut(tr("CTRL+X"));
  action_copy->setShortcut(tr("CTRL+C"));
  action_paste->setShortcut(tr("CTRL+V"));
  action_delete->setShortcuts({ tr("DELETE"), tr("BACKSPACE") });
  action_form_agg->setShortcut(tr("CTRL+G"));
  action_split_agg->setShortcut(tr("CTRL+SHIFT+G"));
  action_dup->setShortcut(tr("CTRL+D"));

  connect(action_undo, &QAction::triggered, this, &gui::DesignPanel::undoAction);
  connect(action_redo, &QAction::triggered, this, &gui::DesignPanel::redoAction);
  connect(action_cut, &QAction::triggered, this, &gui::DesignPanel::cutAction);
  connect(action_copy, &QAction::triggered, this, &gui::DesignPanel::copyAction);
  connect(action_paste, &QAction::triggered, this, &gui::DesignPanel::pasteAction);
  connect(action_delete, &QAction::triggered, this, &gui::DesignPanel::deleteAction);
  connect(action_form_agg, &QAction::triggered,
          [this](){formAggregate();});
  connect(action_split_agg, &QAction::triggered, this, &gui::DesignPanel::splitAggregates);
  connect(action_dup, &QAction::triggered, this, &gui::DesignPanel::duplicateSelection);

  // add the actions to design panel so they're activated
  addActions({action_undo, action_redo, action_cut, action_copy, action_paste,
              action_delete, action_form_agg, action_split_agg, action_dup});
}


void gui::DesignPanel::rubberBandUpdate(QPoint pos){
  // stop rubber band if moving item
  if (moving || resizing) {
    rubberBandClear();
    return;
  }

  rb_cache = pos;

  if (rb == nullptr) {
    // make rubber band
    rb = new QRubberBand(QRubberBand::Rectangle, this);
    rb->setGeometry(QRect(mapFromScene(rb_start), QSize()));
    rb->show();
  } else {
    // update rubberband rectangle
    rb->setGeometry(QRect(mapFromScene(rb_start), pos).normalized());
    rb_scene_rect = QRect(rb_start, mapToScene(pos).toPoint()).normalized();
  }
}


void gui::DesignPanel::rubberBandSelect(){
  if (rb == nullptr)
    return;

  // select items that are enclosed by the rubberband
  QPainterPath painter_path;
  painter_path.addRect(rb_scene_rect);
  scene->setSelectionArea(painter_path, Qt::ReplaceSelection, Qt::ContainsItemShape);

  // append shift-selected items if they're still visible
  // TODO: it might be possible to simplify this by using Qt::AddToSelection in the above call when Shift is held
  for(QGraphicsItem* shift_selected_item : rb_shift_selected)
    if (shift_selected_item->isVisible())
      shift_selected_item->setSelected(true);
}


void gui::DesignPanel::rubberBandClear() {
  if (rb != nullptr) {
    rb->hide();
    rb = nullptr;
    rb_start = QPoint();
    rb_cache = QPoint();
    rb_scene_rect = QRect();
    rb_shift_selected.clear();
  }
}


void gui::DesignPanel::createGhost(bool paste, int count)
{
  clearGhost();

  // qDebug() << tr("Creating ghost...");
  prim::Ghost *ghost = prim::Ghost::instance();
  pasting=paste;
  ghosting=true;
  snap_cache = QPointF();

  if (paste) {
    ghost->prepare(clipboard, count);
    prim::LatticeCoord offset;
    if (snapGhost(mapToScene(mapFromGlobal(QCursor::pos())), offset))
      ghost->moveByCoord(offset, lattice);
  } else {
    QPointF scene_pos = mapToScene(mapFromGlobal(QCursor::pos()));
    //get QList of selected Item object
    ghost->prepare(selectedItems(), 1, scene_pos);
  }
}


void gui::DesignPanel::clearGhost()
{
  ///qDebug() << tr("Clearing ghost...");
  prim::Ghost::instance()->cleanGhost();
  ghosting=false;
  snap_coord=prim::LatticeCoord();
}


bool gui::DesignPanel::snapGhost(QPointF scene_pos, prim::LatticeCoord &offset)
{
  bool is_all_floating = true;

  // check if holding any non-floating objects
  for (prim::Item *item : pasting ? clipboard : selectedItems()) {
    if (item->item_type != prim::Item::Electrode &&
        item->item_type != prim::Item::TextLabel) {
      is_all_floating = false;
      break;
    }
  }

  if (is_all_floating) {
    prim::Ghost *ghost = prim::Ghost::instance();
    if (pasting) { //offset is in the first electrode item
      ghost->moveTo(mapToScene(mapFromGlobal(QCursor::pos()))
          - clipboard[0]->pos()
          - QPointF(static_cast<prim::Electrode*>(clipboard[0])->sceneRect().width()/2.0,
              static_cast<prim::Electrode*>(clipboard[0])->sceneRect().height()/2.0)
      );
    } else {
      ghost->moveTo(mapToScene(mapFromGlobal(QCursor::pos())));
    }
    return false;
  } else {
    // if holding any non-floating (for now just DBs), don't need to recheck
    // snap target unless the cursor has moved significantly
    if ((scene_pos-snap_cache).manhattanLength()<.3*snap_diameter)
      return false;
    snap_cache = scene_pos;

    prim::Ghost *ghost = prim::Ghost::instance();
    prim::GhostDot *anchor = ghost->snapAnchor();

    // if no anchor, allow free movement of the ghost
    if(anchor==0){
      offset = prim::LatticeCoord();
      return true;
    }
    // otherwise restrict possible ghost position to lattice sites
    prim::LatticeCoord old_anchor = anchor->latticeCoord();
    QPointF free_anchor = ghost->freeAnchor(scene_pos);

    // get the nearest lattice site to the free anchor
    prim::LatticeCoord nearest_site = lattice->nearestSite(free_anchor, true);

    // do nothing if target site is not valid or has not changed
    if (!lattice->isValid(nearest_site) || nearest_site == snap_coord)
      return false;

    // move ghost and update validity hash table
    offset = nearest_site - old_anchor;
    if (!ghost->valid_hash.contains(nearest_site))
      ghost->valid_hash[nearest_site] = ghost->checkValid(offset, lattice);
    snap_coord = nearest_site;
    ghost->setValid(ghost->valid_hash[nearest_site]);

    return true;
    /*
    // move ghost and update validity hash table.
    offset = target->scenePos()-old_anchor;
    if(!ghost->valid_hash.contains(target))
      ghost->valid_hash[target] = ghost->checkValid(offset);
    snap_target = target;
    ghost->setValid(ghost->valid_hash[target]);
    return true;*/
  }
}

void gui::DesignPanel::initMove()
{
  createGhost(false);

  // set lattice dots of selected DBs to be unoccupied
  for (prim::Item *item : selectedItems())
    setLatticeSiteOccupancy(item, false);

  moving = true;
}

void gui::DesignPanel::setLatticeSiteOccupancy(prim::Item *item, bool flag)
{
  switch(item->item_type){
    case prim::Item::DBDot:{
      prim::DBDot *dot = static_cast<prim::DBDot*>(item);
      if (flag)
        lattice->setOccupied(dot->latticeCoord(), dot);
      else
        lattice->setUnoccupied(dot->latticeCoord());
      break;
    }
    case prim::Item::Aggregate:
      for(prim::Item *it : static_cast<prim::Aggregate*>(item)->getChildren())
        setLatticeSiteOccupancy(it, flag);
      break;
    default:
      break;
  }
}


void gui::DesignPanel::copySelection()
{
  QList<prim::Item*> selection = selectedItems();
  if(selection.isEmpty())
    return;

  // empty the previous clipboard
  for(prim::Item *item : clipboard)
    delete item;
  clipboard.clear();

  for (prim::Item *item : selection)
    clipboard.append(item->deepCopy());

  qDebug() << tr("Added to clipboard: %1 items").arg(clipboard.count());
}

void gui::DesignPanel::createDBPreviews(QList<prim::LatticeCoord> coords)
{
  destroyDBPreviews();
  appendDBPreviews(coords);
}

void gui::DesignPanel::appendDBPreviews(QList<prim::LatticeCoord> coords)
{
  for (prim::LatticeCoord coord : coords) {
    if (lattice->isOccupied(coord))
      continue;
    prim::DBDotPreview *db_prev = new prim::DBDotPreview(coord);
    db_prev->setPos(lattice->latticeCoord2ScenePos(coord));
    db_previews.append(db_prev);
    scene->addItem(db_prev);
  }
}

void gui::DesignPanel::destroyDBPreviews()
{
  while (!db_previews.isEmpty()) {
    prim::DBDotPreview *db_prev = db_previews.takeFirst();
    scene->removeItem(db_prev);
    delete db_prev;
  }
}

prim::Item *gui::DesignPanel::filteredSnapTarget(QPointF scene_pos, QList<prim::Item::ItemType> &target_types, qreal search_box_width)
{
  snap_cache = scene_pos;

  // set search boundary
  QRectF search_bound;
  search_bound.setSize(QSizeF(search_box_width, search_box_width)); // use the same snap range as other snap functions
  search_bound.moveCenter(scene_pos);
  QList<QGraphicsItem*> near_items = scene->items(search_bound);

  // if no items nearby, return a null pointer
  if (near_items.count()==0) {
    return 0;
  }

  // find the item nearest to cursor position that falls under the targeted item type
  prim::Item *target=0;
  qreal mdist=-1, dist;

  for (QGraphicsItem *gitem : near_items) {
    prim::Item *pitem = static_cast<prim::Item*>(gitem);
    for (prim::Item::ItemType target_type : target_types) {
      if (pitem->item_type == target_type) {
        dist = (pitem->scenePos()-scene_pos).manhattanLength();
        if (mdist < 0 || dist < mdist) {
          mdist = dist;
          target = pitem;
        }
      }
    }
  }

  return target;
}

// UNDO/REDO STACK METHODS
// CreateDB class

gui::DesignPanel::CreateDB::CreateDB(prim::LatticeCoord l_coord, int layer_index,
    DesignPanel *dp, prim::DBDot *cp_src, bool invert, QUndoCommand *parent)
  : QUndoCommand(parent), invert(invert), lat_coord(l_coord), cp_src(cp_src),
      dp(dp), layer_index(layer_index)
{
  db_at_loc = dp->lattice->dbAt(l_coord);

  if (invert && !db_at_loc)
    qFatal("Trying to remove a non-existing DB");
  else if (!invert && db_at_loc)
    qFatal("Trying to make a new DB at a location that already has one");

  // dbdot index in layer
  prim::Layer *layer = dp->layman->getLayer(layer_index);
  index = invert ? layer->getItems().indexOf(db_at_loc) : layer->getItems().size();
}

void gui::DesignPanel::CreateDB::undo()
{
  invert ? create() : destroy();
}

void gui::DesignPanel::CreateDB::redo()
{
  invert ? destroy() : create();
}

void gui::DesignPanel::CreateDB::create()
{
  // add dangling bond to layer and scene, index in layer item stack will be
  // equal to layer->getItems().size()
  prim::DBDot *new_db = new prim::DBDot(lat_coord, layer_index);
  dp->lattice->setOccupied(lat_coord, new_db);
  dp->addItem(new_db, layer_index, index);
  dp->moveDBToLatticeCoord(new_db, lat_coord.n, lat_coord.m, lat_coord.l);
  db_at_loc=new_db;
}

void gui::DesignPanel::CreateDB::destroy()
{
  db_at_loc = dp->lattice->dbAt(lat_coord);
  if (db_at_loc) {
    dp->lattice->setUnoccupied(lat_coord);
    dp->removeItem(db_at_loc, dp->layman->getLayer(db_at_loc->layer_id));
    db_at_loc = 0;
  }
}


// CreatePotPlot class
gui::DesignPanel::CreatePotPlot::CreatePotPlot(gui::DesignPanel *dp, QString pot_plot_path, QRectF graph_container, QString pot_anim_path, prim::PotPlot *pp, bool invert, QUndoCommand *parent)
  : QUndoCommand(parent), dp(dp), pot_plot_path(pot_plot_path), graph_container(graph_container), pot_anim_path(pot_anim_path), pp(pp), invert(invert)
{  //if called to destroy, *elec points to selected electrode. if called to create, *elec = 0
}

void gui::DesignPanel::CreatePotPlot::undo()
{
  invert ? create() : destroy();
}

void gui::DesignPanel::CreatePotPlot::redo()
{
  invert ? destroy() : create();
}


void gui::DesignPanel::CreatePotPlot::create()
{
  pp = new prim::PotPlot(pot_plot_path, graph_container, pot_anim_path);
  dp->addItemToScene(static_cast<prim::Item*>(pp));
  dp->sim_results_items.append(static_cast<prim::Item*>(pp));
  connect(pp->getPotentialAnimation(), &QMovie::frameChanged, dp, &gui::DesignPanel::updateSimMovie);

}

void gui::DesignPanel::CreatePotPlot::destroy()
{
  if(pp != 0){
    dp->removeItemFromScene(static_cast<prim::Item*>(pp));  // deletes PotPlot
    dp->sim_results_items.removeOne(static_cast<prim::Item*>(pp));
    pp = 0;
  }
}



// Create Text Label class
gui::DesignPanel::CreateTextLabel::CreateTextLabel(int layer_index,
    DesignPanel *dp, const QRectF &scene_rect, const QString &text,
    prim::TextLabel *text_lab, bool invert, QUndoCommand *parent)
  : QUndoCommand(parent), dp(dp), invert(invert), layer_index(layer_index),
    scene_rect(scene_rect), text(text)
{
  prim::Layer *layer = dp->layman->getLayer(layer_index);
  item_index = invert ? layer->getItems().indexOf(text_lab) : layer->getItems().size();
}

void gui::DesignPanel::CreateTextLabel::undo()
{
  invert ? create() : destroy();
}

void gui::DesignPanel::CreateTextLabel::redo()
{
  invert ? destroy() : create();
}

void gui::DesignPanel::CreateTextLabel::create()
{
  dp->addItem(new prim::TextLabel(scene_rect, layer_index, text), layer_index,
              item_index);
}

void gui::DesignPanel::CreateTextLabel::destroy()
{
  prim::TextLabel *text_lab = reinterpret_cast<prim::TextLabel*>
    (dp->layman->getLayer(layer_index)->getItem(item_index));
  if (text_lab != 0) {
    dp->removeItem(text_lab, dp->layman->getLayer(text_lab->layer_id));
    text_lab = 0;
  } else {
    qCritical() << tr("Trying to delete non-existent text label");
  }
}


// EditTextLabel class
gui::DesignPanel::EditTextLabel::EditTextLabel(int layer_index, DesignPanel *dp,
                                               const QString &new_text,
                                               prim::TextLabel *text_lab,
                                               bool invert, QUndoCommand *parent)
  : QUndoCommand(parent), dp(dp), invert(invert), layer_index(layer_index),
    text_new(new_text)
{
  prim::Layer *layer= dp->layman->getLayer(layer_index);
  item_index = layer->getItems().indexOf(text_lab);
  text_orig = text_lab->text();
}

void gui::DesignPanel::EditTextLabel::undo()
{
  prim::TextLabel *text_lab = reinterpret_cast<prim::TextLabel*>
    (dp->layman->getLayer(layer_index)->getItem(item_index));
  if (!invert)
    text_lab->setText(text_orig);
  else
    text_lab->setText(text_new);
}

void gui::DesignPanel::EditTextLabel::redo()
{
  prim::TextLabel *text_lab = reinterpret_cast<prim::TextLabel*>
    (dp->layman->getLayer(layer_index)->getItem(item_index));
  if (!invert)
    text_lab->setText(text_new);
  else
    text_lab->setText(text_orig);
}


// CreateItem class
gui::DesignPanel::CreateItem::CreateItem(int layer_index, DesignPanel *dp,
                                         prim::Item *item, bool invert,
                                         QUndoCommand *parent)
  : QUndoCommand(parent), dp(dp), invert(invert), layer_index(layer_index),
    item(item)
{
  prim::Layer *layer = dp->layman->getLayer(layer_index);
  item_index = invert ? layer->getItems().indexOf(item) : layer->getItems().size();
  in_scene = invert ? true : false;
}

gui::DesignPanel::CreateItem::~CreateItem()
{
  if (!in_scene) {
    qDebug() << tr("Deleting item from QUndoStack");
    delete item;
  }
}

void gui::DesignPanel::CreateItem::undo()
{
  invert ? create() : destroy();
}

void gui::DesignPanel::CreateItem::redo()
{
  invert ? destroy() : create();
}

void gui::DesignPanel::CreateItem::create()
{
  if (!item)
    qCritical() << tr("Item pointer is 0, cannot create new item");
  dp->addItem(item, layer_index, item_index);
  in_scene = true;
}

void gui::DesignPanel::CreateItem::destroy()
{
  // NOTE issues will arise if layers have been added/removed
  item = dp->layman->getLayer(layer_index)->getItem(item_index);
  prim::Item *item_copy = item->deepCopy();
  dp->removeItem(item, dp->layman->getLayer(item->layer_id));
  item = item_copy;
  in_scene = false;
}


// ResizeItem class
gui::DesignPanel::ResizeItem::ResizeItem(int layer_index, DesignPanel *dp,
                                         int item_index, const QRectF &orig_rect,
                                         const QRectF &new_rect, bool manual,
                                         bool invert, QUndoCommand *parent)
  : QUndoCommand(parent), dp(dp), invert(invert), manual(manual),
    layer_index(layer_index), item_index(item_index), orig_rect(orig_rect),
    new_rect(new_rect)
{
  top_left_delta = new_rect.topLeft() - orig_rect.topLeft();
  bottom_right_delta = new_rect.bottomRight() - orig_rect.bottomRight();
}

void gui::DesignPanel::ResizeItem::undo()
{
  prim::ResizableRect *item = reinterpret_cast<prim::ResizableRect*>
    (dp->layman->getLayer(layer_index)->getItem(item_index));

  item->resize(-top_left_delta.x(), -top_left_delta.y(),
               -bottom_right_delta.x(), -bottom_right_delta.y(), true);
}

void gui::DesignPanel::ResizeItem::redo()
{
  prim::ResizableRect *item = reinterpret_cast<prim::ResizableRect*>
    (dp->layman->getLayer(layer_index)->getItem(item_index));

  // if the user resized manually, then the area is already the right size
  if (manual) {
    manual = false;
    return;
  }

  item->resize(top_left_delta.x(), top_left_delta.y(),
               bottom_right_delta.x(), bottom_right_delta.y(), true);
}


// RotateItem class
gui::DesignPanel::RotateItem::RotateItem(int layer_index, DesignPanel *dp,
                                         int item_index, double init_ang, double fin_ang,
                                         bool invert, QUndoCommand *parent)
  : QUndoCommand(parent), dp(dp), invert(invert),
    layer_index(layer_index), item_index(item_index), init_ang(init_ang),
    fin_ang(fin_ang)
{

}

void gui::DesignPanel::RotateItem::undo()
{
  prim::Item *item = dp->layman->getLayer(layer_index)->getItem(item_index);
  item->setRotation(init_ang);
}

void gui::DesignPanel::RotateItem::redo()
{
  prim::Item *item = dp->layman->getLayer(layer_index)->getItem(item_index);
  item->setRotation(fin_ang);
}

// ChangeColor class
gui::DesignPanel::ChangeColor::ChangeColor(int layer_index, DesignPanel *dp,
                                         int item_index, QColor init_col, QColor fin_col,
                                         bool invert, QUndoCommand *parent)
  : QUndoCommand(parent), dp(dp), invert(invert),
    layer_index(layer_index), item_index(item_index), init_col(init_col),
    fin_col(fin_col)
{

}

void gui::DesignPanel::ChangeColor::undo()
{
  prim::Item *item = dp->layman->getLayer(layer_index)->getItem(item_index);
  item->setColor(init_col);
  item->update();
}

void gui::DesignPanel::ChangeColor::redo()
{
  prim::Item *item = dp->layman->getLayer(layer_index)->getItem(item_index);
  item->setColor(fin_col);
  item->update();
}


// FromAggregate class
gui::DesignPanel::FormAggregate::FormAggregate(QList<prim::Item *> &items,
                                            DesignPanel *dp, QUndoCommand *parent)
  : QUndoCommand(parent), invert(false), dp(dp), agg_index(-1)
{
  if(items.count()==0){
    qWarning() << tr("Aggregate contains no items");
    return;
  }

  // get layer_index, and check that it is the same for all items
  layer_index = items.at(0)->layer_id;
  prim::Layer *layer = dp->layman->getLayer(layer_index);
  for(prim::Item *item : items)
    if(item->layer_id != layer_index){
      qWarning() << tr("Aggregates can only be formed from items in the same layer");
      return;
    }

  // format the input items to a pointer invariant form, TODO could be done more efficiently
  QStack<prim::Item*> layer_items = layer->getItems();
  for(prim::Item *item : items)
    item_inds.append(layer_items.indexOf(item));
  std::sort(item_inds.begin(), item_inds.end());
}

gui::DesignPanel::FormAggregate::FormAggregate(prim::Aggregate *agg, int offset,
                                          DesignPanel *dp, QUndoCommand *parent)
  : QUndoCommand(parent), invert(true), dp(dp)
{
  // get layer index, assumes aggregate was formed using FormAggregate
  //prim::Layer *layer = agg->layer;
  //layer_index = dp->getLayerIndex(layer);
  layer_index = agg->layer_id;
  prim::Layer *layer = dp->layman->getLayer(layer_index);

  // aggregate index
  QStack<prim::Item*> layer_items = layer->getItems();
  agg_index = layer_items.indexOf(agg);

  // doesn't really matter where we add the items from the aggregate to the layer
  // item stack... add to the end
  for(int i=layer_items.count()-1, j=0; j < agg->getChildren().count(); j++)
    item_inds.append(i+j+offset);
}

// split the aggregate
void gui::DesignPanel::FormAggregate::undo()
{
  invert ? form() : split();
}

// form the aggregate
void gui::DesignPanel::FormAggregate::redo()
{
  invert ? split() : form();
}


void gui::DesignPanel::FormAggregate::form()
{
  prim::Layer *layer = dp->layman->getLayer(layer_index);

  // all items should be in the same layer as the aggregate was and have no parents
  prim::Item *item=0;
  QStack<prim::Item*> layer_items = layer->getItems();
  for(const int &ind : item_inds) {
    if(ind >= layer_items.size()) {
      qFatal("Undo/Redo mismatch... something went wrong");
    }
    item = layer_items.at(ind);
    if(item->layer_id != layer_index || item->parentItem() != 0) {
      qFatal("Undo/Redo mismatch... something went wrong");
    }
  }

  // remove the items from the Layer stack in reverse order
  QStack<prim::Item*> items;
  for(int i=item_inds.count()-1; i>=0; i--) {
    items.push(layer->takeItem(item_inds.at(i)));
  }

  // remove all Items from the scene
  for(prim::Item *item : items) {
    if(item != 0) {
      item->scene()->removeItem(item);
    }
  }

  // add new aggregate to system
  dp->addItem(new prim::Aggregate(layer_index, items), layer_index, agg_index);
}


void gui::DesignPanel::FormAggregate::split()
{
  prim::Layer *layer = dp->layman->getLayer(layer_index);
  prim::Item *item = layer->takeItem(agg_index);

  if(item->item_type != prim::Item::Aggregate)
    qFatal("Undo/Redo mismatch... something went wrong");

  prim::Aggregate *agg = static_cast<prim::Aggregate*>(item);

  // remove aggregate and all children from the scene
  agg->scene()->removeItem(agg);

  // TODO: hate this, need to figure out why the delete agg causes problems
  //      after moving and move the "temp" stuff into the destructor.

  // re-insert the component items into the Layer from the item stack
  QStack<prim::Item*> items = agg->getChildren();
  for(const int& ind : item_inds){
    prim::Item *temp = items.pop();
    dp->addItem(temp, layer_index, ind);
    temp->setParentItem(agg->parentItem());
    temp->setFlag(QGraphicsItem::ItemIsSelectable, true);
    temp->setSelected(true);
  }

  // destroy the aggregate
  // delete agg;
}


// MoveItem class
gui::DesignPanel::MoveItem::MoveItem(prim::Item *item, const QPointF &offset,
                                      DesignPanel *dp, QUndoCommand *parent)
  : QUndoCommand(parent), dp(dp), offset(offset)
{
  layer_index = item->layer_id;
  // qDebug() << dp->layman->getLayer(layer_index)->getItemIndex(item);
  item_index = dp->layman->getLayer(layer_index)->getItems().indexOf(item);
}


void gui::DesignPanel::MoveItem::undo()
{
  move(true);
}


void gui::DesignPanel::MoveItem::redo()
{
  move(false);
}

void gui::DesignPanel::MoveItem::move(bool invert)
{
  prim::Layer *layer = dp->layman->getLayer(layer_index);
  prim::Item *item = layer->getItem(item_index);

  QPointF delta = invert ? -offset : offset;

  // should update original boundingRect after move to handle residual artifacts
  QRectF old_rect = item->boundingRect();
  // move the item
  moveItem(item, delta);

  // redraw old and new bounding rects to handle artifacts
  item->scene()->update(old_rect);
  item->scene()->update(item->boundingRect());
}


void gui::DesignPanel::MoveItem::moveItem(prim::Item *item, const QPointF &delta)
{
  switch(item->item_type){
    case prim::Item::DBDot:
      moveDBDot(static_cast<prim::DBDot*>(item), delta);
      break;
    case prim::Item::Aggregate:
      moveAggregate(static_cast<prim::Aggregate*>(item), delta);
      break;
    default:
      item->moveItemBy(delta.x(), delta.y());
      break;
  }
}


void gui::DesignPanel::MoveItem::moveDBDot(prim::DBDot *dot, const QPointF &delta)
{
  // get the target lattice site coordinate
  QPointF new_pos = dot->scenePos() + delta;
  QPointF nearest_site_pos;
  prim::LatticeCoord coord = dp->lattice->nearestSite(new_pos, nearest_site_pos, true);
  if (dp->lattice->collidesWithLatticeSite(new_pos, coord)) {
    // set the previous site as unoccupied if that site still points to this dot
    if (dp->lattice->dbAt(dot->latticeCoord()) == dot)
      dp->lattice->setUnoccupied(dot->latticeCoord());
    dot->setLatticeCoord(coord);
    dp->moveDBToLatticeCoord(dot, coord.n, coord.m, coord.l);
    dp->lattice->setOccupied(coord, dot);
  } else {
    qCritical() << tr("Failed to move DBDot");
  }
}


void gui::DesignPanel::MoveItem::moveAggregate(prim::Aggregate *agg, const QPointF &delta)
{
  // for Aggregates, move only the contained Items
  for(prim::Item *item : agg->getChildren())
    moveItem(item, delta);

  // workaround to make sure the boundingRect scene position is updated
  agg->setPos(agg->scenePos()+QPointF(1,0));
  agg->setPos(agg->scenePos()+QPointF(-1,0));
}

bool gui::DesignPanel::commandCreateItem(QString type, QString layer_id, QStringList item_args)
{
  prim::Item::ItemType item_type = prim::Item::getEnumItemType(type);
  switch(item_type){
    case prim::Item::Electrode:
      if (item_args.size()==4) {
        setTool(gui::ToolType::ElectrodeTool);
        emit sig_toolChangeRequest(gui::ToolType::ElectrodeTool);
        int layer_index = (layer_id == "auto") ? layman->indexOf(layman->activeLayer()) : layer_id.toInt();
        prim::Electrode* elec = new prim::Electrode(layer_index, item_args);
        undo_stack->push(new CreateItem(layer_index, this, elec));
        emit sig_toolChangeRequest(gui::ToolType::SelectTool);
        return true;
      }
      break;
    case prim::Item::DBDot:
      if (item_args.size()==3) {
        int n = item_args.takeFirst().toInt();
        int m = item_args.takeFirst().toInt();
        int l = item_args.takeFirst().toInt();
        if ((l == 0) || (l == 1)) {  // Check for valid
          setTool(gui::ToolType::DBGenTool);
          //int layer_index = (layer_id == "auto") ? layman->indexOf(layman->activeLayer()) : layer_id.toInt();
          emit sig_toolChangeRequest(gui::ToolType::DBGenTool);
          createDBs(prim::LatticeCoord(n, m, l));
          emit sig_toolChangeRequest(gui::ToolType::SelectTool);
          return true;
        }
      }
      break;
    case prim::Item::Aggregate:
    {
      // NOTE current implementation is a quick hack and is non-ideal, future
      // implementation should also allow (n,m,l) coordinates.
      if (item_args.size() % 2 != 0) {
        qWarning() << tr("Expect an even number of arguments with each pair representing one physical coordinate");
        return false;
      }
      QList<prim::Item*> dbs_for_agg;
      while (item_args.size() != 0) {
        float x = item_args.takeFirst().toFloat();
        float y = item_args.takeFirst().toFloat();
        prim::LatticeCoord l_coord = lattice->nearestSite(QPointF(x,y), false);
        prim::DBDot *db = lattice->dbAt(l_coord);
        if (db == nullptr) {
          qWarning() << tr("Location (%1, %2) does not contain a DB, ceasing aggregate creation.").arg(x).arg(y);
          return false;
        }
        qDebug() << tr("DB found at (%1, %2) and added to pending aggregate list.").arg(x).arg(y);
        dbs_for_agg.append(db);
      }
      if (dbs_for_agg.length() < 2) {
        qWarning() << tr("Less than 2 DBs on Aggregate list when Aggregate must contain more than 1 DB. Ceasing creation.");
        return false;
      }
      formAggregate(dbs_for_agg);
      return true;
      break;
    }
    default:
      return false;
  }
  return false;
}

bool gui::DesignPanel::commandRemoveItem(QString type, QStringList brackets, QStringList numericals)
{
  prim::Item::ItemType item_type = prim::Item::getEnumItemType(type);
  if (brackets.size()==2) {
    // Remove items by location
    float x = brackets.first().toFloat();
    float y = brackets.last().toFloat();
    QPointF pos = QPointF(x,y)*prim::Item::scale_factor;
    if (itemAt(mapFromScene(pos))) {
      QList<QGraphicsItem*> gitems = items(mapFromScene(pos));
      for (QGraphicsItem* item: gitems) {
        if (static_cast<prim::Item*>(item)->item_type == item_type)
          commandRemoveHandler(static_cast<prim::Item*>(item));
      }
      return true;
    }
  } else if (numericals.size()==2) {
    // Remove items by indices
    int lay_id = numericals.first().toInt();
    int item_id = numericals.last().toInt();
    prim::Layer *layer = layman->getLayer(lay_id);
    if (layer) {
      prim::Item *item = layer->getItem(item_id);
      if (item) {
        commandRemoveHandler(item);
        return true;
      }
    }
  }
  return false;
}

void gui::DesignPanel::commandRemoveHandler(prim::Item *item)
{
  switch (item->item_type) {
    case prim::Item::DBDot:
      undo_stack->beginMacro(tr("Deleting DBs"));
      undo_stack->push(new CreateDB(static_cast<prim::DBDot*>(item)->latticeCoord(),
                                  item->layer_id, this, 0, true));
      undo_stack->endMacro();
      break;
    default:
      undo_stack->push(new CreateItem(item->layer_id, this, item, true));
  }
}


bool gui::DesignPanel::commandMoveItem(QString type, QStringList brackets, QStringList numericals)
{
  prim::Item::ItemType item_type = prim::Item::getEnumItemType(type);
  if (brackets.size()>=4) {
    //4 items in brackets means a location and an offset.
    //5 items means a location and a lattice coord offset
    QPointF pos = QPointF(brackets.takeFirst().toFloat(), brackets.takeFirst().toFloat());
    pos *= prim::Item::scale_factor;
    QPointF offset = findMoveOffset(brackets);
    if (offset.isNull())
      return false;
    else if (itemAt(mapFromScene(pos))) {
      QList<QGraphicsItem*> gitems = items(mapFromScene(pos));
      undo_stack->beginMacro(tr("moving item"));
      for (QGraphicsItem* item: gitems){
        if (static_cast<prim::Item*>(item)->item_type == item_type)
          undo_stack->push(new MoveItem(static_cast<prim::Item*>(item), offset, this));
      }
      undo_stack->endMacro();
      return true;
    }
  }
  else if ((numericals.size()==2) && brackets.size()>=2)  {
    int lay_id = numericals.takeFirst().toInt();
    int item_id = numericals.takeFirst().toInt();
    prim::Layer *layer = layman->getLayer(lay_id);
    if (layer) {
      prim::Item *item = layer->getItem(item_id);
      if (item) {
        QPointF offset = findMoveOffset(brackets);
        if (offset.isNull())
          return false;
        else
          undo_stack->push(new MoveItem(static_cast<prim::Item*>(item), offset, this));
        return true;
      }
    }
  }
  return false;
}

QPointF gui::DesignPanel::findMoveOffset(QStringList args)
{
  QPointF offset;
  if (args.size() == 2) {
    offset = QPointF(args.first().toFloat(), args.last().toFloat());
  } else if (args.size() == 3) {
    int n = args[0].toInt();
    int m = args[1].toInt();
    int l = args[2].toInt();
    if ((qAbs(l) > 1)) {  //can be -1, 0, or 1. If anything else, default to 0.
      l = 0;
    }
    QPointF offset_n_m = lattice->latticeCoord2PhysLoc(prim::LatticeCoord(n,m,0));
    // account for negative l movement
    QPointF offset_l = lattice->latticeCoord2PhysLoc(prim::LatticeCoord(0,0,qAbs(l)));
    offset_l = (l >= 0) ? offset_l : -offset_l;
    offset = offset_n_m + offset_l;
  } else {
    return QPointF(0.0,0.0);
  }
  offset *= prim::Item::scale_factor;
  return offset;
}

// Undo/Redo Methods

void gui::DesignPanel::createDBs(prim::LatticeCoord lat_coord)
{
  if (displayMode() != gui::DisplayMode::DesignMode) {
    qDebug() << "DB creation not allowed outside of design mode.";
      return;
  }
  prim::Layer *layer = layman->activeLayer();
  int layer_index = layman->indexOf(layer);
  if (layer->contentType() != prim::Layer::DB) {
    qWarning() << "Current layer type is incompatible with DB creation.";
    return;
  } else if (layer->role() == prim::Layer::Result) {
    qWarning() << "Current layer role is Result, cannot create DBs.";
    return;
  }
  // save list of lattice coords where DBs should be created
  QList<prim::LatticeCoord> lat_list = QList<prim::LatticeCoord>();
  if (!lat_coord.isValid()) {
    // multiple creation, from using tool
    for (prim::DBDotPreview *db_prev : db_previews) {
      if (!lattice->isOccupied(db_prev->latticeCoord())) {
        lat_list.append(db_prev->latticeCoord());
      }
    }
    destroyDBPreviews();
  } else {
    //single creation, using command
    if (!lattice->isOccupied(lat_coord)) {
      lat_list.append(lat_coord);
    }
  }
  if (lat_list.isEmpty()) {
    return;
  }
  undo_stack->beginMacro(tr("create dangling bonds"));
  for (prim::LatticeCoord lc: lat_list) {
    undo_stack->push(new CreateDB(lc, layer_index, this));
  }
  undo_stack->endMacro();
}

void gui::DesignPanel::createElectrode(QRect scene_rect)
{
  if (displayMode() != gui::DisplayMode::DesignMode) {
    qDebug() << "Electrode creation not allowed outside of design mode.";
      return;
  }
  settings::GUISettings *S = settings::GUISettings::instance();
  qreal electrode_min_dim = S->get<qreal>("electrode/min_dim");
  if (scene_rect.isNull()) {
    return;
  } else if (scene_rect.width() / prim::Item::scale_factor < electrode_min_dim
      || scene_rect.width() / prim::Item::scale_factor < electrode_min_dim) {
    qWarning() << tr("Cannot create electrodes with one of the dimensions less "
        " than %1 angstrom").arg(electrode_min_dim);
    return;
  }

  prim::Layer *layer = layman->activeLayer();
  int layer_index = layman->indexOf(layer);
  if (layer->contentType() != prim::Layer::Electrode) {
    qWarning() << "Current layer type is incompatible with electrode creation.";
    return;
  } else if (layer->role() == prim::Layer::Result) {
    qWarning() << "Current layer role is Result, cannot create electrodes.";
    return;
  }
  //only ever create one electrode at a time
  undo_stack->beginMacro(tr("create electrode with given corners"));
  undo_stack->push(new CreateItem(layer_index, this,
                                  new prim::Electrode(layer_index, scene_rect)));
  undo_stack->endMacro();
}


void gui::DesignPanel::createPotPlot(QString pot_plot_path, QRectF graph_container, QString pot_plot_anim)
{
  // int layer_index = layman->indexOf(layman->getMRULayer(prim::Layer::Plot));
  undo_stack->beginMacro(tr("create potential plot with given corners"));
  undo_stack->push(new CreatePotPlot(this, pot_plot_path, graph_container, pot_plot_anim));
  undo_stack->endMacro();
}


void gui::DesignPanel::createTextLabel(const QRect &scene_rect)
{
  if (scene_rect.isNull())
    return;
  bool ok;
  QString text = prim::TextLabel::textPrompt("", &ok);
  if (!ok)
    return;
  int layer_index = layman->indexOf(layman->activeLayer());
  undo_stack->beginMacro(tr("Create Label"));
  undo_stack->push(new CreateItem(layer_index, this,
                                  new prim::TextLabel(scene_rect, layer_index, text)));
  undo_stack->endMacro();
}

void gui::DesignPanel::editTextLabel(prim::TextLabel *text_lab,
                                     const QString &new_text)
{
  int layer_index = text_lab->layer_id;
  undo_stack->beginMacro(tr("Edit Text Label"));
  undo_stack->push(new EditTextLabel(layer_index, this, new_text, text_lab));
  undo_stack->endMacro();
}


void gui::DesignPanel::updateSimMovie()
{
  for (prim::Item* item: sim_results_items) {
    if (item->item_type == prim::Item::PotPlot) {
      static_cast<prim::PotPlot*>(item)->updateSimMovie();
    }
  }
}

void gui::DesignPanel::resizeItemRect(prim::Item *item,
    const QRectF &orig_rect, const QRectF &new_rect)
{
  resizing = false;
  if (item->isResizable()) {
    int item_index = layman->getLayer(item->layer_id)->getItemIndex(item);
    undo_stack->beginMacro(tr("Resize Item"));
    undo_stack->push(new ResizeItem(item->layer_id, this, item_index,
          orig_rect, new_rect, true));
    undo_stack->endMacro();
  }
}


void gui::DesignPanel::deleteSelection()
{
  // do something only if there is a selection
  QList<prim::Item*> selection = selectedItems();
  if(selection.isEmpty())
    return;

  qDebug() << tr("Deleting %1 items").arg(selection.count());

  undo_stack->beginMacro(tr("delete %1 items").arg(selection.count()));
  for(prim::Item *item : selection){
    switch(item->item_type){
      case prim::Item::DBDot:
        undo_stack->push(new CreateDB(static_cast<prim::DBDot*>(item)->latticeCoord(),
                                      item->layer_id, this, 0, true));
        break;
      case prim::Item::Aggregate:
        destroyAggregate(static_cast<prim::Aggregate*>(item));
        break;
      case prim::Item::PotPlot:
        {
        prim::PotPlot *pp = static_cast<prim::PotPlot*>(item);
        undo_stack->push(new CreatePotPlot(this,
            pp->getPotPlotPath(), pp->getGraphContainer(), pp->getAnimPath(),
            static_cast<prim::PotPlot*>(item), true));
        break;
        }
      default:
        // generic item removal
        undo_stack->push(new CreateItem(item->layer_id, this, item, true));
        break;
    }
  }
  undo_stack->endMacro();
}


void gui::DesignPanel::formAggregate(QList<prim::Item*> items)
{
  QList<prim::Item*> selection = items;

  if (items.isEmpty())
    selection = selectedItems();

  if(selection.isEmpty())
    return;

  // check if selected items are on the surface
  for(prim::Item *item : selection){
    // if(items.last()->layer != layers.at(1)){ DOUBLE CHECK WITH JAKE
    if(layman->getLayer(item->layer_id)->contentType() != prim::Layer::DB){
      qCritical() << tr("Selected item not in a DB layer.");
      return;
    }
  }

  if(selection.count()<2){
    qWarning() << tr("Must select multiple items to form an aggregate");
    return;
  }

  // reversably create the aggregate
  undo_stack->push(new FormAggregate(selection, this));
}

void gui::DesignPanel::splitAggregates()
{
  // do something only if there is a selection
  QList<prim::Item*> selection = selectedItems();
  if (selection.isEmpty())
    return;

  // get selected aggregates
  QList<prim::Aggregate*> aggs;
  for (prim::Item *item : selection) {
    if (item->item_type == prim::Item::Aggregate) {
      aggs.append(static_cast<prim::Aggregate*>(item));
    }
  }

  if (aggs.count()==0)
    return;

  undo_stack->beginMacro(tr("Split %1 aggregates").arg(aggs.count()));
  qDebug() << tr("Split %1 aggregates").arg(aggs.count());
  int offset=0;
  for(prim::Aggregate *agg : aggs){
    // the following commented lines are supposed to fix split aggregate issues,
    // might have broken other things though
    //int temp = agg->getChildren().count()-1;
    undo_stack->push(new FormAggregate(agg, offset, this));
    //offset += temp;
  }
  undo_stack->endMacro();
}

void gui::DesignPanel::destroyAggregate(prim::Aggregate *agg)
{
  undo_stack->beginMacro(tr("Split the aggregate"));

  // break the Aggregate
  QStack<prim::Item*> items = agg->getChildren();
  undo_stack->push(new FormAggregate(agg, 0, this));

  // recursively destroy all children using undo/redo enabled methods
  for(prim::Item* item : items){
    switch(item->item_type){
      case prim::Item::DBDot:
        undo_stack->push(new CreateDB(static_cast<prim::DBDot*>(item)->latticeCoord(),
                                      item->layer_id, this, 0, true));
        break;
      case prim::Item::Aggregate:
        destroyAggregate(static_cast<prim::Aggregate*>(item));
        break;
      default:
        break;
    }
  }

  undo_stack->endMacro();
}

bool gui::DesignPanel::pasteAtGhost()
{
  prim::Ghost *ghost = prim::Ghost::instance();
  bool is_all_floating = true;
  for(QGraphicsItem *gitem : clipboard){
    if (static_cast<prim::Item*>(gitem)->item_type != prim::Item::Electrode &&
        reinterpret_cast<prim::TextLabel*>(gitem)->item_type != prim::Item::TextLabel){
      is_all_floating = false;
      break;
    }
  }
  // do nothing if clipboard empty
  if (clipboard.isEmpty()) {
    return false;
  } else {
    if (!is_all_floating && !ghost->valid_hash[snap_coord]) {
      return false;
    }
  }


  undo_stack->beginMacro(tr("Paste %1 items").arg(clipboard.count()));

  for(int i=0; i<ghost->getCount(); i++){
    for(prim::Item *item : ghost->getTopItems())
      pasteItem(ghost, i, item);
  }
  undo_stack->endMacro();

  pasting=false;
  return true;
}

void gui::DesignPanel::pasteItem(prim::Ghost *ghost, int n, prim::Item *item)
{
  switch(item->item_type){
    case prim::Item::DBDot:
      pasteDBDot(ghost, n, static_cast<prim::DBDot*>(item));
      break;
    case prim::Item::Aggregate:
      pasteAggregate(ghost, n, static_cast<prim::Aggregate*>(item));
      break;
    case prim::Item::Electrode:
      pasteElectrode(ghost, n, static_cast<prim::Electrode*>(item));
      break;
    default:
      qCritical() << tr("No functionality for pasting given item... update pasteItem");
      break;
  }
}

void gui::DesignPanel::pasteDBDot(prim::Ghost *ghost, int n, prim::DBDot *db)
{
  // get the target lattice dot
  auto coord = ghost->getLatticeCoord(db, n);
  if(lattice->isValid(coord))
    undo_stack->push(new CreateDB(coord, layman->indexOf(layman->getMRULayer(prim::Layer::DB)), this, db));
}

void gui::DesignPanel::pasteAggregate(prim::Ghost *ghost, int n, prim::Aggregate *agg)
{
  undo_stack->beginMacro("Paste an aggregate");

  // paste all the children items
  QList<prim::Item*> items;
  for(prim::Item *item : agg->getChildren()){
    pasteItem(ghost, n, item);
    // new item will be at the top of the Layer Item stack
    items.append(layman->activeLayer()->getItems().top());
  }

  // form Aggregate from Items
  undo_stack->push(new FormAggregate(items, this));

  undo_stack->endMacro();

}

void gui::DesignPanel::pasteElectrode(prim::Ghost *ghost, int, prim::Electrode *elec)
{
  QRectF rect = elec->sceneRect();
  rect.moveTopLeft(ghost->pos()+rect.topLeft());
  undo_stack->beginMacro(tr("create electrode with given corners"));
  undo_stack->push(new CreateItem(elec->layer_id, this,
                                  new prim::Electrode(elec->layer_id, rect)));
  undo_stack->endMacro();
}


// NOTE: currently item move relies on there being a snap target (i.e. at least
//       one dangling bond is being moved). Should modify in future to be more
//       general. If no move is made, need to make originial lattice dots
//       unselectable again.
bool gui::DesignPanel::moveToGhost(bool kill)
{
  bool is_all_floating = true;
  prim::Ghost *ghost = prim::Ghost::instance();
  moving = false;
  // get the move offset
  QPointF offset = (!kill && ghost->valid_hash[snap_coord]) ? ghost->moveOffset() : QPointF();

  if (offset.isNull()) {
    // There is no offset for dbs. Check if selection is all electrodes, and if it is, move them.
    // reset the original lattice dot selectability and return false
    for (prim::Item *item : selectedItems()) {
      if (item->item_type != prim::Item::Electrode &&
          item->item_type != prim::Item::TextLabel) {
        is_all_floating = false;
      }
      setLatticeSiteOccupancy(item, true);
    }

    if (is_all_floating) {//selection is all electrodes. try to move them without snapping.
      //offset is kept inside ghost->pos()
      for (prim::Item *item : ghost->getTopItems())
        undo_stack->push(new MoveItem(item, ghost->pos(), this));
      return true; //things were moved.
    }
    //either no selection, or selection contains non-electrodes. return false as usual.
    return false;
  }

  undo_stack->beginMacro(tr("Move items"));

  // move each source item by the offset.
  for(prim::Item *item : ghost->getTopItems())
    undo_stack->push(new MoveItem(item, offset, this));

  undo_stack->endMacro();
  return true;
}

void gui::DesignPanel::changeItemColors(QColor color)
{
  if (color.isValid()) {
    //change each selected item's fill color to color
    QList<prim::Item *> items = color_dialog->getTargetItems();

    if (items.length()==0)
      qDebug() << "No items selected for color change.";

    int item_index;

    undo_stack->beginMacro(tr("Change Colors"));
    for (auto item: items){
      item_index = layman->getLayer(item->layer_id)->getItemIndex(item);
      undo_stack->push(new ChangeColor(item->layer_id, this, item_index,
            item->getCurrentFillColor(), color, false));
    }
    undo_stack->endMacro();
  } else {
    qDebug() << "Selected color: " << color << " is invalid.";
  }
  color_dialog->clearItems();
}

void gui::DesignPanel::showColorDialog(QList<prim::Item*> target_items)
{
  for (prim::Item* item : target_items)
    color_dialog->show(item);
}

void gui::DesignPanel::showRotateDialog(QList<prim::Item*> target_items)
{
  for (prim::Item* item : target_items)
    rotate_dialog->show(item);
}

void gui::DesignPanel::setItemRotations(double rot)
{
  QList<prim::Item *> items = rotate_dialog->getTargetItems();

  if (items.length() == 0)
    qDebug() << "No items selected for rotation.";

  int item_index;
  undo_stack->beginMacro(tr("Rotate Item"));
  for (auto item: items){
    item_index = layman->getLayer(item->layer_id)->getItemIndex(item);

    undo_stack->push(new RotateItem(item->layer_id, this, item_index,
          (double) static_cast<prim::ResizeRotateRect*>(item)->getAngleDegrees(), rot, false));
  }
  undo_stack->endMacro();

  rotate_dialog->clearItems();
}
