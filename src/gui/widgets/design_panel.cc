// @file:     design_panel.cc
// @author:   Jake
// @created:  2016.11.02
// @editted:  2017.07.11  - Jake
// @license:  GNU LGPL v3
//
// @desc:     DesignPanel implementation

#include "design_panel.h"
#include "src/settings/settings.h"

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
  initDesignPanel();

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
  connect(prim::Emitter::instance(), &prim::Emitter::sig_resizeFinalize,
          this, &gui::DesignPanel::resizeItem);
  connect(prim::Emitter::instance(), &prim::Emitter::sig_moveDBToLatticeCoord,
          this, &gui::DesignPanel::moveDBToLatticeCoord);
  connect(prim::Emitter::instance(), &prim::Emitter::sig_physLoc2LatticeCoord,
          this, &gui::DesignPanel::physLoc2LatticeCoord);
  connect(prim::Emitter::instance(), &prim::Emitter::sig_setLatticeVisibility,
          this, &gui::DesignPanel::updateBackground);
  connect(prim::Emitter::instance(), &prim::Emitter::sig_editTextLabel,
          this, QOverload<prim::Item*, const QString &>::of(&gui::DesignPanel::editTextLabel));
}

// destructor
gui::DesignPanel::~DesignPanel()
{
  clearDesignPanel(false);
}

// initialise design panel on first init or after reset
void gui::DesignPanel::initDesignPanel() {
  undo_stack = new QUndoStack();
  connect(undo_stack, SIGNAL(cleanChanged(bool)),
          this, SLOT(emitUndoStackCleanChanged(bool)));

  // initialize contained widgets
  layman = new LayerManager(this);
  property_editor = new PropertyEditor(this);

  settings::AppSettings *app_settings = settings::AppSettings::instance();

  scene = new QGraphicsScene(this);
  setScene(scene);
  setMouseTracking(true);

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
  resetMatrix(); // resets QTransform, which undoes the zoom
  scale(0.1, 0.1);

  setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
            QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);

  // color scheme
  QColor col;
  setFrameShadow(QFrame::Raised);

  setCacheMode(QGraphicsView::CacheBackground);

  // make lattice and surface layer
  buildLattice();
  setSceneMinSize();

  // initialise the Ghost and set the scene
  prim::Ghost::instance()->setScene(scene);

  // initialise scroll bar position and policies
  verticalScrollBar()->setValue((verticalScrollBar()->minimum()+verticalScrollBar()->maximum())/2);
  horizontalScrollBar()->setValue((horizontalScrollBar()->minimum()+horizontalScrollBar()->maximum())/2);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  // set display mode
  setDisplayMode(DesignMode);


  // construct widgets
  afm_panel = new AFMPanel(layman->indexOf(layman->getMRULayer(prim::Layer::AFMTip)), this);
  scene->addItem(afm_panel->ghostNode());
  scene->addItem(afm_panel->ghostSegment());
  connect(this, &gui::DesignPanel::sig_toolChanged,
            afm_panel, &gui::AFMPanel::toolChangeResponse);
}

// clear design panel
void gui::DesignPanel::clearDesignPanel(bool reset)
{
  // delete child widgets
  delete afm_panel;
  delete property_editor;

  // delete layers and contained items
  delete layman;
  if(reset) prim::Layer::resetLayers(); // reset layer counter

  // delete all graphical items from the scene
  scene->clear();
  //if(!reset) delete scene;
  delete scene;

  // purge the clipboard
  for(prim::Item *item : clipboard)
    delete item;
  clipboard.clear();

  delete undo_stack;

  qDebug() << tr("Finished clearing design panel");
}

// reset
void gui::DesignPanel::resetDesignPanel()
{
  clearDesignPanel(true);
  initDesignPanel();

  // REBUILD

  //let application know that design panel has been reset.
  emit sig_resetDesignPanel();
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
  // add Item
  layer->addItem(item, ind);
  scene->addItem(item);

  // update scene rect
  QRectF sbr = scene->itemsBoundingRect();
  QRectF vp = mapToScene(viewport()->rect()).boundingRect();
  setSceneRect(min_scene_rect | sbr | vp);
  scene->setSceneRect(min_scene_rect | sbr);
}

void gui::DesignPanel::removeItem(prim::Item *item, int layer_index)
{
  removeItem(item, layman->getLayer(layer_index));
}

void gui::DesignPanel::removeItem(prim::Item *item, prim::Layer *layer)
{
  // if layer contains the item, delete and remove froms scene, otherwise
  // do nothing
  if(layer->removeItem(item)){
    scene->removeItem(item);
    delete item;
    QRectF sbr = scene->itemsBoundingRect();
    QRectF vp = mapToScene(viewport()->rect()).boundingRect();
    setSceneRect(min_scene_rect | sbr | vp);
    scene->setSceneRect(min_scene_rect | sbr);
  }
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


QList<prim::Item*> gui::DesignPanel::selectedItems()
{
  QList<prim::Item*> casted_list;
  for (QGraphicsItem *gitem : scene->selectedItems())
    casted_list.append(static_cast<prim::Item*>(gitem));
  return casted_list;
}


QList<prim::DBDot *> gui::DesignPanel::getSurfaceDBs() const
{
  QList<prim::Layer*> db_layers = layman->getLayers(prim::Layer::DB);
  if (db_layers.count() == 0) {
    qWarning() << tr("No DB layers found");
    return QList<prim::DBDot*>();
  }

  // extract all DBDot items from the DB layers
  QList<prim::DBDot *> dbs;
  for (prim::Layer* layer : db_layers)
    for(prim::Item *item : layer->getItems())
      if(item->item_type==prim::Item::DBDot)
        dbs.append(static_cast<prim::DBDot *>(item));

  return dbs;
}


void gui::DesignPanel::buildLattice(const QString &fname)
{

  if(!fname.isEmpty() && DEFAULT_OVERRIDE){
    qWarning() << tr("Cannot change lattice when DEFAULT_OVERRIDE set");
    // do nothing if the lattice has previously been defined
    if(layman->layerCount() != 0)
      return;
  }

  // destroy all layers if they exist
  layman->removeAllLayers();

  // build the new lattice
  lattice = new prim::Lattice(fname, layman->layerCount());
  updateBackground();

  // add the lattice dots to the scene
  for(prim::Item *const item : lattice->getItems())
    scene->addItem(item);

  // add the lattice to the layers, as layer 0
  layman->addLattice(lattice);

  // add in the dangling bond surface
  layman->addLayer("Surface", prim::Layer::DB,0,0);

  // add in the metal layer for electrodes
  layman->addLayer("Metal", prim::Layer::Electrode,-100E-9,10E-9);

  // add in the AFM layer for AFM tip travel paths
  layman->addLayer("AFM", prim::Layer::AFMTip,500E-12,50E-12);

  layman->populateLayerTable();
  layman->initSideWidget();
  emit sig_setLayerManagerWidget(layman->sideWidget());

  layman->setActiveLayer(layman->getLayer("Surface"));
}



void gui::DesignPanel::setSceneMinSize()
{
  // add an invisible rectangle to the scene to set a minimum scene rect
  int min_size = settings::GUISettings::instance()->get<int>("lattice/minsize");
  QPoint bot_right = min_size * (lattice->sceneLatticeVector(0) + lattice->sceneLatticeVector(1));
  min_scene_rect = QRectF(QPoint(0,0),bot_right);
  min_scene_rect.moveCenter(QPoint(0,0));
  //scene->addItem(new QGraphicsRectItem(scene_rect));
  scene->setSceneRect(min_scene_rect); // TODO reenable this line when implementing minimum set scene rect
}


void gui::DesignPanel::setTool(gui::ToolType tool)
{
  // do nothing if tool has not been changed
  if(tool==tool_type)
    return;

  // reset selected items
  scene->clearSelection();

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
    case gui::ToolType::AFMAreaTool:
      layman->setActiveLayer(layman->getMRULayer(prim::Layer::AFMTip));
      setInteractive(true);
      break;
    case gui::ToolType::AFMPathTool:
      layman->setActiveLayer(layman->getMRULayer(prim::Layer::AFMTip));
      setInteractive(true);
      break;
    case gui::ToolType::ScreenshotAreaTool:
      setInteractive(true);
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
  QList<prim::DBDot *> dbs = getSurfaceDBs();
  for (int i=0; i<dbs.count(); i++) {
    if(qAbs(fills[i])>1.)
      qWarning() << tr("Given fill invalid");
    else
      dbs.at(i)->setFill(fills[i]);
  }
}


void gui::DesignPanel::screenshot(QPainter *painter, const QRect &region)
{
  // add lattice dot previews (vector graphics) instead of using the bitmap
  // lattice background
  QList<prim::LatticeCoord> coords = lattice->enclosedSites(region);
  QList<prim::LatticeDotPreview*> latdot_previews;
  for (prim::LatticeCoord coord : coords) {
    if (lattice->isOccupied(coord))
      continue;
    prim::LatticeDotPreview *ldp = new prim::LatticeDotPreview(coord);
    ldp->setPos(lattice->latticeCoord2ScenePos(coord));
    ldp->setZValue(INT_MIN);
    latdot_previews.append(ldp);
    scene->addItem(ldp);
  }

  // render scene onto painter
  prev_screenshot_area = region;
  scene->render(painter, region, region);

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
  updateBackground();
}


// SAVE

void gui::DesignPanel::saveToFile(QXmlStreamWriter *ws, bool for_sim)
{
  if(for_sim){
    // if saving for simulation, do something
  }
  // save gui flags
  ws->writeComment("GUI Flags");
  ws->writeStartElement("gui");

  // save zoom and scroll bar position
  ws->writeTextElement("zoom", QString::number(transform().m11())); // m11 of qtransform
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
  layman->saveLayerItems(ws);
  ws->writeEndElement(); // end of design node
}

void gui::DesignPanel::loadFromFile(QXmlStreamReader *rs)
{
  // reset the design panel state
  resetDesignPanel();

  // read from xml stream and hand nodes off to appropriate functions
  while (rs->readNextStartElement()) {
    // read program flags
    if (rs->name() == "program") {
      // TODO implement
      rs->skipCurrentElement();
    } else if(rs->name() == "gui") {
      loadGUIFlags(rs);
    } else if (rs->name() == "layers") {
      loadLayers(rs);
    } else if(rs->name() == "layer_prop") {
      // starting version 0.0.2 layer_prop should appear inside the layers level
      loadLayerProps(rs);
    } else if(rs->name() == "design") {
      loadDesign(rs);
    } else {
      qDebug() << tr("Design Panel: invalid element encountered on line %1 - %2")
          .arg(rs->lineNumber()).arg(rs->name().toString());
      rs->skipCurrentElement();
    }
  }

  // show error if any
  if(rs->hasError()){
    qCritical() << tr("XML error: ") << rs->errorString().data();
  }
}


void gui::DesignPanel::loadGUIFlags(QXmlStreamReader *rs)
{
  qDebug() << "Loading GUI flags";
  qreal zoom=0.1, scroll_v=0, scroll_h=0;
  while (rs->readNextStartElement()) {
    if (rs->name() == "zoom") {
      zoom = rs->readElementText().toDouble();
    } else if (rs->name() == "scroll") {
      scroll_v = rs->attributes().value("x").toInt();
      scroll_h = rs->attributes().value("y").toInt();
      // no text is being read so the current element has to be explicitly skipped
      rs->skipCurrentElement();
    } else {
      qDebug() << tr("Design Panel: invalid element encountered on line %1 - %2")
          .arg(rs->lineNumber()).arg(rs->name().toString());
      rs->skipCurrentElement();
    }
  }
  setTransform(QTransform(zoom,0,0,zoom,0,0));
  verticalScrollBar()->setValue(scroll_v);
  horizontalScrollBar()->setValue(scroll_h);
  qDebug() << tr("Zoom set to %1, scroll v=%2, h=%3").arg(zoom).arg(scroll_v).arg(scroll_h);
}


void gui::DesignPanel::loadLayers(QXmlStreamReader *rs)
{
  qDebug() << "Loading layers";
  while (rs->readNextStartElement())
    if (rs->name() == "layer_prop")
      loadLayerProps(rs);
}


void gui::DesignPanel::loadLayerProps(QXmlStreamReader *rs)
{
  QString layer_nm;
  float zoffset=0, zheight=0;
  prim::Layer::LayerType layer_type = prim::Layer::DB;
  bool layer_visible=false, layer_active=false;

  // keep reading until end of layer_prop tag
  while (rs->readNextStartElement()) {
    if (rs->name() == "name") {
      layer_nm = rs->readElementText();
    } else if (rs->name() == "type") {
      layer_type = static_cast<prim::Layer::LayerType>(
        QMetaEnum::fromType<prim::Layer::LayerType>().keyToValue(
          rs->readElementText().toStdString().c_str()));
    } else if (rs->name() == "zoffset") {
      zoffset = rs->readElementText().toFloat();
    } else if (rs->name() == "zheight") {
      zheight = rs->readElementText().toFloat();
    } else if (rs->name() == "visible") {
      layer_visible = (rs->readElementText() == "1") ? true : false;
    } else if (rs->name() == "active") {
      layer_active = (rs->readElementText() == "1") ? true : false;
    } else {
      qDebug() << tr("Design Panel: invalid element encountered on line %1 - %2")
          .arg(rs->lineNumber()).arg(rs->name().toString());
      rs->skipCurrentElement();
    }
  }
  // edit layer if it exists, create new otherwise
  qDebug() << tr("Loading layer %1 with type %2").arg(layer_nm).arg(layer_type);
  // TODO rethink this layer loading method
  prim::Layer* load_layer = layman->getLayer(layer_nm);
  if (!load_layer) {
    layman->addLayer(layer_nm);
    load_layer = layman->getLayer(layman->layerCount()-1);
  }
  load_layer->setContentType(layer_type);
  load_layer->setZOffset(zoffset);
  load_layer->setZHeight(zheight);
  load_layer->setVisible(layer_visible);
  load_layer->setActive(layer_active);
}


void gui::DesignPanel::loadDesign(QXmlStreamReader *rs)
{
  qDebug() << "Loading design";
  int layer_id=0;
  while (rs->readNextStartElement()) {
    if (rs->name() == "layer") {
      // recursively populate layer with items
      rs->readNext();
      layman->getLayer(layer_id)->loadItems(rs, scene);
      layer_id++;
    } else {
      qDebug() << tr("Design Panel: invalid element encountered on line %1 - %2")
          .arg(rs->lineNumber()).arg(rs->name().toString());
      rs->skipCurrentElement();
    }
  }
}


// SIMULATION RESULT DISPLAY
void gui::DesignPanel::displaySimResults(prim::SimJob *job, int dist_ind, bool avg_degen)
{
  // TODO in the future, show results in a pop up windows instead of the result screen itself
  setDisplayMode(SimDisplayMode);

  if(!job){
    qDebug() << tr("DisplayPanel: Job pointer invalid");
    return;
  } else if (dist_ind > job->filteredElecDists().size() || job->filteredElecDists().size() == 0) {
    qDebug() << tr("DesignPanel: dist_ind out of range when attempting to display sim results: %1").arg(dist_ind);
    return;
  }

  // grab the list of DBDots in the order of job->physlocs
  db_dots_result.clear();
  qreal scale_factor = settings::GUISettings::instance()->get<qreal>("view/scale_fact");
  for(auto job_pl : job->physlocs){
    QPointF scene_loc;
    scene_loc.setX(scale_factor*job_pl.first);
    scene_loc.setY(scale_factor*job_pl.second);

    bool db_exists = false;
    QList<QGraphicsItem*> items_at_loc = scene->items(scene_loc);
    for(auto i_at_loc : items_at_loc){
      if(static_cast<prim::Item*>(i_at_loc)->item_type == prim::Item::DBDot){
        db_dots_result.append(static_cast<prim::DBDot*>(i_at_loc));
        db_exists = true;
        break;
      }
    }

    if(!db_exists){
      qDebug() << tr("DesignPanel: unable to show result, no DBDot is present "
                     "at location x=%1, y=%2").arg(scene_loc.x()).arg(scene_loc.y());
      return;
    }
  }

  // set their show_elec to the set specified by job->elec_dists
  for(int i=0; i<db_dots_result.size(); i++){
    if (dist_ind == -1) {
      // show average distribution if distribution index is -1
      db_dots_result[i]->setShowElec(job->elec_dists_avg[i]);
      //qDebug() << tr("Setting electron %1 to %2").arg(i).arg(job->elec_dists_avg[i]);
    } else if(db_dots_result[i]) {
      // show the distribution of the selected index
      if (avg_degen) {
        db_dots_result[i]->setShowElec(job->elecDistAvgDegenOfDB(dist_ind, i));
        //qDebug() << tr("Setting electron %1 to %2, averaged").arg(i).arg(job->elecDistAvgDegenOfDB(dist_ind,i));
      } else {
        db_dots_result[i]->setShowElec(job->filteredElecDists().at(dist_ind).dist[i]);
      }
    }
  }
}


void gui::DesignPanel::clearSimResults()
{
  setDisplayMode(DesignMode);

  // set show_elec of all DBDots to 0
  if(!db_dots_result.isEmpty()) {
    for(auto *db : db_dots_result)
      db->setShowElec(0);
    db_dots_result.clear();
  }
  while(!sim_results_items.isEmpty()){
    prim::Item* temp_item = sim_results_items.takeFirst();
    removeItemFromScene(temp_item);
    delete temp_item;
    // temp_item = 0;
  }
}

void gui::DesignPanel::clearPlots()
{
  setDisplayMode(DesignMode);
  for (prim::Item* temp_item: sim_results_items) {
    if (temp_item->item_type == prim::Item::PotPlot) {
      removeItemFromScene(temp_item);
      delete temp_item;
      // temp_item = 0;
    }
  }
}

void gui::DesignPanel::displayPotentialPlot(QImage potential_plot, QRectF graph_container, QMovie *potential_animation)
{
  qDebug() << tr("graph_container height: ") << graph_container.height();
  qDebug() << tr("graph_container width: ") << graph_container.width();
  qDebug() << tr("graph_container topLeft: ") << graph_container.topLeft().x() << tr(", ") << graph_container.topLeft().y();
  clearPlots();
  setDisplayMode(SimDisplayMode);
  createPotPlot(potential_plot, graph_container, potential_animation);

  // QLabel *gif_anim = new QLabel();
  // QMovie *movie = new QMovie("/home/nathan/test.gif");
  // gif_anim->setMovie(movie);
  // movie->start();
  // QGraphicsProxyWidget *proxy = scene->addWidget(gif_anim);
}

// SLOTS

void gui::DesignPanel::selectClicked(prim::Item *)
{
  // for now, if an item is clicked in selection mode, act as though the highest
  // level aggregate was clicked
  if(tool_type == gui::ToolType::SelectTool && display_mode == DesignMode)
    initMove();

}

void gui::DesignPanel::simVisualizeDockVisibilityChanged(bool visible)
{
  if(!visible && display_mode == SimDisplayMode)
    clearSimResults();
}

void gui::DesignPanel::resizeBegin()
{
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
  static_cast<prim::DBDot*>(item)->setPhysLoc(lattice->latticeCoord2PhysLoc(prim::LatticeCoord(n,m,l)));
  setLatticeSiteOccupancy(item, true);
}

void gui::DesignPanel::physLoc2LatticeCoord(QPointF physloc, int &n, int &m, int &l)
{
  prim::LatticeCoord coord = lattice->nearestSite(physloc * prim::Item::scale_factor);
  n = coord.n;
  m = coord.m;
  l = coord.l;
}

void gui::DesignPanel::updateBackground()
{
  QColor col = (display_mode == gui::ScreenshotMode) ? background_col_publish : background_col;
  bool lattice_visible = true;

  if (transform().m11() < zoom_visibility_threshold
      || !lattice->isVisible())
    lattice_visible = false;

  if (lattice_visible)
    scene->setBackgroundBrush(QBrush(lattice->tileableLatticeImage(col, display_mode == gui::ScreenshotMode)));
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
    rubberBandEnd();

  switch(e->button()){
    case Qt::LeftButton:
      if (tool_type == ScreenshotAreaTool) {
        // use rubberband to select screenshot area
        rb_start = mapToScene(e->pos()).toPoint();
        rb_cache = e->pos();

      } else if (tool_type == SelectTool || tool_type == ElectrodeTool ||
          tool_type == AFMAreaTool || tool_type == LabelTool) {
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
        coord_start = lattice->nearestSite(mapToScene(e->pos()));

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

  if (ghosting) {
    // update snap
    QPointF scene_pos = mapToScene(e->pos());
    prim::LatticeCoord offset;
    if (snapGhost(scene_pos, offset)) // if there are db dots
      prim::Ghost::instance()->moveByCoord(offset, lattice);

  } else if (tool_type == AFMPathTool) {
    // update ghost node and ghost segment if there is a focused node, only update
    // ghost node if there's none.
    /* TODO re-enable it later
    QList<prim::Item::ItemType> target_types;
    target_types.append(prim::Item::LatticeDot);
    target_types.append(prim::I
    tem::DBDot);
    prim::Item *snap_target = filteredSnapTarget(mapToScene(e->pos()), target_types, snap_diameter);
    if (snap_target) {
      afm_panel->ghostNode()->setPos(snap_target->scenePos());
      afm_panel->showGhost(true);
    }*/

  } else if (!clicked && tool_type == DBGenTool) {
    // show preview location of new DB
    createDBPreviews({lattice->nearestSite(mapToScene(e->pos()))});

  } else if (clicked) {
    // not ghosting, mouse dragging of some sort
    switch(e->buttons()){
      case Qt::LeftButton:
        if (tool_type == SelectTool || tool_type == ElectrodeTool ||
            tool_type == AFMAreaTool || tool_type == ScreenshotAreaTool ||
            tool_type == LabelTool) {
          rubberBandUpdate(e->pos());
        } else if (tool_type == DBGenTool) {
          createDBPreviews(lattice->enclosedSites(coord_start, lattice->nearestSite(mapToScene(e->pos()))));
        }
        // use default behaviour for left mouse button
        QGraphicsView::mouseMoveEvent(e);
        break;
      case Qt::MidButton:
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
            // filter out items in the lattice
            filterSelection(true);
            break;
          case gui::ToolType::DBGenTool:
            // identify free lattice sites and create dangling bonds
            createDBs();
            break;
          case gui::ToolType::ElectrodeTool:
            // get start and end locations, and create the electrode.
            filterSelection(false);
            createElectrode(rb_scene_rect);
            break;
          case gui::ToolType::AFMAreaTool:
            filterSelection(false);
            createAFMArea(rb_scene_rect);
            break;
          case gui::ToolType::AFMPathTool:
            // Make node at the ghost position
            createAFMNode();
            break;
          case gui::ToolType::ScreenshotAreaTool:
            // take a screenshot of the rubberband area
            filterSelection(false);
            sig_screenshot(rb_scene_rect);
            break;
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
    rubberBandEnd();

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
  QGraphicsView::keyPressEvent(e);
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
        // quit screenshot mode if currently in it
        if (display_mode == ScreenshotMode)
          emit sig_cancelScreenshot();

        // deactivate current tool
        if (tool_type != gui::ToolType::SelectTool) {
          //qDebug() << tr("Esc pressed, drop back to select tool");
          // emit signal to be picked up by application.cc
          emit sig_toolChangeRequest(gui::ToolType::SelectTool);
        }
        break;
      case Qt::Key_S:
        if (display_mode == ScreenshotMode) {
          sig_screenshot(prev_screenshot_area);
        }
        break;
      default:
        QGraphicsView::keyReleaseEvent(e);
        break;
    }
  }
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
  // get list of selected items
  cache.clear();
  cache = selectedItems();
  if(cache.count()==0)
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

  // assert scale limitations
  boundZoom(ds);

  if(ds!=0){
    // zoom under mouse, should be indep of transformationAnchor
    QPointF old_pos = mapToScene(e->pos());

    // pre-zoom scene rect update
    QRectF sbr = scene->itemsBoundingRect();
    QRectF vp = mapToScene(viewport()->rect()).boundingRect();
    if (ds < 0)
      vp.adjust(-vp.width(), -vp.height(), vp.width(), vp.height()); // account for zoom out viewport size
    setSceneRect(min_scene_rect | sbr | vp);

    // perform zoom
    scale(1+ds,1+ds);

    // move to anchor
    QPointF delta = mapToScene(e->pos()) - old_pos;
    scrollDelta(delta); // scroll with anchoring

    // post-zoom scene rect update
    vp = mapToScene(viewport()->rect()).boundingRect();
    setSceneRect(min_scene_rect | sbr | vp);
  }

  // reset both scrolls (avoid repeat from |x|>=120)
  wheel_deg.setX(0);
  wheel_deg.setY(0);

  updateBackground();

  //qDebug() << tr("Zoom: QTransform m11 = %1, m12 = %2, m21 = %3, m22 = %4, dx = %5, dy = %6").arg(transform().m11()).arg(transform().m12()).arg(transform().m21()).arg(transform().m22()).arg(transform().dx()).arg(transform().dy());
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
  if(tool_type == gui::ToolType::SelectTool && display_mode == DesignMode)
    deleteSelection();
}


void gui::DesignPanel::dummyAction()
{
  QPoint pos = sender()->property("pos").toPoint();
  if (itemAt(pos)) {
    QList<QGraphicsItem*> gitems = items(pos);
    for (auto gitem: gitems) {
      //make sure the item type is correct.
      if (static_cast<prim::Item*>(gitem)->item_type == sender()->property("item_type").toInt()) {
        static_cast<prim::Item*>(gitem)->performAction(static_cast<QAction*>(sender()));
      }
    }
  }
}

void gui::DesignPanel::initActions()
{
  action_undo = new QAction(QIcon::fromTheme("edit-undo"), tr("&Undo"), this);
  action_redo = new QAction(QIcon::fromTheme("edit-redo"), tr("&Redo"), this);
  action_cut = new QAction(QIcon::fromTheme("edit-cut"), tr("Cut"), this);
  action_copy = new QAction(QIcon::fromTheme("edit-copy"), tr("&Copy"), this);
  action_paste = new QAction(QIcon::fromTheme("edit-paste"), tr("&Paste"), this);
  action_delete = new QAction(QIcon::fromTheme("edit-delete"), tr("&Delete"), this);
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
  action_dup->setShortcut(tr("D"));

  connect(action_undo, &QAction::triggered, this, &gui::DesignPanel::undoAction);
  connect(action_redo, &QAction::triggered, this, &gui::DesignPanel::redoAction);
  connect(action_cut, &QAction::triggered, this, &gui::DesignPanel::cutAction);
  connect(action_copy, &QAction::triggered, this, &gui::DesignPanel::copyAction);
  connect(action_paste, &QAction::triggered, this, &gui::DesignPanel::pasteAction);
  connect(action_delete, &QAction::triggered, this, &gui::DesignPanel::deleteAction);
  connect(action_form_agg, &QAction::triggered, this, &gui::DesignPanel::formAggregate);
  connect(action_split_agg, &QAction::triggered, this, &gui::DesignPanel::splitAggregates);
  connect(action_dup, &QAction::triggered, this, &gui::DesignPanel::duplicateSelection);

  // add the actions to design panel so they're activated
  addActions({action_undo, action_redo, action_cut, action_copy, action_paste,
              action_delete, action_form_agg, action_split_agg, action_dup});
}


void gui::DesignPanel::filterSelection(bool select_flag)
{
  // should only be here if tool type is one of the following
  if (tool_type != gui::ToolType::SelectTool &&
      tool_type != gui::ToolType::DBGenTool &&
      tool_type != gui::ToolType::ElectrodeTool &&
      tool_type != gui::ToolType::AFMAreaTool &&
      tool_type != gui::ToolType::ScreenshotAreaTool){
    qCritical() << tr("Filtering selection with invalid tool type...");
    return;
  }

  // NOTE need to make sure than only active layers are selectable

  // if select_flag, deselect all items in the lattice. Otherwise, keep only items in the lattice
  for(prim::Item *item : selectedItems()){
    if( (item->layer_id == 0) == select_flag)
      item->setSelected(false);
  }
}


void gui::DesignPanel::rubberBandUpdate(QPoint pos){
  // stop rubber band if moving item
  if(moving || resizing){
    rubberBandEnd();
    return;
  }

  // do nothing if mouse hasn't moved much
  // TODO change snap_diameter to a separate variable
  if((pos-rb_cache).manhattanLength()<.01*snap_diameter)
    return;
  rb_cache = pos;

  if(!rb){
    // make rubber band
    rb = new QRubberBand(QRubberBand::Rectangle, this);
    rb->setGeometry(QRect(mapFromScene(rb_start), QSize()));
    rb->show();
  }
  else{
    // update rubberband rectangle
    rb->setGeometry(QRect(mapFromScene(rb_start), pos).normalized());
    rb_scene_rect = QRect(rb_start, mapToScene(pos).toPoint()).normalized();

    // deselect all items
    QList<QGraphicsItem*> selected_items = scene->selectedItems();
    for(QGraphicsItem* selected_item : selected_items)
      selected_item->setSelected(false);

    // select items that are now enclosed by the rubberband
    QList<QGraphicsItem*> rb_items = scene->items(rb_scene_rect);
    for(QGraphicsItem* rb_item : rb_items)
      rb_item->setSelected(true);

    // append shift-selected items
    for(QGraphicsItem* shift_selected_item : rb_shift_selected)
      shift_selected_item->setSelected(true);
  }
}


void gui::DesignPanel::rubberBandEnd(){
  if(rb){
    rb->hide();
    rb = 0;
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
    filterSelection(true);
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
        item->item_type != prim::Item::AFMArea &&
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
    prim::LatticeCoord nearest_site = lattice->nearestSite(free_anchor);

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

  qDebug() << tr("Clipboard: %1 items").arg(clipboard.count());
  for(prim::Item *item : clipboard){
    if(item->item_type == prim::Item::Aggregate){
      qDebug() << tr("  aggregate");
    }
    else
      qDebug() << tr("  item at: %1,%2").arg(item->x()).arg(item->y());

  }
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
gui::DesignPanel::CreatePotPlot::CreatePotPlot(gui::DesignPanel *dp, QImage potential_plot, QRectF graph_container, QMovie *potential_animation, prim::PotPlot *pp, bool invert, QUndoCommand *parent)
  : QUndoCommand(parent), dp(dp), potential_plot(potential_plot), graph_container(graph_container), potential_animation(potential_animation), pp(pp), invert(invert)
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
  pp = new prim::PotPlot(potential_plot, graph_container);
  dp->addItemToScene(static_cast<prim::Item*>(pp));
  dp->sim_results_items.append(static_cast<prim::Item*>(pp));
}

void gui::DesignPanel::CreatePotPlot::destroy()
{
  if(pp != 0){
    dp->removeItemFromScene(static_cast<prim::Item*>(pp));  // deletes PotPlot
    dp->sim_results_items.removeOne(static_cast<prim::Item*>(pp));
    pp = 0;
  }
}


// CreateAFMPath class

gui::DesignPanel::CreateAFMPath::CreateAFMPath(int layer_index, gui::DesignPanel *dp,
                        prim::AFMPath *afm_path, bool invert, QUndoCommand *parent)
  : QUndoCommand(parent), invert(invert), dp(dp), layer_index(layer_index)
{
  //qDebug() << tr("Entered CreateAFMPath");
  prim::Layer *layer = dp->layman->getLayer(layer_index);
  index = invert ? layer->getItemIndex(afm_path) : layer->getItems().size();
  if (index == -1)
    qCritical() << tr("Index for AFMPath is -1, this shouldn't happen.");
}

void gui::DesignPanel::CreateAFMPath::undo()
{
  invert ? create() : destroy();
}

void gui::DesignPanel::CreateAFMPath::redo()
{
  invert ? destroy() : create();
}

void gui::DesignPanel::CreateAFMPath::create()
{
  //qDebug() << tr("Entered CreateAFMPath::create()");
  prim::AFMPath *new_path = new prim::AFMPath(layer_index);
  dp->addItem(new_path, layer_index, index);
  dp->afmPanel()->setFocusedPath(new_path);
}

void gui::DesignPanel::CreateAFMPath::destroy()
{
  //qDebug() << tr("Entered CreateAFMPath::destroy()");
  prim::AFMPath *afm_path = static_cast<prim::AFMPath*>(dp->layman->getLayer(layer_index)->getItem(index));

  // contained nodes and segments should be deleted automatically when deleting the path
  // since they're children items to the path.
  dp->removeItem(afm_path, dp->layman->getLayer(afm_path->layer_id));
  dp->afmPanel()->setFocusedPath(0);
}


// CreateAFMNode class

gui::DesignPanel::CreateAFMNode::CreateAFMNode(int layer_index, gui::DesignPanel *dp,
                        QPointF scenepos, float z_offset, int afm_index,
                        int index_in_path, bool invert, QUndoCommand *parent)
  : QUndoCommand(parent), invert(invert), layer_index(layer_index), dp(dp),
      afm_index(afm_index), scenepos(scenepos), z_offset(z_offset)
{
  prim::AFMPath *afm_path = static_cast<prim::AFMPath*>(dp->layman->getLayer(layer_index)->getItem(afm_index));
  node_index = (index_in_path == -1) ? afm_path->nodeCount() : index_in_path;
}

void gui::DesignPanel::CreateAFMNode::undo()
{
  invert ? create() : destroy();
}

void gui::DesignPanel::CreateAFMNode::redo()
{
  invert ? destroy() : create();
}

void gui::DesignPanel::CreateAFMNode::create()
{
  //qDebug() << tr("Entered CreateAFMNode::create()");
  prim::AFMPath *afm_path = static_cast<prim::AFMPath*>(dp->layman->getLayer(layer_index)->getItem(afm_index));
  prim::AFMNode *new_node = new prim::AFMNode(layer_index, scenepos, z_offset);
  afm_path->insertNode(new_node, node_index);

  dp->afmPanel()->setFocusedPath(afm_path);
  dp->afmPanel()->setFocusedNodeIndex(node_index);
}

void gui::DesignPanel::CreateAFMNode::destroy()
{
  //qDebug() << tr("Entered CreateAFMNode::destroy()");
  prim::AFMPath *afm_path = static_cast<prim::AFMPath*>(dp->layman->getLayer(layer_index)->getItem(afm_index));
  afm_path->removeNode(node_index); // pointer cleanup is done by AFMPath
  dp->afmPanel()->setFocusedNodeIndex(node_index-1);
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
  for(const int &ind : item_inds){
    if(ind >= layer_items.size())
      qFatal("Undo/Redo mismatch... something went wrong");
    item = layer_items.at(ind);
    if(item->layer_id != layer_index || item->parentItem() != 0)
      qFatal("Undo/Redo mismatch... something went wrong");
  }

  // remove the items from the Layer stack in reverse order
  QStack<prim::Item*> items;
  for(int i=item_inds.count()-1; i>=0; i--)
    items.push(layer->takeItem(item_inds.at(i)));

  // remove all Items from the scene
  for(prim::Item *item : items)
    if(item != 0)
      item->scene()->removeItem(item);

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
  prim::LatticeCoord coord = dp->lattice->nearestSite(new_pos, nearest_site_pos);
  if (dp->lattice->collidesWithLatticeSite(new_pos, coord)) {
    // set the previous site as unoccupied if that site still points to this dot
    if (dp->lattice->dbAt(dot->latticeCoord()) == dot)
      dp->lattice->setUnoccupied(dot->latticeCoord());
    dot->setLatticeCoord(coord);
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

QList<QStringList> gui::DesignPanel::cleanItemArgs(QStringList item_args)
{
  QList<QStringList> clean_args = QList<QStringList>();
  for (QString arg: item_args){
    //strip parentheses
    arg.remove("(");
    arg.remove(")");
    // return arguments enclosed in parentheses in sets.
    clean_args.append(arg.split(" ", QString::SkipEmptyParts));
  }
  return clean_args;
}


bool gui::DesignPanel::commandCreateItem(QString type, QString layer_id, QStringList item_args)
{
  prim::Item::ItemType item_type = prim::Item::getEnumItemType(type);
  QList<QStringList> clean_args = cleanItemArgs(item_args);
  switch(item_type){
    case prim::Item::Electrode:
      if ((clean_args.size() == 2) && (clean_args.first().size() == 2) && (clean_args.last().size()) == 2) {
        setTool(gui::ToolType::ElectrodeTool);
        emit sig_toolChangeRequest(gui::ToolType::ElectrodeTool);
        int layer_index = (layer_id == "auto") ? layman->indexOf(layman->activeLayer()) : layer_id.toInt();
        prim::Electrode* elec = new prim::Electrode(layer_index, clean_args);
        undo_stack->push(new CreateItem(layer_index, this, elec));
        emit sig_toolChangeRequest(gui::ToolType::SelectTool);
        return true;
      }
      break;
    case prim::Item::AFMArea:
      if ((clean_args.size() == 2) && (clean_args.first().size() == 2) && (clean_args.last().size()) == 2) {
        setTool(gui::ToolType::AFMAreaTool);
        emit sig_toolChangeRequest(gui::ToolType::AFMAreaTool);
        int layer_index = (layer_id == "auto") ? layman->indexOf(layman->activeLayer()) : layer_id.toInt();
        prim::AFMArea* afm_area = new prim::AFMArea(layer_index, clean_args);
        undo_stack->push(new CreateItem(layer_index, this, afm_area));
        emit sig_toolChangeRequest(gui::ToolType::SelectTool);
        return true;
      }
      break;
    case prim::Item::DBDot:
      if ((clean_args.size() == 1) && (clean_args[0].size() == 3)) {
        int n = clean_args.first()[0].toInt();
        int m = clean_args.first()[1].toInt();
        int l = clean_args.first()[2].toInt();
        if ((l == 0) || (l == 1)) {  // Check for valid
          setTool(gui::ToolType::DBGenTool);
          int layer_index = (layer_id == "auto") ? layman->indexOf(layman->activeLayer()) : layer_id.toInt();
          emit sig_toolChangeRequest(gui::ToolType::DBGenTool);
          createDBs(prim::LatticeCoord(n, m, l));
          emit sig_toolChangeRequest(gui::ToolType::SelectTool);
          return true;
        }
      }
      break;
    default:
      return false;
  }
}

bool gui::DesignPanel::commandRemoveItem(QString type, QStringList item_args)
{
  prim::Item::ItemType item_type = prim::Item::getEnumItemType(type);
  QList<QStringList> clean_args = cleanItemArgs(item_args);
  if ((clean_args.size() == 1) && (clean_args.first().size() == 2)) {
    QStringList point = clean_args.first();
    // Remove items by location
    float x = point.first().toFloat();
    float y = point.last().toFloat();
    QPointF pos = QPointF(x,y)*prim::Item::scale_factor;
    if (itemAt(mapFromScene(pos))) {
      QList<QGraphicsItem*> gitems = items(mapFromScene(pos));
      for (QGraphicsItem* item: gitems){
        if (static_cast<prim::Item*>(item)->item_type == item_type) {
          commandRemoveHandler(static_cast<prim::Item*>(item));
        }
      }
      return true;
    }
  } else if ((clean_args.size() == 2) && (clean_args[0].size() == 1) && (clean_args[1].size() == 1)) {
    // Remove items by indices
    int lay_id = clean_args.first().first().toInt();
    int item_id = clean_args.last().first().toInt();
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


bool gui::DesignPanel::commandMoveItem(QString type, QStringList item_args)
{
  prim::Item::ItemType item_type = prim::Item::getEnumItemType(type);
  QList<QStringList> clean_args = cleanItemArgs(item_args);
  if ((clean_args.size() == 2) && (clean_args.first().size() == 2)) {
    QStringList point = clean_args.takeFirst();
    QPointF pos = QPointF(point.first().toFloat()*prim::Item::scale_factor, point.last().toFloat()*prim::Item::scale_factor);
    QPointF offset = findMoveOffset(clean_args.first());
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
  else if ((clean_args.size() == 3) && (clean_args[0].size() == 1)
        && (clean_args[1].size() == 1)) {
    int lay_id = clean_args.takeFirst().first().toInt();
    int item_id = clean_args.takeFirst().first().toInt();
    prim::Layer *layer = layman->getLayer(lay_id);
    if (layer) {
      prim::Item *item = layer->getItem(item_id);
      if (item) {
        QPointF offset = findMoveOffset(clean_args.first());
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

QPointF gui::DesignPanel::findMoveOffset(QStringList clean_args)
{
  QPointF offset;
  if (clean_args.size() == 2) {
    offset = QPointF(clean_args.first().toFloat(), clean_args.last().toFloat());
  } else if (clean_args.size() == 3) {
    int n = clean_args[0].toInt();
    int m = clean_args[1].toInt();
    int l = clean_args[2].toInt();
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
  int layer_index = layman->indexOf(layman->activeLayer());
  QList<prim::LatticeCoord> lat_list = QList<prim::LatticeCoord>();
  // since l = -1 is invalid, default is set to n = m = l = -1
  if (lat_coord == prim::LatticeCoord(-1,-1,-1)) {
    // multiple creation, from using tool
    for (prim::DBDotPreview *db_prev : db_previews)
      lat_list.append(db_prev->latticeCoord());
    destroyDBPreviews();
  } else {
    //single creation, using command
    lat_list.append(lat_coord);
  }
  undo_stack->beginMacro(tr("create dangling bonds"));
  for (prim::LatticeCoord lc: lat_list)
    undo_stack->push(new CreateDB(lc, layer_index, this));
  undo_stack->endMacro();
}

void gui::DesignPanel::createElectrode(QRect scene_rect)
{
  int layer_index = layman->indexOf(layman->activeLayer());
  //only ever create one electrode at a time
  undo_stack->beginMacro(tr("create electrode with given corners"));
  undo_stack->push(new CreateItem(layer_index, this,
                                  new prim::Electrode(layer_index, scene_rect)));
  undo_stack->endMacro();
}

void gui::DesignPanel::createPotPlot(QImage potential_plot, QRectF graph_container, QMovie *potential_animation)
{
  // int layer_index = layman->indexOf(layman->getMRULayer(prim::Layer::Plot));
  undo_stack->beginMacro(tr("create potential plot with given corners"));
  undo_stack->push(new CreatePotPlot(this, potential_plot, graph_container, potential_animation));
  undo_stack->endMacro();
}

void gui::DesignPanel::createAFMArea(QRect scene_rect)
{
  int layer_index = layman->indexOf(layman->activeLayer());

  // create the AFM area
  undo_stack->beginMacro(tr("create AFM area with given corners"));
  undo_stack->push(new CreateItem(layer_index, this,
                                  new prim::AFMArea(layer_index, scene_rect)));
  undo_stack->endMacro();
}

void gui::DesignPanel::createAFMNode()
{
  //qDebug() << tr("Entered createAFMNode()");
  int layer_index = layman->indexOf(layman->activeLayer());
  QPointF scene_pos;
  if (afm_panel->ghostNode())
    scene_pos = afm_panel->ghostNode()->scenePos();
  else
    scene_pos = press_scene_pos;

  // TODO UNDOable version
  undo_stack->beginMacro(tr("create AFMNode in the focused AFMPath after the focused AFMNode"));
  if (!afm_panel->focusedPath()) {
    // create new path if there's no focused path
    prim::AFMPath *new_path = new prim::AFMPath(layer_index);
    afm_panel->setFocusedPath(new_path);
    undo_stack->push(new CreateAFMPath(layer_index, this));
  }
  prim::Layer *afm_layer = layman->activeLayer();
  int afm_path_index = afm_layer->getItemIndex(afm_panel->focusedPath());
  undo_stack->push(new CreateAFMNode(layer_index, this, scene_pos, afm_layer->zOffset(),
                                        afm_path_index));
  undo_stack->endMacro();
}

void gui::DesignPanel::createTextLabel(const QRect &scene_rect)
{
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

void gui::DesignPanel::resizeItem(prim::Item *item,
    const QRectF &orig_rect, const QRectF &new_rect)
{
  resizing = false;

  // assume all resizable items are simply ResizableRects right now, need
  // special implementation otherwise
  if (item->isResizable()) {
    int item_index = layman->getLayer(item->layer_id)->getItemIndex(item);
    undo_stack->beginMacro(tr("Resize Item"));
    undo_stack->push(new ResizeItem(item->layer_id, this, item_index,
          orig_rect, new_rect, true));
    undo_stack->endMacro();
  }
}

void gui::DesignPanel::destroyAFMPath(prim::AFMPath *afm_path)
{
  undo_stack->beginMacro(tr("Remove AFM Path and contained nodes"));

  // destroy children nodes
  qDebug() << "delete children nodes";
  prim::AFMNode *afm_node;
  while (afm_path->getLastNode()) {
    //qDebug() << "getting last node";
    afm_node = afm_path->getLastNode();

    //qDebug() << "getting afm and node indices";
    int afm_index = layman->getLayer(afm_node->layer_id)->getItemIndex(afm_path);
    int node_index = afm_path->getNodeIndex(afm_node);
    //qDebug() << tr("pushing to undo stack. layer_id=%1, zoffset=%2, afm_index=%3, node_index=%4").arg(afm_node->layer_id).arg(afm_node->zOffset()).arg(afm_index).arg(node_index);
    undo_stack->push(new CreateAFMNode(afm_node->layer_id, this, afm_node->scenePos(),
          afm_node->zOffset(), afm_index, node_index, true));
  }

  // destroy empty path
  //qDebug() << "delete afmpath";
  undo_stack->push(new CreateAFMPath(afm_path->layer_id, this, afm_path, true));

  undo_stack->endMacro();
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
      case prim::Item::AFMPath:
        destroyAFMPath(static_cast<prim::AFMPath*>(item));
        break;
      case prim::Item::AFMNode:
        {
        prim::AFMNode *afm_node = static_cast<prim::AFMNode*>(item);
        prim::AFMPath *afm_path = static_cast<prim::AFMPath*>(afm_node->parentItem());
        int afm_index = layman->getLayer(afm_node->layer_id)->getItemIndex(afm_path);
        int node_index = afm_path->getNodeIndex(afm_node);
        undo_stack->push(new CreateAFMNode(afm_node->layer_id, this,
            afm_node->scenePos(), afm_node->zOffset(), afm_index, node_index, true));
        qDebug() << "shouldn't be here yet";
        break;
        }
      case prim::Item::PotPlot:
        {
        prim::PotPlot *pp = static_cast<prim::PotPlot*>(item);
        undo_stack->push(new CreatePotPlot(this,
            pp->getPotentialPlot(), pp->getGraphContainer(), pp->getPotentialAnimation(),
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


void gui::DesignPanel::formAggregate()
{
  // do something only if there is a selection
  QList<prim::Item*> selection = selectedItems();
  if(selection.isEmpty())
    return;

  // check if selected items are on the surface
  for(prim::Item *item : selection){
    // if(items.last()->layer != layers.at(1)){ DOUBLE CHECK WITH JAKE
    if(item->layer_id != 1){
      qCritical() << tr("Selected aggregate item not in the surface...");
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
        static_cast<prim::Item*>(gitem)->item_type != prim::Item::AFMArea &&
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

  for(prim::Item *item: cache)
    item->setSelected(true);

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
    case prim::Item::AFMArea:
      pasteAFMArea(ghost, n, static_cast<prim::AFMArea*>(item));
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

void gui::DesignPanel::pasteElectrode(prim::Ghost *ghost, int n, prim::Electrode *elec)
{
  QRectF rect = elec->sceneRect();
  rect.moveTopLeft(ghost->pos()+rect.topLeft());
  undo_stack->beginMacro(tr("create electrode with given corners"));
  undo_stack->push(new CreateItem(elec->layer_id, this,
                                  new prim::Electrode(elec->layer_id, rect)));
  undo_stack->endMacro();
}

void gui::DesignPanel::pasteAFMArea(prim::Ghost *ghost, int n, prim::AFMArea *afm_area)
{
  QRectF rect = afm_area->sceneRect();
  rect.moveTopLeft(ghost->pos()+rect.topLeft());
  undo_stack->beginMacro(tr("create AFMArea with given afm_area params"));
  // TODO copy AFM tip attributes
  undo_stack->push(new CreateItem(afm_area->layer_id, this,
                                  new prim::AFMArea(afm_area->layer_id, rect)));
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
          item->item_type != prim::Item::AFMArea &&
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
