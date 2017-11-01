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

// constructor
gui::DesignPanel::DesignPanel(QWidget *parent)
  : QGraphicsView(parent)
{
  //rotate(-90);

  // set-up scale_factor in prim::Item
  prim::Item::init();

  undo_stack = new QUndoStack();

  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  settings::AppSettings *app_settings = settings::AppSettings::instance();

  scene = new QGraphicsScene(this);
  setScene(scene);
  setMouseTracking(true);

  connect(prim::Emitter::instance(), &prim::Emitter::sig_selectClicked,
            this, &gui::DesignPanel::selectClicked);

  // setup flags
  clicked = ghosting = moving = false;

  // initialising parameters
  snap_diameter = app_settings->get<qreal>("snap/diameter")*prim::Item::scale_factor;
  qDebug() << tr("SD: %1").arg(snap_diameter);
  snap_target = 0;

  tool_type = gui::DesignPanel::NoneTool;     // now setTool will update the tool

  // set view behaviour
  setTransformationAnchor(QGraphicsView::NoAnchor);
  setResizeAnchor(QGraphicsView::AnchorViewCenter);

  setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
            QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);

  // color scheme
  QColor col;
  setBackgroundBrush(QBrush(gui_settings->get<QColor>("view/bg_col")));
  setFrameShadow(QFrame::Raised);

  setCacheMode(QGraphicsView::CacheBackground);

  // make lattice and surface layer
  buildLattice();
  setScenePadding();

  // surface layer active on init
  top_layer = layers.at(1);

  // initialise the Ghost and set the scene
  prim::Ghost::instance()->setScene(scene);

  // set scroll to top left
  QScrollBar *vsb = verticalScrollBar();
  QScrollBar *hsb = horizontalScrollBar();
  vsb->setValue(vsb->minimum());
  hsb->setValue(hsb->minimum());

  // set display mode
  setDisplayMode(DesignMode);
}

// destructor
gui::DesignPanel::~DesignPanel()
{
  clearDesignPanel(false);
}

// clear design panel
void gui::DesignPanel::clearDesignPanel(bool reset)
{
  // delete all graphical items from the scene
  scene->clear();
  if(!reset) delete scene;

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

  // REBUILD
  // reset flags
  clicked = ghosting = moving = false;
  tool_type = gui::DesignPanel::NoneTool;     // now setTool will update the tool

  undo_stack = new QUndoStack();
  // TODO reset undo stack counter

  buildLattice();
  top_layer = layers.at(1);

  prim::Ghost::instance()->setScene(scene);

  resetMatrix(); // resets QTransform, which undoes the zoom

  // rotate scene and set scroll to top left
  //rotate(-90);
  QScrollBar *vsb = verticalScrollBar();
  QScrollBar *hsb = horizontalScrollBar();
  vsb->setValue(vsb->minimum());
  hsb->setValue(hsb->minimum());

  // set display mode
  setDisplayMode(DesignMode);

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




void gui::DesignPanel::addLayer(const QString &name, const QString &cnt_type)
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
  prim::Layer *layer = new prim::Layer(name, cnt_type, layers.size());
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
  prim::Lattice *lattice = new prim::Lattice(fname, layers.size());

  // add the lattice dots to the scene
  for(prim::Item *const item : lattice->getItems())
    scene->addItem(item);

  // add the lattice to the layers, as layer 0
  layers.append(lattice);

  // add in the dangling bond surface
  addLayer(tr("Surface"),tr("db"));
  top_layer = layers.at(1);
}

void gui::DesignPanel::setScenePadding(){
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  // resize the scene with padding
  QRectF rect = scene->sceneRect();
  qreal pad = qMin(rect.width(), rect.height())*gui_settings->get<qreal>("view/padding");
  rect.adjust(-.5*pad, -.5*pad, pad, pad);
  scene->setSceneRect(rect);
}


void gui::DesignPanel::setTool(gui::DesignPanel::ToolType tool)
{
  // do nothing if tool has not been changed
  if(tool==tool_type)
    return;

  // reset selected items
  scene->clearSelection();

  // inform all items of select mode
  prim::Item::select_mode = tool==gui::DesignPanel::SelectTool;

  switch(tool){
    case gui::DesignPanel::SelectTool:
      // replaced with custom rubberBandUpdate, delete this later
      setDragMode(QGraphicsView::NoDrag);
      setInteractive(true);
      break;
    case gui::DesignPanel::DragTool:
      setDragMode(QGraphicsView::ScrollHandDrag);
      setInteractive(false);
      break;
    case gui::DesignPanel::DBGenTool:
      // replaced with custom rubberBandUpdate, delete this later
      setDragMode(QGraphicsView::NoDrag);
      setInteractive(true);
      break;
    default:
      qCritical() << tr("Invalid ToolType... should not have happened");
      return;
  }

  tool_type = tool;
}

void gui::DesignPanel::setFills(float *fills)
{
  QList<prim::DBDot *> dbs = getSurfaceDBs();
  for(int i=0; i<dbs.count(); i++){
    if(qAbs(fills[i])>1.)
      qWarning() << tr("Given fill invalid");
    else
      dbs.at(i)->setFill(fills[i]);
  }
}


void gui::DesignPanel::setDisplayMode(DisplayMode mode)
{
  display_mode = mode;

  for(prim::Layer* layer : layers)
    for(prim::Item* item : layer->getItems())
      item->setDesignMode(mode == DesignMode);
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
            qreal zoom=1, scroll_v=0, scroll_h=0;
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
        // keep reading until end of layer_prop tag
        while(stream->name() != "layer_prop"){
          //bool visible_ld, active_ld;

          if(stream->isStartElement()){
            if(stream->name() == "name"){
              layer_nm = stream->readElementText();
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

          // make layer object using loaded information
          /* haven't fully implemented
          addLayer(layer_nm);
          getLayer(layer_id)->setVisible(visible_ld);
          getLayer(layer_id)->setActive(active_ld);
          layer_id++;*/
          Q_UNUSED(layer_visible);  // for now, suppress warning
          Q_UNUSED(layer_active);   // ditto
        }
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
void gui::DesignPanel::displaySimResults(prim::SimJob *job, int dist_ind)
{
  // TODO in the future, show results in a pop up windows instead of the result screen itself
  setDisplayMode(SimDisplayMode);

  if(!job){
    qDebug() << tr("DisplayPanel: Job pointer invalid");
    return;
  }
  // TODO perform this check in job's accessor rather than here
  else if(dist_ind < 0 || dist_ind > job->elec_dists.size()){
    qDebug() << tr("DesignPanel: dist_ind out of range when attempting to display sim results: %1").arg(dist_ind);
    return;
  }

  // grab a list of DBDots in the order of job->physlocs
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
      qDebug() << tr("DesignPanel: unable to show result, no DBDot is present at location x=%1, y=%2").arg(scene_loc.x()).arg(scene_loc.y());
      return;
    }
  }
  
  // set their show_elec to the set specified by job->elec_dists
  for(int i=0; i<db_dots_result.size(); i++){
    if(db_dots_result[i])
      db_dots_result[i]->setShowElec(job->elec_dists[dist_ind][i]);
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



// SLOTS

void gui::DesignPanel::selectClicked(prim::Item *)
{
  // for now, if an item is clicked in selection mode, act as though the highest
  // level aggregate was clicked
  if(tool_type == gui::DesignPanel::SelectTool && display_mode == DesignMode)
    initMove();

}

void gui::DesignPanel::simDockVisibilityChanged(bool visible)
{
  if(!visible)
    clearSimResults();
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

  // if other buttons are clicked during rubber band selection, end selection
  if(rb)
    rubberBandEnd();

  clicked = true;
  switch(e->button()){
    case Qt::LeftButton:
      if(tool_type == SelectTool || tool_type == DBGenTool){
        // rubber band variables
        rb_start = mapToScene(e->pos()).toPoint();
        rb_cache = e->pos();

        // save current selection if Shift is pressed
        if(keymods & Qt::ShiftModifier)
          rb_shift_selected = scene->selectedItems();
        else
          QGraphicsView::mousePressEvent(e);
      }
      else{
        QGraphicsView::mousePressEvent(e);
      }

      break;
    case Qt::MiddleButton:
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
  //QTransform trans = transform();
  QScrollBar *vsb = verticalScrollBar();
  QScrollBar *hsb = horizontalScrollBar();
  qreal dx, dy;

  if(ghosting){
    // update snap
    QPointF scene_pos = mapToScene(e->pos());
    QPointF offset;
    if(snapGhost(scene_pos, offset))
      prim::Ghost::instance()->moveBy(offset.x(), offset.y());
  }
  /* TODO DB ghosting when DBGen tool is in use
    else if(tool_type == gui::DesignPanel::DBGenTool){
     show "ghost" of new DB
    QPointF scene_pos = mapToScene(e->pos());
    snapDB(scene_pos);
  }*/
  else if(clicked){
    // not ghosting, mouse dragging of some sort
    switch(e->buttons()){
      case Qt::LeftButton:
        if(clicked && (tool_type == SelectTool || tool_type == DBGenTool))
          rubberBandUpdate(e->pos());

        // use default behaviour for left mouse button
        QGraphicsView::mouseMoveEvent(e);
        break;
      case Qt::MidButton:
        // middle button always pans
        mouse_pos_del = e->pos()-mouse_pos_old;
        dx = mouse_pos_del.x();
        dy = mouse_pos_del.y();
        vsb->setValue(vsb->value()-dy);
        hsb->setValue(hsb->value()-dx);
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


//
//
//
void gui::DesignPanel::mouseReleaseEvent(QMouseEvent *e)
{
  // for now, default behaviour first
  QGraphicsView::mouseReleaseEvent(e);
  // QPointF scene_pos = mapToScene(e->pos());
  QTransform trans = transform();

  // end rubber band if active
  if(rb)
    rubberBandEnd();

  // case specific behaviour
  if(ghosting){
    // plant ghost and end ghosting
    if(moving)
      moveToGhost();
    else
      pasteAtGhost();
    clearGhost();
  }
  else if(clicked){
    switch(e->button()){
      case Qt::LeftButton:
        // action based on chosen tool
        switch(tool_type){
          case gui::DesignPanel::SelectTool:
            // filter out items in the lattice
            filterSelection(true);
            break;
          case gui::DesignPanel::DBGenTool:
            // identify free lattice sites and create dangling bonds
            filterSelection(false);
            createDBs();
            break;
          case gui::DesignPanel::DragTool:
            // pan ends
            break;
          case gui::DesignPanel::MeasureTool:{
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
  else if(~deg_del.isNull())
    wheel_deg += deg_del;

  // if enough scroll achieved, act and reset wheel_deg
  if(qMax(qAbs(wheel_deg.x()),qAbs(wheel_deg.y())) >= 120) {
    Qt::KeyboardModifiers keymods = QApplication::keyboardModifiers();
    if(keymods & Qt::ControlModifier)
      wheelZoom(e, keymods & Qt::ShiftModifier);
    else
      wheelPan(keymods & Qt::ShiftModifier);
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
        // deactivate current tool
        if(tool_type != gui::DesignPanel::SelectTool){
          //qDebug() << tr("Esc pressed, drop back to select tool");
          // emit signal to be picked up by application.cc
          emit sig_toolChange(gui::DesignPanel::SelectTool);
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
        if(tool_type == gui::DesignPanel::SelectTool && display_mode == DesignMode)
          deleteSelection();
        break;
      case Qt::Key_S:
        if(keymods == Qt::ControlModifier){
          //gui::ApplicationGUI::saveToFile();
        }
        break;
      default:
        QGraphicsView::keyReleaseEvent(e);
        break;
    }
  }
}





// INTERNAL METHODS


void gui::DesignPanel::wheelZoom(QWheelEvent *e, bool boost)
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  if(qAbs(wheel_deg.y())>=120){
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
      scale(1+ds,1+ds);
      QPointF delta = mapToScene(e->pos()) - old_pos;
      verticalScrollBar()->setValue(verticalScrollBar()->value()-delta.y()*transform().m11());
      horizontalScrollBar()->setValue(horizontalScrollBar()->value()-delta.x()*transform().m22());
    }
  }

  // reset both scrolls (avoid repeat from |x|>=120)
  wheel_deg.setX(0);
  wheel_deg.setY(0);

  //qDebug() << tr("Zoom: QTransform m11 = %1, m12 = %2, m21 = %3, m22 = %4, dx = %5, dy = %6").arg(transform().m11()).arg(transform().m12()).arg(transform().m21()).arg(transform().m22()).arg(transform().dx()).arg(transform().dy());
}


void gui::DesignPanel::wheelPan(bool boost)
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  
  qreal dx=0, dy=0;
  QScrollBar *vsb = verticalScrollBar();
  QScrollBar *hsb = horizontalScrollBar();

  // y scrolling
  if(qAbs(wheel_deg.y())>=120){
    if(wheel_deg.y()>0)
      dy -= gui_settings->get<qreal>("view/wheel_pan_step");
    else
      dy += gui_settings->get<qreal>("view/wheel_pan_step");
    wheel_deg.setY(0);
  }

  // x scrolling
  if(qAbs(wheel_deg.x())>=120){
    if(wheel_deg.x()>0)
      dx -= gui_settings->get<qreal>("view/wheel_pan_step");
    else
      dx += gui_settings->get<qreal>("view/wheel_pan_step");
    wheel_deg.setX(0);
  }

  // apply boost
  if(boost){
    qreal boost_fact = gui_settings->get<qreal>("view/wheel_pan_boost");
    dx *= boost_fact;
    dy *= boost_fact;
  }

  vsb->setValue(vsb->value()+dy);
  hsb->setValue(hsb->value()+dx);
}


void gui::DesignPanel::boundZoom(qreal &ds)
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  qreal m = transform().m11();  // m = m11 = m22

  // need zoom_min <= m11*(1+ds) <= zoom_max
  if(ds<0)
    ds = qMax(ds, gui_settings->get<qreal>("view/zoom_min")/m-1);
  else
    ds = qMin(ds, gui_settings->get<qreal>("view/zoom_max")/m-1);
}


void gui::DesignPanel::filterSelection(bool select_flag)
{
  // should only be here if tool type is either select or dbgen
  if(tool_type != gui::DesignPanel::SelectTool && tool_type != gui::DesignPanel::DBGenTool){
    qCritical() << tr("Filtering selection with invalid tool type...");
    return;
  }

  // NOTE need to make sure than only active layers are selectable

  // if select_flag, deselect all items in the lattice. Otherwise, keep only items in the lattice
  for(QGraphicsItem *gitem : scene->selectedItems()){
    if( ( static_cast<prim::Item*>(gitem)->layer_id == 0) == select_flag)
      gitem->setSelected(false);
  }
}


void gui::DesignPanel::rubberBandUpdate(QPoint pos){
  // stop rubber band if moving item
  if(moving){
    rubberBandEnd();
    return;
  }

  // do nothing if mouse hasn't moved much
  // TODO change snap_diameter to a separate variable
  if((pos-rb_cache).manhattanLength()<.1*snap_diameter)
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
    for(QGraphicsItem* selected_item : selected_items){
      // NOTE foregone the check code for now since it doesn't seem to be more efficient
      //if(!rb_shift_selected.contains(selected_item)
      //    || !rb_rect_scene.intersects(selected_item->boundingRect().toRect()))
      selected_item->setSelected(false);
    }

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
  //qDebug() << tr("Creating ghost...");

  prim::Ghost *ghost = prim::Ghost::instance();

  ghosting=true;
  snap_cache = QPointF();

  if(paste){
    ghost->prepare(clipboard);
    QPointF offset;
    if(snapGhost(mapToScene(mapFromGlobal(QCursor::pos())), offset))
      ghost->moveBy(offset.x(), offset.y());
  }
  else{
    //get QList of selected Item object
    filterSelection(true);
    QList<prim::Item*> items;
    for(QGraphicsItem *qitem : scene->selectedItems())
      items.append(static_cast<prim::Item*>(qitem));
    ghost->prepare(items);
  }
}


void gui::DesignPanel::clearGhost()
{
  ///qDebug() << tr("Clearing ghost...");
  prim::Ghost::instance()->cleanGhost();
  ghosting=false;
  snap_target=0;
}


bool gui::DesignPanel::snapGhost(QPointF scene_pos, QPointF &offset)
{
  // don't need to recheck snap target unless the cursor has moved significantly
  if((scene_pos-snap_cache).manhattanLength()<.3*snap_diameter)
    return false;
  snap_cache = scene_pos;

  prim::Ghost *ghost = prim::Ghost::instance();
  prim::GhostDot *anchor = ghost->snapAnchor();

  // if no anchor, allow free movement of the ghost
  if(anchor==0){
    offset = QPointF();
    return true;
  }

  // otherwise restrict possible ghost position to lattice sites

  QPointF old_anchor = anchor->scenePos();
  QPointF free_anchor = ghost->freeAnchor(scene_pos);

  // get nearest lattice site to free anchor
  QRectF rect;
  rect.setSize(QSize(snap_diameter, snap_diameter));
  rect.moveCenter(free_anchor);
  QList<QGraphicsItem*> near_items = scene->items(rect);

  // tdot->setPos(free_anchor);
  // trect->setRect(rect);

  // if no items nearby, change nothing
  if(near_items.count()==0)
    return false;

  // select the nearest lattice point to the free_anchor
  prim::LatticeDot *target=0;
  qreal mdist=-1, dist;

  for(QGraphicsItem *gitem : near_items){
    // lattice dot
    if(static_cast<prim::Item*>(gitem)->item_type == prim::Item::LatticeDot){
      dist = (gitem->pos()-free_anchor).manhattanLength();
      if(mdist<0 || dist<mdist){
        target = static_cast<prim::LatticeDot*>(gitem);
        mdist=dist;
      }
    }
  }

  // if no valid target or target has not changed, do nothing
  if(!target || (target==snap_target))
    return false;

  // move ghost and update validity hash table.
  offset = target->scenePos()-old_anchor;
  if(!ghost->valid_hash.contains(target))
    ghost->valid_hash[target] = ghost->checkValid(offset);
  snap_target = target;
  ghost->setValid(ghost->valid_hash[target]);

  return true;
}


void gui::DesignPanel::initMove()
{
  // create ghost
  createGhost(false);

  // set lattice dots of objects to be moved as selectable
  for(QGraphicsItem *gitem : scene->selectedItems())
    setLatticeDotSelectability(static_cast<prim::Item*>(gitem), true);

  moving = true;
}


void gui::DesignPanel::setLatticeDotSelectability(prim::Item *item, bool flag)
{
  prim::LatticeDot *ldot=0;
  switch(item->item_type){
    case prim::Item::DBDot:
      ldot = static_cast<prim::DBDot*>(item)->getSource();
      ldot->setFlag(QGraphicsItem::ItemIsSelectable, flag);
      break;
    case prim::Item::Aggregate:
      for(prim::Item *it : static_cast<prim::Aggregate*>(item)->getChildren())
        setLatticeDotSelectability(it, flag);
      break;
    case prim::Item::LatticeDot:
      ldot = static_cast<prim::LatticeDot*>(item);
      ldot->setFlag(QGraphicsItem::ItemIsSelectable, flag);
      break;
    default:
      break;
  }
}

void gui::DesignPanel::copySelection()
{
  QList<QGraphicsItem*> selection = scene->selectedItems();
  if(selection.isEmpty())
    return;

  // empty the previous clipboard
  for(prim::Item *item : clipboard)
    delete item;
  clipboard.clear();

  for(QGraphicsItem *gitem : selection)
    clipboard.append(static_cast<prim::Item*>(gitem)->deepCopy());

  qDebug() << tr("Clipboard: %1 items").arg(clipboard.count());
  for(prim::Item *item : clipboard){
    if(item->item_type == prim::Item::Aggregate){
      qDebug() << tr("  aggregate");
    }
    else
      qDebug() << tr("  item at: %1,%2").arg(item->x()).arg(item->y());

  }
}

void gui::DesignPanel::snapDB(QPointF scene_pos)
{
  // don't need to recheck snap target unless the cursor has moved significantly
  if(snap_target != 0 && (scene_pos-snap_cache).manhattanLength()<.1*snap_diameter)
    return;
  snap_cache = scene_pos;

  // get nearest lattice site to cursor position
  QRectF rect;
  rect.setSize(QSize(snap_diameter, snap_diameter));
  rect.moveCenter(scene_pos);
  QList<QGraphicsItem*> near_items = scene->items(rect);

  // if no items nearby, change nothing
  if(near_items.count()==0){
    if(snap_target){
      snap_target->setSelected(false); // unselect the previous target
      snap_target = 0;
    }
    return;
  }

  // select the nearest db to cursor position
  prim::LatticeDot *target=0;
  qreal mdist=-1, dist;

  for(QGraphicsItem *gitem : near_items) {
    // lattice dot
    dist = (gitem->pos()-scene_pos).manhattanLength();
    if(mdist<0 || dist<mdist){
      target = static_cast<prim::LatticeDot*>(gitem);
      mdist=dist;
    }
  }

  // if no valid target or target has not changed, do nothing
  if(!target || target==snap_target)
    return;

  // move db indicator
  if(snap_target)
    snap_target->setSelected(false); // unselect the previous target
  snap_target = target;
  snap_target->setSelected(true); // select the new target

  return;
}





// UNDO/REDO STACK METHODS


// CreateDB class

gui::DesignPanel::CreateDB::CreateDB(prim::LatticeDot *ldot, int layer_index,
                        gui::DesignPanel *dp, prim::DBDot *src_db, bool invert, QUndoCommand *parent)
  : QUndoCommand(parent), invert(invert), dp(dp), layer_index(layer_index), ldot(ldot)
{
  prim::DBDot *dbdot = ldot->getDBDot();

  // dbdot should be 0 if invert is false else non-zero
  if( !(invert ^ (dbdot==0)) )
    qFatal("Either trying to delete a non-existing DB or overwrite an existing one");

  // dbdot index in layer
  prim::Layer *layer = dp->getLayer(layer_index);
  index = invert ? layer->getItems().indexOf(dbdot) : layer->getItems().size();
  elec = src_db ? src_db->getElec() : 0; // TODO in the future, instead of copying properties 1 by 1, probably want to make something that copies all properties
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
  dp->addItem(new prim::DBDot(layer_index, ldot, elec), layer_index, index);
}

void gui::DesignPanel::CreateDB::destroy()
{
  prim::DBDot *dbdot = static_cast<prim::DBDot*>(dp->getLayer(layer_index)->getItem(index));

  if(dbdot != 0){
    // make source lattice site selectable again
    dbdot->getSource()->setDBDot(0);

    // destroy dbdot
    dp->removeItem(dbdot, dp->getLayer(dbdot->layer_id));  // deletes dbdot
    dbdot = 0;
  }
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
    default:
      item->moveBy(delta.x(), delta.y());
      break;
  }
}


void gui::DesignPanel::MoveItem::moveDBDot(prim::DBDot *dot, const QPointF &delta)
{
  // get the target LatticeDot
  QList<QGraphicsItem*> cands = dot->scene()->items(dot->scenePos()+delta);
  prim::LatticeDot *ldot=0;
  for(QGraphicsItem *cand : cands){
    if(static_cast<prim::Item*>(cand)->item_type == prim::Item::LatticeDot){
      ldot=static_cast<prim::LatticeDot*>(cand);
      break;
    }
  }

  if(ldot==0)
    qCritical() << tr("Failed to move DBDot");
  else
    dot->setSource(ldot);
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




// Undo/Redo Methods

void gui::DesignPanel::createDBs()
{
  // do something only if there is a selection
  QList<QGraphicsItem*> selection = scene->selectedItems();
  if(selection.isEmpty())
    return;

  // check that the selection is valid
  prim::Item *item=0;
  for(QGraphicsItem *gitem : selection){
    item = static_cast<prim::Item*>(gitem);
    if(item->item_type != prim::Item::LatticeDot){
      qCritical() << tr("Dangling bond target is not a lattice dot...");
      return;
    }
  }

  // push actions onto the QUndoStack
  int layer_index = layers.indexOf(top_layer);
  undo_stack->beginMacro(tr("create dangling bonds at selected sites"));
  for(QGraphicsItem *gitem : selection)
    undo_stack->push(new CreateDB(static_cast<prim::LatticeDot *>(gitem), layer_index, this));
  undo_stack->endMacro();
}

void gui::DesignPanel::deleteSelection()
{
  // do something only if there is a selection
  QList<QGraphicsItem*> selection = scene->selectedItems();
  if(selection.isEmpty())
    return;

  qDebug() << tr("Deleting %1 items").arg(selection.count());

  undo_stack->beginMacro(tr("delete %1 items").arg(selection.count()));
  for(QGraphicsItem *gitem : selection){
    prim::Item *item = static_cast<prim::Item*>(gitem);
    switch(item->item_type){
      case prim::Item::DBDot:
        undo_stack->push(new CreateDB( static_cast<prim::DBDot*>(item)->getSource(),
                                      item->layer_id, this, 0, true));
        break;
      case prim::Item::Aggregate:
        destroyAggregate(static_cast<prim::Aggregate*>(item));
      default:
        break;
    }
  }
  undo_stack->endMacro();
}


void gui::DesignPanel::formAggregate()
{
  // do something only if there is a selection
  QList<QGraphicsItem*> selection = scene->selectedItems();
  if(selection.isEmpty())
    return;

  // get selected items as prim::Item pointers
  QList<prim::Item*> items;
  for(QGraphicsItem *gitem : selection){
    items.append(static_cast<prim::Item*>(gitem));
    // if(items.last()->layer != layers.at(1)){ DOUBLE CHECK WITH JAKE
    if(items.last()->layer_id != 1){
      qCritical() << tr("Selected aggregate item not in the surface...");
      return;
    }
  }

  if(selection.count()<2){
    qWarning() << tr("Must select multiple items to form an aggregate");
    return;
  }

  // reversably create the aggregate
  undo_stack->push(new FormAggregate(items, this));
}

void gui::DesignPanel::splitAggregates()
{
  // do something only if there is a selection
  QList<QGraphicsItem*> selection = scene->selectedItems();
  if(selection.isEmpty())
    return;

  // get selected aggregates
  QList<prim::Aggregate*> aggs;
  prim::Item *item=0;
  for(QGraphicsItem *gitem : selection){
    item = static_cast<prim::Item*>(gitem);
    if(item->item_type == prim::Item::Aggregate)
      aggs.append(static_cast<prim::Aggregate*>(item));
  }

  if(aggs.count()==0)
    return;

  undo_stack->beginMacro(tr("Split %1 aggregates").arg(aggs.count()));
  int offset=0;
  for(prim::Aggregate *agg : aggs){
    int temp = agg->getChildren().count()-1;
    undo_stack->push(new FormAggregate(agg, offset, this));
    offset += temp;
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
        undo_stack->push(new CreateDB(static_cast<prim::DBDot*>(item)->getSource(),
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

  // do nothing if clipboard empty
  if(clipboard.isEmpty() || !ghost->valid_hash[snap_target])
    return false;

  undo_stack->beginMacro(tr("Paste %1 items").arg(clipboard.count()));

  // paste each item in the clipboard, same as ghost top items (preferred order)
  for(prim::Item *item : ghost->getTopItems())
    pasteItem(ghost, item);

  undo_stack->endMacro();
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
    default:
      qCritical() << tr("No functionality for pasting given item... update pasteItem");
      break;
  }
}

void gui::DesignPanel::pasteDBDot(prim::Ghost *ghost, prim::DBDot *db)
{
  // get the target lattice dor
  prim::LatticeDot *ldot = ghost->getLatticeDot(db);
  if(ldot){
    undo_stack->push(new CreateDB(ldot, getLayerIndex(top_layer), this, db));
  }

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


// NOTE: currently item move relies on there being a snap target (i.e. at least
//       one dangling bond is being moved). Should modify in future to be more
//       general. If no move is made, need to make originial lattice dots
//       unselectable again.
bool gui::DesignPanel::moveToGhost(bool kill)
{
  prim::Ghost *ghost = prim::Ghost::instance();
  moving = false;

  // get the move offset
  QPointF offset = (!kill && ghost->valid_hash[snap_target]) ? ghost->moveOffset() : QPointF();

  if(offset.isNull()){
    // reset the original lattice dot selectability and return false
    for(QGraphicsItem *gitem : scene->selectedItems())
      setLatticeDotSelectability(static_cast<prim::Item*>(gitem), false);
    return false;
  }

  undo_stack->beginMacro(tr("Move items"));

  // move each source item by the offset.
  for(prim::Item *item : ghost->getTopItems())
    undo_stack->push(new MoveItem(item, offset, this));

  undo_stack->endMacro();
  return true;
}
