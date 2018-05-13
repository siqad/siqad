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

  // initialize contained widgets
  property_editor = new PropertyEditor(this);

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
}

// destructor
gui::DesignPanel::~DesignPanel()
{
  clearSimResults();        // clear simulation results first if they're being shown
  clearDesignPanel(false);
}

// initialise design panel on first init or after reset
void gui::DesignPanel::initDesignPanel() {
  undo_stack = new QUndoStack();
  connect(undo_stack, SIGNAL(cleanChanged(bool)),
          this, SLOT(emitUndoStackCleanChanged(bool)));

  settings::AppSettings *app_settings = settings::AppSettings::instance();

  scene = new QGraphicsScene(this);
  setScene(scene);
  setMouseTracking(true);

  // setup flags
  clicked = ghosting = moving = pasting = resizing = false;

  // initialising parameters
  snap_diameter = app_settings->get<qreal>("snap/diameter")*prim::Item::scale_factor;
  qDebug() << tr("SD: %1").arg(snap_diameter);
  snap_target = prim::LatticeCoord();

  tool_type = gui::ToolType::NoneTool;     // now setTool will update the tool

  // set view behaviour
  setTransformationAnchor(QGraphicsView::NoAnchor);
  setResizeAnchor(QGraphicsView::AnchorViewCenter);
  resetMatrix(); // resets QTransform, which undoes the zoom
  scale(0.1, 0.1);

  setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
            QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);

  // color scheme
  QColor col;
  //setBackgroundBrush(QBrush(gui_settings->get<QColor>("view/bg_col")));
  setFrameShadow(QFrame::Raised);

  setCacheMode(QGraphicsView::CacheBackground);

  //createActions
  createActions();

  // make lattice and surface layer
  buildLattice();
  setScenePadding();

  // initialise the Ghost and set the scene
  prim::Ghost::instance()->setScene(scene);

  // set scroll to top left
  verticalScrollBar()->setValue(verticalScrollBar()->minimum());
  horizontalScrollBar()->setValue(horizontalScrollBar()->minimum());

  // set display mode
  setDisplayMode(DesignMode);


  // construct widgets
  afm_panel = new AFMPanel(getLayerIndex(afm_layer), this);
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

  // delete all graphical items from the scene
  scene->clear();
  //if(!reset) delete scene;
  delete scene;

  // purge the clipboard
  for(prim::Item *item : clipboard)
    delete item;
  clipboard.clear();

  // delete all the layers
  for(prim::Layer *layer : layers)
    delete layer;
  layers.clear();
  if(reset) prim::Layer::resetLayers();

  delete undo_stack;
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
  if(layer_index == 0 || layer_index >= layers.count()){
    qCritical() << tr("Invalid layer index");
    return;
  }

  prim::Layer *layer = layer_index > 0 ? layers.at(layer_index) : top_layer;
  if(ind > layer->getItems().count()){
    qCritical() << tr("Invalid item index");
    return;
  }
  // add Item
  layer->addItem(item, ind);
  scene->addItem(item);
}

void gui::DesignPanel::removeItem(prim::Item *item, prim::Layer *layer)
{
  // if layer contains the item, delete and remove froms scene, otherwise
  // do nothing
  if(layer->removeItem(item)){
    scene->removeItem(item);
    delete item;
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

void gui::DesignPanel::addLayer(const QString &name, const prim::Layer::LayerType cnt_type, const float zoffset, const float zheight)
{
  // check if name already taken
  bool taken = false;
  for(prim::Layer *layer : layers)
    if(layer->getName() == name){
      taken = true;
      break;
    }

  if(taken){
    qWarning() << tr("A layer already exists with the name : %1").arg(name);
    return;
  }

  // layer is added to the end of layers stack, so ID = layers.size() before it was added
  prim::Layer *layer = new prim::Layer(name, cnt_type, zoffset, zheight, layers.size());
  layers.append(layer);
}

void gui::DesignPanel::removeLayer(const QString &name)
{
  bool removed = false;
  for(int i=0; i<layers.count(); i++){
    if(layers.at(i)->getName() == name){
      removeLayer(i);
      removed=true;
      break;
    }
  }

  if(!removed)
    qWarning() << tr("Failed to remove layer : %1").arg(name);

}

void gui::DesignPanel::removeLayer(int n)
{
  if(n<0 ||  n>= layers.count())
    qWarning() << tr("Layer index out of bounds...");
  else{
    // delete all items in the layer
    prim::Layer *layer = layers.at(n);
    for(prim::Item *item : layer->getItems())
      removeItem(item, layer);

    // delete layer
    delete layer;
    layers.removeAt(n);

    // update layer_id for subsequent layers in the stack and their contained items
    for(int i=n; i<layers.count(); i++)
      layers.at(i)->setLayerIndex(i);

    // if top_layer was removed, default to surface if available else NULL
    if(top_layer==layer)
      top_layer = layers.count() > 1 ? layers.at(1) : 0;
  }

}


prim::Layer* gui::DesignPanel::getLayer(const QString &name) const
{
  for(prim::Layer *layer : layers)
    if(layer->getName() == name)
      return layer;

  // no layer had a matching name, return 0
  qWarning() << tr("Failed to find layer : %1").arg(name);
  return 0;
}


prim::Layer* gui::DesignPanel::getLayer(int n) const
{
  if(n<0 || n>=layers.count()){
    qWarning() << tr("Layer index out of bounds...");
    return 0;
  }
  else
    return layers.at(n);
}


int gui::DesignPanel::getLayerIndex(prim::Layer *layer) const
{
  return layer==0 ? layers.indexOf(top_layer) : layers.indexOf(layer);
}


QList<prim::DBDot *> gui::DesignPanel::getSurfaceDBs() const
{
  if(layers.count()<2){
    qWarning() << tr("Requested non-existing surface...");
    return QList<prim::DBDot *>();
  }

  // extract all DBDot items from the first (surface) layer.
  QList<prim::DBDot *> dbs;
  for(prim::Item *item : layers.at(1)->getItems())
    if(item->item_type==prim::Item::DBDot)
      dbs.append(static_cast<prim::DBDot *>(item));

  return dbs;
}

void gui::DesignPanel::setLayer(const QString &name)
{
  for(prim::Layer *layer : layers)
    if(layer->getName() == name){
      top_layer = layer;
      return;
    }

  // no layer had a matching name, do nothing...
  qWarning() << tr("Failed to find layer : %1").arg(name);
}

void gui::DesignPanel::setLayer(int n)
{
  if(n<0 || n>=layers.count())
    qWarning() << tr("Layer index out of bounds...");
  else
    top_layer = layers.at(n);
}

void gui::DesignPanel::buildLattice(const QString &fname)
{

  if(!fname.isEmpty() && DEFAULT_OVERRIDE){
    qWarning() << tr("Cannot change lattice when DEFAULT_OVERRIDE set");
    // do nothing if the lattice has previously been defined
    if(!layers.isEmpty())
      return;
  }


  // NOTE: probably want a prompt to make sure user want to change the lattice

  // destroy all layers if they exist
  while(layers.count()>0)
    removeLayer(0);

  // build the new lattice
  lattice = new prim::Lattice(fname, layers.size());
  scene->setBackgroundBrush(QBrush(lattice->tileableLatticeImage(background_col)));

  // add the lattice dots to the scene
  for(prim::Item *const item : lattice->getItems())
    scene->addItem(item);

  // add the lattice to the layers, as layer 0
  layers.append(lattice);

  // TODO transfer all the hard-coded layer potitions to actual functions in LayerManager

  // add in the dangling bond surface
  addLayer(tr("Surface"),prim::Layer::DB,0,0);
  top_layer = layers.at(1);

  // add in the metal layer for electrodes
  addLayer(tr("Metal"),prim::Layer::Electrode,-100E-9,10E-9);
  electrode_layer = layers.at(2);

  // add in the AFM layer for AFM tip travel paths
  addLayer(tr("AFM"), prim::Layer::AFMTip,500E-12,50E-12);
  afm_layer = layers.at(3);

  // add in the potential layer for potential plots
  addLayer(tr("Plot"), prim::Layer::Plot,-50E-9,0);
  plot_layer = layers.at(4);
}



void gui::DesignPanel::setScenePadding()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  // resize the scene with padding
  QRectF rect = scene->sceneRect();
  qreal pad = qMin(rect.width(), rect.height())*gui_settings->get<qreal>("view/padding");
  rect.adjust(-.5*pad, -.5*pad, pad, pad);
  scene->setSceneRect(rect);
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
      // replaced with custom rubberBandUpdate, delete this later
      setDragMode(QGraphicsView::NoDrag);
      setInteractive(true);
      break;
    case gui::ToolType::DragTool:
      setDragMode(QGraphicsView::ScrollHandDrag);
      setInteractive(false);
      break;
    case gui::ToolType::DBGenTool:
      // replaced with custom rubberBandUpdate, delete this later
      setDragMode(QGraphicsView::NoDrag);
      setInteractive(true);
      break;
    case gui::ToolType::ElectrodeTool:
      // replaced with custom rubberBandUpdate, delete this later
      setDragMode(QGraphicsView::NoDrag);
      setInteractive(true);
      break;
    case gui::ToolType::AFMAreaTool:
      setInteractive(true);
      break;
    case gui::ToolType::AFMPathTool:
      setInteractive(true);
      break;
    case gui::ToolType::ScreenshotAreaTool:
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
  prev_screenshot_area = region;
  scene->render(painter, region, region);
}


void gui::DesignPanel::setDisplayMode(DisplayMode mode)
{
  display_mode = mode;
  prim::Item::display_mode = mode;

  /*if (mode == gui::ScreenshotMode)
    setBackgroundBrush(QBrush(background_col_publish));
  else
    setBackgroundBrush(QBrush(background_col));*/
}


// SAVE

void gui::DesignPanel::saveToFile(QXmlStreamWriter *stream, bool for_sim)
{
  if(for_sim){
    // if saving for simulation, do something
  }
  // save gui flags
  stream->writeComment("GUI Flags");
  stream->writeStartElement("gui");

  // save zoom and scroll bar position
  stream->writeTextElement("zoom", QString::number(transform().m11())); // m11 of qtransform
  stream->writeEmptyElement("scroll");
  stream->writeAttribute("x", QString::number(verticalScrollBar()->value()));
  stream->writeAttribute("y", QString::number(horizontalScrollBar()->value()));

  // TODO lattice type

  stream->writeEndElement();

  // save layer properties
  stream->writeComment("Layer Properties");
  stream->writeComment("Layer ID is intrinsic to the layer order");
  for(prim::Layer *layer : layers){
    layer->saveLayer(stream);
  }

  // save item hierarchy
  stream->writeComment("Item Hierarchy");
  stream->writeStartElement("design");
  for(prim::Layer *layer : layers){
    stream->writeComment(layer->getName());
    layer->saveItems(stream);
  }
  stream->writeEndElement(); // end design level
}

void gui::DesignPanel::loadFromFile(QXmlStreamReader *stream)
{
  int layer_id=0;
  QString layer_nm;
  float zoffset, zheight;
  prim::Layer::LayerType layer_type;
  bool layer_visible, layer_active;

  // reset the design panel state
  resetDesignPanel();

  // read from XML stream (children will be created recursively, add those children to stack)
  while(!stream->atEnd()){
    if(stream->isStartElement()){
      // read program flags
      if(stream->name() == "program"){
        // TODO implement
        stream->readNext();
      }
      // read GUI flags
      else if(stream->name() == "gui"){
        stream->readNext();
        // keep reading until end of gui tag
        while(stream->name() != "gui"){
          if(stream->isStartElement()){
            qreal zoom=0.1, scroll_v=0, scroll_h=0;
            if(stream->name() == "zoom"){
              zoom = stream->readElementText().toDouble();
            }
            else if(stream->name() == "scroll"){
              for(QXmlStreamAttribute &attr : stream->attributes()){
                if(attr.name().toString() == QLatin1String("x"))
                  scroll_v = attr.value().toInt();
                else if(attr.name().toString() == QLatin1String("y"))
                  scroll_h = attr.value().toInt();
              }
              setTransform(QTransform(zoom,0,0,zoom,0,0));
              verticalScrollBar()->setValue(scroll_v);
              horizontalScrollBar()->setValue(scroll_h);
            }
            else{
              qDebug() << QObject::tr("Design Panel: invalid element encountered on line %1 - %2").arg(stream->lineNumber()).arg(stream->name().toString());
            }
            stream->readNext();
          }
          else
            stream->readNext();
        }
      }
      else if(stream->name() == "layer_prop"){
        // construct layers
        stream->readNext();

        //bool visible_ld, active_ld;
        layer_nm = QString();
        layer_type = prim::Layer::DB;
        zheight = 0;
        zoffset = 0;
        layer_visible = layer_active = false;

        // keep reading until end of layer_prop tag
        while(stream->name() != "layer_prop"){
          if(stream->isStartElement()){
            if(stream->name() == "name"){
              layer_nm = stream->readElementText();
              stream->readNext();
            }
            else if(stream->name() == "type"){
              layer_type = static_cast<prim::Layer::LayerType>(QMetaEnum::fromType<prim::Layer::LayerType>().keyToValue(stream->readElementText().toStdString().c_str()));
              stream->readNext();
            }
            else if(stream->name() == "zoffset"){
              zoffset = stream->readElementText().toFloat();
              stream->readNext();
            }
            else if(stream->name() == "zheight"){
              zheight = stream->readElementText().toFloat();
              stream->readNext();
            }
            else if(stream->name() == "visible"){
              layer_visible = (stream->readElementText() == "1")?1:0;
              stream->readNext();
            }
            else if(stream->name() == "active"){
              layer_active = (stream->readElementText() == "1")?1:0;
              stream->readNext();
            }
            else{
              qDebug() << QObject::tr("Design Panel: invalid element encountered on line %1 - %2").arg(stream->lineNumber()).arg(stream->name().toString());
              stream->readNext();
            }
          }
          else{
            stream->readNext();
          }
        }
        // edit layer if it exists, create new otherwise
        qDebug() << tr("Loading layer %1 with type %2").arg(layer_nm).arg(layer_type);
        prim::Layer* load_layer = getLayer(layer_nm);
        if (!load_layer) {
          addLayer(layer_nm);
          load_layer = getLayer(layers.count()-1);
        }
        load_layer->setContentType(layer_type);
        load_layer->setZOffset(zoffset);
        load_layer->setZHeight(zheight);
        load_layer->setVisible(layer_visible);
        load_layer->setActive(layer_active);
      }
      else if(stream->name() == "design") {
        stream->readNext();
        while(stream->name() != "design"){
          if(stream->name() == "layer"){
            // recursively populate layer with items
            stream->readNext();
            getLayer(layer_id)->loadItems(stream, scene);
            layer_id++;
          }
          else
            stream->readNext();
        }
      }
      else{
        qDebug() << QObject::tr("Design Panel: invalid element encountered on line %1 - %2").arg(stream->lineNumber()).arg(stream->name().toString());
        stream->readNext();
      }
    }
    else
      stream->readNext();
  }

  // show error if any
  if(stream->hasError()){
    qCritical() << QObject::tr("XML error: ") << stream->errorString().data();
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
  } else if (dist_ind > job->elec_dists.size() || job->elec_dists.size() == 0) {
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
      qDebug() << tr("Setting electron %1 to %2").arg(i).arg(job->elec_dists_avg[i]);
    } else if(db_dots_result[i]) {
      // show the distribution of the selected index
      if (avg_degen) {
        db_dots_result[i]->setShowElec(job->elecDistAvgDegenOfDB(dist_ind, i));
        qDebug() << tr("Setting electron %1 to %2, averaged").arg(i).arg(job->elecDistAvgDegenOfDB(dist_ind,i));
      } else {
        db_dots_result[i]->setShowElec(job->elec_dists[dist_ind].dist[i]);
      }
    }
  }
}


void gui::DesignPanel::clearSimResults()
{
  setDisplayMode(DesignMode);

  // set show_elec of all DBDots to 0
  if(!db_dots_result.isEmpty())
    for(auto *db : db_dots_result)
      db->setShowElec(0);
}

void gui::DesignPanel::displayPotentialPlot(QPixmap potential_plot, QRectF graph_container)
{
  qDebug() << tr("displayPotentialPlot");
  qDebug() << tr("graph_container height: ") << graph_container.height();
  qDebug() << tr("graph_container width: ") << graph_container.width();
  qDebug() << tr("graph_container topLeft: ") << graph_container.topLeft().x() << tr(", ") << graph_container.topLeft().y();
  createPotPlot(potential_plot, graph_container);
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
  setLatticeSiteOccupancy(item, true);
}

// INTERRUPTS

// most behaviour will be connected to mouse move/release. However, when
// ghosting (dragging items or copy/paste), show the ghost on the left button
// press
void gui::DesignPanel::mousePressEvent(QMouseEvent *e)
{
  Qt::KeyboardModifiers keymods = QApplication::keyboardModifiers();

  // set clicked flag and store current mouse position for move behaviour
  mouse_pos_old = e->pos();
  mouse_pos_cached = e->pos(); // this might be a referencing clash, check.
  press_scene_pos = mapToScene(e->pos()); // the scene position of the click event

  // if other buttons are clicked during rubber band selection, end selection
  if(rb)
    rubberBandEnd();

  clicked = true;
  switch(e->button()){
    case Qt::LeftButton:
      if (tool_type == ScreenshotAreaTool) {
        // use rubberband to select screenshot area
        rb_start = mapToScene(e->pos()).toPoint();
        rb_cache = e->pos();

      } else if (tool_type == SelectTool || tool_type == ElectrodeTool ||
          tool_type == AFMAreaTool) {
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
  // Qt::KeyboardModifiers keymods = QApplication::keyboardModifiers(); // uncomment if keymods are needed

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
    QList<prim::Item::ItemType> target_types;
    target_types.append(prim::Item::LatticeDot);
    target_types.append(prim::Item::DBDot);
    prim::Item *snap_target = filteredSnapTarget(mapToScene(e->pos()), target_types, snap_diameter);
    if (snap_target) {
      afm_panel->ghostNode()->setPos(snap_target->scenePos());
      afm_panel->showGhost(true);
    }

  } else if (!clicked && tool_type == DBGenTool) {
    // show preview location of new DB
    createDBPreviews({lattice->nearestSite(mapToScene(e->pos()))});

  } else if (clicked) {
    // not ghosting, mouse dragging of some sort
    switch(e->buttons()){
      case Qt::LeftButton:
        if (tool_type == SelectTool || tool_type == DBGenTool ||
            tool_type == ElectrodeTool || tool_type == AFMAreaTool ||
            tool_type == ScreenshotAreaTool) {
          rubberBandUpdate(e->pos());
        }
        // use default behaviour for left mouse button
        QGraphicsView::mouseMoveEvent(e);
        break;
      case Qt::MidButton:
        // middle button always pans
        mouse_pos_del = e->pos()-mouse_pos_old;
        dx = mouse_pos_del.x();
        dy = mouse_pos_del.y();
        verticalScrollBar()->setValue(verticalScrollBar()->value()-dy);
        horizontalScrollBar()->setValue(horizontalScrollBar()->value()-dx);
        mouse_pos_old = e->pos();
        break;
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

  // end rubber band if active
  if (rb)
    rubberBandEnd();

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
            createElectrodes(e->pos());
            break;
          case gui::ToolType::AFMAreaTool:
            filterSelection(false);
            createAFMArea(e->pos());
            break;
          case gui::ToolType::AFMPathTool:
            // Make node at the ghost position
            createAFMNode();
            break;
          case gui::ToolType::ScreenshotAreaTool:
            // take a screenshot of the rubberband area
            filterSelection(false);
            sig_screenshot(QRect(press_scene_pos.toPoint(), mapToScene(e->pos()).toPoint()));
            break;
          case gui::ToolType::DragTool:
            // pan ends
            break;
          case gui::ToolType::MeasureTool:{
            // display measurement from start to finish
            QPointF delta = e->pos()-mouse_pos_cached;
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
  Qt::KeyboardModifiers keymods = QApplication::keyboardModifiers();

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
      case Qt::Key_G:
        if(display_mode == DesignMode){
          // grouping behaviour for selecting surface dangling bonds
          if(keymods == (Qt::ControlModifier | Qt::ShiftModifier))
            splitAggregates();
          else if(keymods == Qt::ControlModifier)
            formAggregate();
        }
        break;
      case Qt::Key_C:
        // copy selected items to the clipboard
        copySelection();
        break;
      case Qt::Key_V:{
        // create ghost for clipboard if any
        if(!clipboard.isEmpty() && display_mode == DesignMode)
          createGhost(true);
        break;
      }
      case Qt::Key_Z:{
          if(display_mode == DesignMode){
            // undo/redo based on keymods
            //qDebug() << tr("Index before undo/redo: %1").arg(undo_stack->index());
            if(keymods == (Qt::ControlModifier | Qt::ShiftModifier))
              undo_stack->redo();
            else if(keymods == Qt::ControlModifier)
              undo_stack->undo();
            //qDebug() << tr("Index after undo/redo: %1").arg(undo_stack->index());
            //qDebug() << tr("ptr %1").arg((size_t)undo_stack->command(undo_stack->index()));
          }
        }
        break;
      case Qt::Key_Y:{
        if(keymods == Qt::ControlModifier && display_mode == DesignMode)
          undo_stack->redo();
        break;
      }
      case Qt::Key_X:{
        if(keymods == Qt::ControlModifier && display_mode == DesignMode){
          // copy current selection
          copySelection();
          // delete current selection
          deleteSelection();
        }
        break;
      }
      case Qt::Key_Backspace:
      case Qt::Key_Delete:
        // delete selected items
        if(tool_type == gui::ToolType::SelectTool && display_mode == DesignMode)
          deleteSelection();
        break;
      case Qt::Key_S:
        if (display_mode == ScreenshotMode) {
          sig_screenshot(prev_screenshot_area);
        }
        break;
      case Qt::Key_R:{
        if (keymods == (Qt::ControlModifier | Qt::ShiftModifier)) {
          QMessageBox::StandardButton reply;
          reply = QMessageBox::question(this, "Quick run simulation",
              "Are you sure you want to run a simulation with previous settings?",
              QMessageBox::Yes | QMessageBox::No);
          if (reply == QMessageBox::Yes) {
            emit sig_quickRunSimulation();
          }
        } else if (keymods == Qt::ControlModifier) {
          emit sig_showSimulationSetup();
        }
        break;
      }
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
    scale(1+ds,1+ds);   // perform the zoom
    QPointF delta = mapToScene(e->pos()) - old_pos;
    scrollDelta(delta); // scroll with anchoring
  }

  // reset both scrolls (avoid repeat from |x|>=120)
  wheel_deg.setX(0);
  wheel_deg.setY(0);

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

  verticalScrollBar()->setValue(verticalScrollBar()->value() + ((shift_scroll) ? dx : dy));
  horizontalScrollBar()->setValue(horizontalScrollBar()->value()+ ((shift_scroll) ? dy: dx));
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
  QMenu menu(this); //create the context menu object
  if (QGraphicsItem *gitem = itemAt(e->pos())) {
    qDebug() << tr("Item clicked was at: (%1 , %2)").arg(gitem->x()).arg(gitem->y());
    if (static_cast<prim::Item*>(gitem)->upSelected()) {
      //Something was selected, so determine the type and give the correct context menu.
      if (static_cast<prim::Item*>(gitem)->item_type == prim::Item::Electrode) {
        menu.addAction(action_set_potential);
        menu.addSeparator();
      } else if (static_cast<prim::Item*>(gitem)->item_type == prim::Item::DBDot) {
        menu.addAction(action_toggle_db_elec);
        menu.addSeparator();
      }
    }
  } else {
  }
  menu.addAction(action_undo);
  menu.addAction(action_redo);
  menu.addSeparator();
  menu.addAction(action_cut);
  menu.addAction(action_copy);
  menu.addAction(action_paste);
  menu.addSeparator();
  menu.addAction(action_delete);
  menu.exec(e->globalPos());
}

void gui::DesignPanel::undoAction()
{
    // qDebug() << tr("Undo...");
    undo_stack->undo();
}

void gui::DesignPanel::redoAction()
{
    // qDebug() << tr("Redo...");
    undo_stack->redo();
}

void gui::DesignPanel::cutAction()
{
    // qDebug() << tr("Cut...");
    copySelection();
    deleteSelection();
}

void gui::DesignPanel::copyAction()
{
    // qDebug() << tr("Copy...");
    copySelection();
}

void gui::DesignPanel::pasteAction()
{
    // qDebug() << tr("Paste...");
    if(!clipboard.isEmpty() && display_mode == DesignMode)
      createGhost(true);
}

void gui::DesignPanel::deleteAction()
{
  if(tool_type == gui::ToolType::SelectTool && display_mode == DesignMode)
    deleteSelection();
}

void gui::DesignPanel::electrodeSetPotentialAction()
{
  // setting potential is not an un-doable action for now.
  bool ok = false;
  double potential;
  // qDebug() << tr("electrodeSetPotential...");
  QList<prim::Item*> selection = selectedItems();
  if(selection.isEmpty()){ //TODO:figure out how we want to handle right click without selection
    qDebug() << tr("Please select an item...");
  } else {
    if(selection.count() == 1){
      potential = QInputDialog::getDouble(this, tr("Set Potential"),
                  tr("Set electrode potential(s) to:"), static_cast<prim::Electrode*>(selection.at(0))->getPotential(),
                  -2147483647, 2147483647, 3, &ok);
    } else {
      potential = QInputDialog::getDouble(this, tr("Set Potential"),
                  tr("WARNING: Multiple electrodes selected.\nSet electrode potential(s) to:"), 0.0,
                  -2147483647, 2147483647, 3, &ok);
    }
    if(ok){
      for(prim::Item *item : selection){
        if(item->item_type == prim::Item::Electrode)
          static_cast<prim::Electrode*>(item)->setPotential(potential);
      }
    }
  }
}


void gui::DesignPanel::createActions()
{
  action_undo = new QAction(tr("&Undo"), this);
  connect(action_undo, &QAction::triggered, this, &gui::DesignPanel::undoAction);

  action_redo = new QAction(tr("&Redo"), this);
  connect(action_redo, &QAction::triggered, this, &gui::DesignPanel::redoAction);

  action_cut = new QAction(tr("Cut"), this);
  connect(action_cut, &QAction::triggered, this, &gui::DesignPanel::cutAction);

  action_copy = new QAction(tr("&Copy"), this);
  connect(action_copy, &QAction::triggered, this, &gui::DesignPanel::copyAction);

  action_paste = new QAction(tr("&Paste"), this);
  connect(action_paste, &QAction::triggered, this, &gui::DesignPanel::pasteAction);

  action_delete = new QAction(tr("&Delete"), this);
  connect(action_delete, &QAction::triggered, this, &gui::DesignPanel::deleteAction);

  action_set_potential = new QAction(tr("&Set Potential"), this);
  connect(action_set_potential, &QAction::triggered, this, &gui::DesignPanel::electrodeSetPotentialAction);
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
    QRect rb_rect_scene;
    rb_rect_scene.setTopLeft(mapFromScene(rb_start));
    rb_rect_scene.setBottomRight(pos);
    rb_rect_scene = rb_rect_scene.normalized();

    // deselect items that are no longer contained
    QList<QGraphicsItem*> selected_items = scene->selectedItems();
    for(QGraphicsItem* selected_item : selected_items)
      selected_item->setSelected(false);

    // select the new items
    QList<QGraphicsItem*> rb_items = scene->items(QRect(rb_start,mapToScene(pos).toPoint()).normalized());
    for(QGraphicsItem* rb_item : rb_items)
      rb_item->setSelected(true);

    // select shift list items
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


void gui::DesignPanel::createGhost(bool paste)
{
  // qDebug() << tr("Creating ghost...");
  prim::Ghost *ghost = prim::Ghost::instance();
  pasting=paste;
  ghosting=true;
  snap_cache = QPointF();

  if (paste) {
    ghost->prepare(clipboard);
    prim::LatticeCoord offset;
    if (snapGhost(mapToScene(mapFromGlobal(QCursor::pos())), offset))
      ghost->moveByCoord(offset, lattice);
  } else {
    QPointF scene_pos = mapToScene(mapFromGlobal(QCursor::pos()));
    //get QList of selected Item object
    filterSelection(true);
    ghost->prepare(selectedItems(), scene_pos);
  }
}


void gui::DesignPanel::clearGhost()
{
  ///qDebug() << tr("Clearing ghost...");
  prim::Ghost::instance()->cleanGhost();
  ghosting=false;
  snap_target=prim::LatticeCoord();
}


bool gui::DesignPanel::snapGhost(QPointF scene_pos, prim::LatticeCoord &offset)
{
  bool is_all_floating = true;

  // check if holding any non-floating objects
  for (prim::Item *item : pasting ? clipboard : selectedItems()) {
    if (item->item_type != prim::Item::Electrode &&
        item->item_type != prim::Item::AFMArea) {
      is_all_floating = false;
      break;
    }
  }

  if (is_all_floating) {
    prim::Ghost *ghost = prim::Ghost::instance();
    if (pasting) { //offset is in the first electrode item
      ghost->moveTo(mapToScene(mapFromGlobal(QCursor::pos()))
          - clipboard[0]->pos()
          - QPointF(static_cast<prim::Electrode*>(clipboard[0])->getWidth()/2.0,
              static_cast<prim::Electrode*>(clipboard[0])->getHeight()/2.0)
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
    snap_coord = nearest_site;
    if (!ghost->valid_hash.contains(nearest_site))
      ghost->valid_hash[nearest_site] = ghost->checkValid(offset, lattice);
    snap_target = nearest_site;
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
    case prim::Item::DBDot:
      if (flag)
        lattice->setOccupied(static_cast<prim::DBDot*>(item)->latticeCoord(), item);
      else
        lattice->setUnoccupied(static_cast<prim::DBDot*>(item)->latticeCoord());
      break;
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
  db_at_loc = static_cast<prim::DBDot*>(dp->lattice->dbAt(l_coord));

  if (invert && !db_at_loc)
    qFatal("Trying to remove a non-existing DB");
  else if (!invert && db_at_loc)
    qFatal("Trying to make a new DB at a location that already has one");

  // dbdot index in layer
  prim::Layer *layer = dp->getLayer(layer_index);
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
  if (db_at_loc) {
    dp->lattice->setUnoccupied(db_at_loc->latticeCoord());
    dp->removeItem(db_at_loc, dp->getLayer(db_at_loc->layer_id));
    db_at_loc = 0;
  }
}


// CreateElectrode class

gui::DesignPanel::CreateElectrode::CreateElectrode(int layer_index, gui::DesignPanel *dp, QPointF point1, QPointF point2, prim::Electrode *elec, bool invert, QUndoCommand *parent)
  : QUndoCommand(parent), dp(dp), layer_index(layer_index), point1(point1), point2(point2), invert(invert)
{  //if called to destroy, *elec points to selected electrode. if called to create, *elec = 0
  prim::Layer *layer = dp->getLayer(layer_index);
  index = invert ? layer->getItems().indexOf(elec) : layer->getItems().size();
}

void gui::DesignPanel::CreateElectrode::undo()
{
  invert ? create() : destroy();
}

void gui::DesignPanel::CreateElectrode::redo()
{
  invert ? destroy() : create();
}


void gui::DesignPanel::CreateElectrode::create()
{
  dp->addItem(new prim::Electrode(layer_index, point1, point2), layer_index, index);
}

void gui::DesignPanel::CreateElectrode::destroy()
{
  prim::Electrode *electrode = static_cast<prim::Electrode*>(dp->getLayer(layer_index)->getItem(index));
  if(electrode != 0){
    // destroy electrode
    dp->removeItem(electrode, dp->getLayer(electrode->layer_id));  // deletes electrode
    electrode = 0;
  }
}


// CreatePotPlot class
gui::DesignPanel::CreatePotPlot::CreatePotPlot(int layer_index, gui::DesignPanel *dp, QPixmap potential_plot, QRectF graph_container, prim::PotPlot *pp, bool invert, QUndoCommand *parent)
  : QUndoCommand(parent), dp(dp), layer_index(layer_index), potential_plot(potential_plot), graph_container(graph_container), invert(invert)
{  //if called to destroy, *elec points to selected electrode. if called to create, *elec = 0
  prim::Layer *layer = dp->getLayer(layer_index);
  index = invert ? layer->getItems().indexOf(pp) : layer->getItems().size();
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
  dp->addItem(new prim::PotPlot(layer_index, potential_plot, graph_container), layer_index, index);
}

void gui::DesignPanel::CreatePotPlot::destroy()
{
  prim::PotPlot *pp = static_cast<prim::PotPlot*>(dp->getLayer(layer_index)->getItem(index));
  if(pp != 0){
    dp->removeItem(pp, dp->getLayer(pp->layer_id));  // deletes PotPlot
    pp = 0;
  }
}


// CreateAFMArea class

gui::DesignPanel::CreateAFMArea::CreateAFMArea(int layer_index,
    gui::DesignPanel *dp, QPointF point1, QPointF point2,
    prim::AFMArea *afm_area, bool invert, QUndoCommand *parent)
  : QUndoCommand(parent), dp(dp), layer_index(layer_index),
    point1(point1), point2(point2), invert(invert)
{
  prim::Layer *layer = dp->getLayer(layer_index);
  index = invert ? layer->getItems().indexOf(afm_area) : layer->getItems().size();
}

void gui::DesignPanel::CreateAFMArea::undo()
{
  invert ? create() : destroy();
}

void gui::DesignPanel::CreateAFMArea::redo()
{
  invert ? destroy() : create();
}

void gui::DesignPanel::CreateAFMArea::create()
{
  dp->addItem(new prim::AFMArea(layer_index, point1, point2), layer_index, index);
}

void gui::DesignPanel::CreateAFMArea::destroy()
{
  prim::AFMArea *afmarea = static_cast<prim::AFMArea*>(
      dp->getLayer(layer_index)->getItem(index));
  if (afmarea != 0) {
    // destroy AFMArea
    dp->removeItem(afmarea, dp->getLayer(afmarea->layer_id));
  }
}


// CreateAFMPath class

gui::DesignPanel::CreateAFMPath::CreateAFMPath(int layer_index, gui::DesignPanel *dp,
                        prim::AFMPath *afm_path, bool invert, QUndoCommand *parent)
  : QUndoCommand(parent), invert(invert), dp(dp), layer_index(layer_index)
{
  //qDebug() << tr("Entered CreateAFMPath");
  prim::Layer *layer = dp->getLayer(layer_index);
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
  prim::AFMPath *afm_path = static_cast<prim::AFMPath*>(dp->getLayer(layer_index)->getItem(index));

  // contained nodes and segments should be deleted automatically when deleting the path
  // since they're children items to the path.
  dp->removeItem(afm_path, dp->getLayer(afm_path->layer_id));
  dp->afmPanel()->setFocusedPath(0);
}


// CreateAFMNode class

gui::DesignPanel::CreateAFMNode::CreateAFMNode(int layer_index, gui::DesignPanel *dp,
                        QPointF scenepos, float z_offset, int afm_index,
                        int index_in_path, bool invert, QUndoCommand *parent)
  : QUndoCommand(parent), invert(invert), layer_index(layer_index), dp(dp),
      afm_index(afm_index), scenepos(scenepos), z_offset(z_offset)
{
  prim::AFMPath *afm_path = static_cast<prim::AFMPath*>(dp->getLayer(layer_index)->getItem(afm_index));
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
  prim::AFMPath *afm_path = static_cast<prim::AFMPath*>(dp->getLayer(layer_index)->getItem(afm_index));
  prim::AFMNode *new_node = new prim::AFMNode(layer_index, scenepos, z_offset);
  afm_path->insertNode(new_node, node_index);

  dp->afmPanel()->setFocusedPath(afm_path);
  dp->afmPanel()->setFocusedNodeIndex(node_index);
}

void gui::DesignPanel::CreateAFMNode::destroy()
{
  //qDebug() << tr("Entered CreateAFMNode::destroy()");
  prim::AFMPath *afm_path = static_cast<prim::AFMPath*>(dp->getLayer(layer_index)->getItem(afm_index));
  afm_path->removeNode(node_index); // pointer cleanup is done by AFMPath
  dp->afmPanel()->setFocusedNodeIndex(node_index-1);
}


// ResizeAFMArea class
gui::DesignPanel::ResizeAFMArea::ResizeAFMArea(int layer_index, DesignPanel *dp,
    const QRectF &orig_rect, const QRectF &new_rect, int afm_area_index,
    bool invert, QUndoCommand *parent)
  : QUndoCommand(parent), invert(invert), layer_index(layer_index), dp(dp),
        afm_area_index(afm_area_index), orig_rect(orig_rect), new_rect(new_rect)
{
  top_left_delta = new_rect.topLeft() - orig_rect.topLeft();
  bot_right_delta = new_rect.bottomRight() - orig_rect.bottomRight();
}

void gui::DesignPanel::ResizeAFMArea::undo()
{
  prim::AFMArea *afm_area = static_cast<prim::AFMArea*>(
      dp->getLayer(layer_index)->getItem(afm_area_index));

  if (afm_area->boundingRect().topLeft() == orig_rect.topLeft() &&
      afm_area->boundingRect().bottomRight() == orig_rect.bottomRight())
    return;

  afm_area->resize(-top_left_delta.x(), -top_left_delta.y(),
      -bot_right_delta.x(), -bot_right_delta.y(), true);
}

void gui::DesignPanel::ResizeAFMArea::redo()
{
  prim::AFMArea *afm_area = static_cast<prim::AFMArea*>(
      dp->getLayer(layer_index)->getItem(afm_area_index));

  // if the user resized the afm area with the cursor, then the area might
  // already be the right size, in which case do nothing
  if (afm_area->boundingRect().topLeft() == new_rect.topLeft() &&
      afm_area->boundingRect().bottomRight() == new_rect.bottomRight())
    return;

  afm_area->resize(top_left_delta.x(), top_left_delta.y(),
      bot_right_delta.x(), bot_right_delta.y(), true);
}


// ResizeElectrode class
gui::DesignPanel::ResizeElectrode::ResizeElectrode(int layer_index, DesignPanel *dp,
    const QRectF &orig_rect, const QRectF &new_rect, int electrode_index,
    bool invert, QUndoCommand *parent)
  : QUndoCommand(parent), invert(invert), layer_index(layer_index), dp(dp),
        electrode_index(electrode_index), orig_rect(orig_rect), new_rect(new_rect)
{
  top_left_delta = new_rect.topLeft() - orig_rect.topLeft();
  bot_right_delta = new_rect.bottomRight() - orig_rect.bottomRight();
}

void gui::DesignPanel::ResizeElectrode::undo()
{
  prim::Electrode *electrode = static_cast<prim::Electrode*>(
      dp->getLayer(layer_index)->getItem(electrode_index));

  if (electrode->boundingRect().topLeft() == orig_rect.topLeft() &&
      electrode->boundingRect().bottomRight() == orig_rect.bottomRight())
    return;

  electrode->resize(-top_left_delta.x(), -top_left_delta.y(),
      -bot_right_delta.x(), -bot_right_delta.y(), true);
}

void gui::DesignPanel::ResizeElectrode::redo()
{
  prim::Electrode *electrode = static_cast<prim::Electrode*>(
      dp->getLayer(layer_index)->getItem(electrode_index));

  // if the user resized the afm area with the cursor, then the area might
  // already be the right size, in which case do nothing
  if (electrode->boundingRect().topLeft() == new_rect.topLeft() &&
      electrode->boundingRect().bottomRight() == new_rect.bottomRight())
    return;

  electrode->resize(top_left_delta.x(), top_left_delta.y(),
      bot_right_delta.x(), bot_right_delta.y(), true);
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
  prim::Layer *layer = dp->getLayer(layer_index);
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
  prim::Layer *layer = dp->getLayer(layer_index);

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
  prim::Layer *layer = dp->getLayer(layer_index);

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
  prim::Layer *layer = dp->getLayer(layer_index);
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
  item_index = dp->getLayer(layer_index)->getItems().indexOf(item);
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
  prim::Layer *layer = dp->getLayer(layer_index);
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
    case prim::Item::Electrode:
      moveElectrode(static_cast<prim::Electrode*>(item), delta);
      break;
    case prim::Item::AFMArea:
      moveAFMArea(static_cast<prim::AFMArea*>(item), delta);
      break;
    default:
      item->moveBy(delta.x(), delta.y());
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

void gui::DesignPanel::MoveItem::moveElectrode(prim::Electrode *electrode, const QPointF &delta)
{
  electrode->setPos( electrode->pos() + delta );
  electrode->updatePoints(delta);
}

void gui::DesignPanel::MoveItem::moveAFMArea(prim::AFMArea *afm_area,
    const QPointF &delta)
{
  afm_area->setPos(afm_area->pos() + delta);
  afm_area->updatePoints(delta);
}

// Undo/Redo Methods

void gui::DesignPanel::createDBs()
{
  // create DBs at preview DB locations
  int layer_index = layers.indexOf(top_layer);
  undo_stack->beginMacro(tr("create dangling bonds at DB preview locations"));
  for (prim::DBDotPreview *db_prev : db_previews)
    undo_stack->push(new CreateDB(db_prev->latticeCoord(), layer_index, this));
  undo_stack->endMacro();

  destroyDBPreviews();
}

void gui::DesignPanel::createElectrodes(QPoint point1)
{
  QPoint point2 = mapToScene(mouse_pos_cached).toPoint(); //get coordinates relative to top-left
  int layer_index = layers.indexOf(electrode_layer);
  //only ever create one electrode at a time
  undo_stack->beginMacro(tr("create electrode with given corners"));
  undo_stack->push(new CreateElectrode(layer_index, this, mapToScene(point1).toPoint(), point2));
  undo_stack->endMacro();
}

void gui::DesignPanel::createPotPlot(QPixmap potential_plot, QRectF graph_container)
{
  int layer_index = layers.indexOf(plot_layer);
  undo_stack->beginMacro(tr("create potential plot with given corners"));
  undo_stack->push(new CreatePotPlot(layer_index, this, potential_plot, graph_container));
  undo_stack->endMacro();
}

void gui::DesignPanel::createAFMArea(QPoint point1)
{
  point1 = mapToScene(point1).toPoint();
  QPoint point2 = mapToScene(mouse_pos_cached).toPoint();
  int layer_index = getLayerIndex(afm_layer);

  // create the AFM area
  undo_stack->beginMacro(tr("create AFM area with given corners"));
  undo_stack->push(new CreateAFMArea(layer_index, this, point1, point2));
  undo_stack->endMacro();
}

void gui::DesignPanel::createAFMNode()
{
  //qDebug() << tr("Entered createAFMNode()");
  int layer_index = getLayerIndex(afm_layer);
  QPointF scene_pos;
  if (afm_panel->ghostNode())
    scene_pos = afm_panel->ghostNode()->scenePos();
  else
    scene_pos = mapToScene(mouse_pos_cached);

  // TODO UNDOable version
  undo_stack->beginMacro(tr("create AFMNode in the focused AFMPath after the focused AFMNode"));
  //qDebug() << tr("AFMNode creation macro began");
  if (!afm_panel->focusedPath()) {
    //qDebug() << tr("No existing focused path, making new");
    // create new path if there's no focused path
    prim::AFMPath *new_path = new prim::AFMPath(layer_index);
    afm_panel->setFocusedPath(new_path);
    undo_stack->push(new CreateAFMPath(layer_index, this));
  }
  int afm_path_index = afm_layer->getItemIndex(afm_panel->focusedPath());
  //qDebug() << tr("About to push new AFMNode to undo stack, afm_path_index=%1").arg(afm_path_index);
  undo_stack->push(new CreateAFMNode(layer_index, this, scene_pos, afm_layer->zOffset(),
                                        afm_path_index));
  undo_stack->endMacro();
}

void gui::DesignPanel::resizeItem(prim::Item *item,
    const QRectF &orig_rect, const QRectF &new_rect)
{
  resizing = false;
  switch (item->item_type) {
    case prim::Item::AFMArea:
      resizeAFMArea(static_cast<prim::AFMArea*>(item), orig_rect, new_rect);
      break;
    case prim::Item::Electrode:
      resizeElectrode(static_cast<prim::Electrode*>(item), orig_rect, new_rect);
      break;
    default:
      break;
  }
}

void gui::DesignPanel::resizeAFMArea(prim::AFMArea *afm_area,
    const QRectF &orig_rect, const QRectF &new_rect)
{
  undo_stack->beginMacro(tr("Resize AFM Area"));
  int ind_in_layer = getLayer(afm_area->layer_id)->getItemIndex(afm_area);
  undo_stack->push(new ResizeAFMArea(afm_area->layer_id, this, orig_rect,
      new_rect, ind_in_layer));
  undo_stack->endMacro();
}

void gui::DesignPanel::resizeElectrode(prim::Electrode *electrode,
    const QRectF &orig_rect, const QRectF &new_rect)
{
  undo_stack->beginMacro(tr("Resize Electrode"));
  int ind_in_layer = getLayer(electrode->layer_id)->getItemIndex(electrode);
  undo_stack->push(new ResizeElectrode(electrode->layer_id, this, orig_rect,
      new_rect, ind_in_layer));
  undo_stack->endMacro();
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
    int afm_index = getLayer(afm_node->layer_id)->getItemIndex(afm_path);
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
      case prim::Item::Electrode:
        undo_stack->push(new CreateElectrode( item->layer_id, this, QPointF(item->x(), item->y()),
                        QPointF(item->x() + static_cast<prim::Electrode*>(item)->getWidth(),
                        item->y() + static_cast<prim::Electrode*>(item)->getHeight()),
                        static_cast<prim::Electrode*>(item), true));
        break;
      case prim::Item::AFMArea:
        {
        prim::AFMArea *afm_area = static_cast<prim::AFMArea*>(item);
        undo_stack->push(new CreateAFMArea(afm_area->layer_id, this,
            afm_area->topLeft(), afm_area->bottomRight(), afm_area, true));
        break;
        }
      case prim::Item::AFMPath:
        destroyAFMPath(static_cast<prim::AFMPath*>(item));
        break;
      case prim::Item::AFMNode:
        {
        prim::AFMNode *afm_node = static_cast<prim::AFMNode*>(item);
        prim::AFMPath *afm_path = static_cast<prim::AFMPath*>(afm_node->parentItem());
        int afm_index = getLayer(afm_node->layer_id)->getItemIndex(afm_path);
        int node_index = afm_path->getNodeIndex(afm_node);
        undo_stack->push(new CreateAFMNode(afm_node->layer_id, this,
            afm_node->scenePos(), afm_node->zOffset(), afm_index, node_index, true));
        qDebug() << "shouldn't be here yet";
        break;
        }
      case prim::Item::PotPlot:
        {
        prim::PotPlot *pp = static_cast<prim::PotPlot*>(item);
        undo_stack->push(new CreatePotPlot( pp->layer_id, this,
            pp->getPotentialPlot(), pp->getGraphContainer(),
            static_cast<prim::PotPlot*>(item), true));
        break;
        }
      default:
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
        static_cast<prim::Item*>(gitem)->item_type != prim::Item::AFMArea){
      is_all_floating = false;
      break;
    }
  }
  // do nothing if clipboard empty
  if (clipboard.isEmpty()) {
    return false;
  } else {
    if (!is_all_floating && !ghost->valid_hash[snap_target]) {
      return false;
    }
  }
  undo_stack->beginMacro(tr("Paste %1 items").arg(clipboard.count()));
  // paste each item in the clipboard, same as ghost top items (preferred order)
  for(prim::Item *item : ghost->getTopItems())
    pasteItem(ghost, item);

  undo_stack->endMacro();
  pasting=false;
  return true;
}

void gui::DesignPanel::pasteItem(prim::Ghost *ghost, prim::Item *item)
{
  switch(item->item_type){
    case prim::Item::DBDot:
      pasteDBDot(ghost, static_cast<prim::DBDot*>(item));
      break;
    case prim::Item::Aggregate:
      pasteAggregate(ghost, static_cast<prim::Aggregate*>(item));
      break;
    case prim::Item::Electrode:
      pasteElectrode(ghost, static_cast<prim::Electrode*>(item));
      break;
    case prim::Item::AFMArea:
      pasteAFMArea(ghost, static_cast<prim::AFMArea*>(item));
      break;
    default:
      qCritical() << tr("No functionality for pasting given item... update pasteItem");
      break;
  }
}

void gui::DesignPanel::pasteDBDot(prim::Ghost *ghost, prim::DBDot *db)
{
  // get the target lattice dor
  qDebug() << "shoud paste DB now";
  prim::LatticeCoord l_coord = ghost->getLatticeCoord(db);
  undo_stack->push(new CreateDB(l_coord, getLayerIndex(top_layer), this, db));
}

void gui::DesignPanel::pasteAggregate(prim::Ghost *ghost, prim::Aggregate *agg)
{
  undo_stack->beginMacro("Paste an aggregate");

  // paste all the children items
  QList<prim::Item*> items;
  for(prim::Item *item : agg->getChildren()){
    pasteItem(ghost, item);
    // new item will be at the top of the Layer Item stack
    items.append(top_layer->getItems().top());
  }

  // form Aggregate from Items
  undo_stack->push(new FormAggregate(items, this));

  undo_stack->endMacro();

}

void gui::DesignPanel::pasteElectrode(prim::Ghost *ghost, prim::Electrode *elec)
{
  undo_stack->beginMacro(tr("create electrode with given corners"));
  undo_stack->push(new CreateElectrode(elec->layer_id, this, ghost->pos()+elec->pos(),
              ghost->pos()+elec->pos()+QPointF(elec->getWidth(), elec->getHeight())));
  undo_stack->endMacro();
}

void gui::DesignPanel::pasteAFMArea(prim::Ghost *ghost, prim::AFMArea *afm_area)
{
  undo_stack->beginMacro(tr("create AFMArea with given afm_area params"));
  // TODO copy AFM tip attributes
  undo_stack->push(new CreateAFMArea(afm_area->layer_id, this,
      ghost->pos()+afm_area->topLeft(), ghost->pos()+afm_area->bottomRight()));
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
  QPointF offset = (!kill && ghost->valid_hash[snap_target]) ? ghost->moveOffset() : QPointF();

  if (offset.isNull()) {
    // There is no offset for dbs. Check if selection is all electrodes, and if it is, move them.
    // reset the original lattice dot selectability and return false
    for (prim::Item *item : selectedItems()) {
      if (item->item_type != prim::Item::Electrode &&
          item->item_type != prim::Item::AFMArea) {
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
