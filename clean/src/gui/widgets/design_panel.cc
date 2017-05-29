// @file:     design_panel.cc
// @author:   Jake
// @created:  2016.11.02
// @editted:  2017.05.11  - Jake
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
  undo_stack = new QUndoStack();

  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  settings::AppSettings *app_settings = settings::AppSettings::instance();

  scene = new QGraphicsScene(this);
  setScene(scene);

  connect(prim::Emitter::instance(), &prim::Emitter::sig_selectClicked,
            this, &gui::DesignPanel::selectClicked);

  // setup flags
  clicked = ghosting = moving = false;

  // initialising parameters
  snap_diameter = app_settings->get<qreal>("snap/diameter")*prim::Item::scale_factor;
  snap_target = 0;

  tool_type = gui::DesignPanel::None;     // now setTool will update the tool
  setTool(gui::DesignPanel::SelectTool);

  // rubber band selection
  setRubberBandSelectionMode(Qt::ContainsItemBoundingRect);
  setStyleSheet("selection-background-color: rgba(100, 100, 255, 10)");

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

  // surface layer active on init
  top_layer = layers.at(1);
}

// destructor
gui::DesignPanel::~DesignPanel()
{
  delete undo_stack;
  delete scene;       // will NOT delete all contained QGraphicsItem objects.
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
  if(layer->removeItem(item)){
    scene->removeItem(item);
    delete item;
  }
}




void gui::DesignPanel::addLayer(const QString &name)
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

  prim::Layer *layer = new prim::Layer(name);
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
      dbs.append((prim::DBDot *)item);

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
    // do nothing is the lattce has previously been defined
    if(!layers.isEmpty())
      return;
  }

  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  // NOTE: probably want a prompt to make sure user want to change the lattice

  // destroy all layers if they exist
  while(layers.count()>0)
    removeLayer(0);

    qDebug() << tr("Building lattice : %1").arg(fname);

  // build the new lattice
  prim::Lattice *lattice = new prim::Lattice(fname);

  // add the lattice dots to the scene
  for(prim::Item *const item : lattice->getItems())
    scene->addItem(item);

  // add the lattice to the layers, as layer 0
  layers.append(lattice);

  // add in the dangling bond surface
  addLayer(tr("Surface"));
  top_layer = layers.at(1);

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

  switch(tool){
    case gui::DesignPanel::SelectTool:
      setDragMode(QGraphicsView::RubberBandDrag);
      setInteractive(true);
      break;
    case gui::DesignPanel::DragTool:
      setDragMode(QGraphicsView::ScrollHandDrag);
      setInteractive(false);
      break;
    case gui::DesignPanel::DBGenTool:
      setDragMode(QGraphicsView::RubberBandDrag);
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



// SLOTS

void gui::DesignPanel::selectClicked(prim::Item *item)
{
  item=item;
  if(tool_type == gui::DesignPanel::SelectTool){
    // initialise move
    // initMove();
  }
}



// INTERRUPTS

// most behaviour will be connected to mouse move/release. However, when
// ghosting (dragging items or copy/paste), show the ghost on the left button
// press
void gui::DesignPanel::mousePressEvent(QMouseEvent *e)
{
  // set clicked flag and store current mouse position for move behaviour
  mouse_pos_old = e->pos();
  mouse_pos_cached = e->pos(); // this might be a referencing clash, check.

  switch(e->button()){
    case Qt::LeftButton:
      clicked = true;
      if(ghosting){
        // when ghosting, show the ghost at the cursor position on the click
      }
      else
        QGraphicsView::mousePressEvent(e);
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
  QTransform trans = transform();
  qreal dx, dy;

  if(ghosting){
    // update snap
  }
  else if(clicked){
    // not ghosting, mouse dragging of some sort
    switch(e->buttons()){
      case Qt::LeftButton:
        // use default behaviour for left mouse button
        QGraphicsView::mouseMoveEvent(e);
        break;
      case Qt::MidButton:
        // middle button always pans
        mouse_pos_del = e->pos()-mouse_pos_old;
        dx = mouse_pos_del.x()/trans.m11();
        dy = mouse_pos_del.y()/trans.m22();
        translate(dx, dy);
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

  // case specific behaviour
  if(ghosting){
    // plant ghost and end ghosting
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
      case Qt::Key_H:
        // flip ghost horizontally
        break;
      case Qt::Key_V:
        // flip ghost vertically
        break;
      case Qt::Key_R:
        // rotate based on whether the control modifiers is pressed
        break;
      default:
        break;
    }
  }
  else{
    switch(e->key()){
      case Qt::Key_G:
        // grouping behaviour for selecting surface dangling bonds
        break;
      case Qt::Key_C:
        // copy selected items to the clipboard
        break;
      case Qt::Key_V:
        // create ghost for clipboard if any
        break;
      case Qt::Key_Z:{
        // undo/redo based on keymods
        if(keymods == (Qt::ControlModifier | Qt::ShiftModifier))
          undo_stack->redo();
        else if(keymods == Qt::ControlModifier)
          undo_stack->undo();
        }
        break;
      case Qt::Key_Delete:
        // delete selected items
        if(tool_type == gui::DesignPanel::SelectTool)
          deleteSelection();
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
      translate(delta.x(), delta.y());
    }
  }

  // reset both scrolls (avoid repeat from |x|>=120)
  wheel_deg.setX(0);
  wheel_deg.setY(0);
}


void gui::DesignPanel::wheelPan(bool boost)
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  QTransform trans = transform();
  qreal dx=0, dy=0;

  // y scrolling
  if(qAbs(wheel_deg.y())>=120){
    if(wheel_deg.y()>0)
      dy += gui_settings->get<qreal>("view/wheel_pan_step");
    else
      dy -= gui_settings->get<qreal>("view/wheel_pan_step");
    wheel_deg.setY(0);
  }

  // x scrolling
  if(qAbs(wheel_deg.x())>=120){
    if(wheel_deg.x()>0)
      dx += gui_settings->get<qreal>("view/wheel_pan_step");
    else
      dx -= gui_settings->get<qreal>("view/wheel_pan_step");
    wheel_deg.setX(0);
  }

  // apply boost
  if(boost){
    qreal boost_fact = gui_settings->get<qreal>("view/wheel_pan_boost");
    dx *= boost_fact;
    dy *= boost_fact;
  }

  translate(dx/trans.m11(), dy/trans.m22());
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
    if( ( ((prim::Item*) gitem)->layer == layers.at(0)) == select_flag)
      gitem->setSelected(false);
  }
}


// UNDO/REDO STACK METHODS

// CreateDB class

gui::DesignPanel::CreateDB::CreateDB(prim::LatticeDot *ldot, int layer_index,
                        gui::DesignPanel *dp, bool invert, QUndoCommand *parent)
  : QUndoCommand(parent), invert(invert), dp(dp), layer_index(layer_index), ldot(ldot)
{
  prim::DBDot *dbdot = ldot->getDBDot();

  // dbdot should be 0 if invert is false else non-zero
  if( !(invert ^ (dbdot==0)) )
    qFatal("Either trying to delete a non-existing DB or overwrite an existing one");

  // dbdot index in layer
  prim::Layer *layer = dp->getLayer(layer_index);
  index = invert ? layer->getItems().indexOf(dbdot) : layer->getItems().size();
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
  qDebug() << tr("Creating dangling bond: %1::%2 ").arg(layer_index).arg(index);

  // add dangling bond to layer and scene, index in layer item stack will be
  // equal to layer->getItems().size()
  dp->addItem(new prim::DBDot(dp->getLayer(layer_index), ldot), layer_index, index);
}

void gui::DesignPanel::CreateDB::destroy()
{
  prim::DBDot *dbdot = (prim::DBDot*)dp->getLayer(layer_index)->getItem(index);

  if(dbdot != 0){

    qDebug() << tr("Destroying dangling bond: %1::%2").arg(layer_index).arg(index);
    // make source lattice site selectable again
    dbdot->getSource()->setDBDot(0);

    // destroy dbdot
    dp->removeItem(dbdot, dbdot->layer);  // deletes dbdot
    dbdot = 0;
  }
}




// CreateAggregate class

gui::DesignPanel::CreateAggregate::CreateAggregate(QList<prim::Item *> &items,
                                            DesignPanel *dp, QUndoCommand *parent)
  : QUndoCommand(parent), dp(dp), agg(0)
{
  if(items.count()==0){
    qWarning() << tr("Aggregate contains no items");
    return;
  }

  prim::Layer *layer = items.at(0)->layer;

  // get layer_index, and check that it is the same for all items
  for(prim::Item *item : items)
    if(item->layer != layer){
      qWarning() << tr("Aggregates can only be formed from items in the same layer");
      return;
    }

  layer_index = dp->getLayerIndex(layer);

  // format the input items to a pointer invariant form, could be done more efficiently
  for(prim::Item *item : items)
    item_inds.append(layer->getItems().indexOf(item));
  std::sort(item_inds.begin(), item_inds.end());
}

// split the aggregate
void gui::DesignPanel::CreateAggregate::undo()
{
  prim::Layer *layer = dp->getLayer(layer_index);

  // aggregate should be on the top of the layer Item stack, pop it and check
  if(agg != layer->getItems().pop())
    qFatal("Undo/Redo mismatch... something went wrong");

  // remove aggregate and all children from the scene
  agg->scene()->removeItem(agg);

  // destroy the aggregate
  QList<prim::Item*> items = agg->getChildren();
  delete agg;

  // re-insert the component items into the Layer in ascending index order
  int i=0;
  for(prim::Item *item : items)
    dp->addItem(item, layer_index, item_inds.at(i++));
}

// form the aggregate
void gui::DesignPanel::CreateAggregate::redo()
{
  prim::Layer *layer = dp->getLayer(layer_index);

  // all items should be in the same layer as the aggregate was and have no parents
  prim::Item *item=0;
  for(const int &ind : item_inds){
    if(ind >= layer->getItems().size())
      qFatal("Undo/Redo mismatch... something went wrong");
    item = layer->getItems().at(ind);
    if(item->layer != layer || item->parentItem() != 0)
      qFatal("Undo/Redo mismatch... something went wrong");
  }

  // remove the items from the Layer stack in reverse order
  QList<prim::Item*> items;
  for(int i=item_inds.count()-1; i>=0; i--){
    items.append(layer->getItems().takeAt(item_inds.at(i)));
  }

  // remove all Items from the scene
  for(prim::Item *item : items)
    item->scene()->removeItem(item);

  // create new aggregate
  agg = new prim::Aggregate(layer, items);

  // add aggregate to system
  dp->addItem(agg, layer_index);
}


// Methods

void gui::DesignPanel::createDBs()
{

  // check that the selection is valid
  prim::Item *item=0;
  for(QGraphicsItem *gitem : scene->selectedItems()){
    item = (prim::Item*)gitem;
    if(item->item_type != prim::Item::LatticeDot){
      qCritical() << tr("Dangling bond target is not a lattice dot...");
      return;
    }
  }

  // push actions onto the QUndoStack
  int layer_index = layers.indexOf(top_layer);
  undo_stack->beginMacro(tr("create dangling bonds at selected sites"));
  for(QGraphicsItem *gitem : scene->selectedItems())
    undo_stack->push(new CreateDB((prim::LatticeDot *)gitem, layer_index, this));
  undo_stack->endMacro();
}

void gui::DesignPanel::deleteSelection()
{
  QList<QGraphicsItem*> items = scene->selectedItems();
  qDebug() << tr("Deleting %1 items").arg(items.count());

  undo_stack->beginMacro(tr("delete %1 items").arg(items.count()));
  for(QGraphicsItem *gitem : items){
    prim::Item *item = (prim::Item*) gitem;
    switch(item->item_type){
      case prim::Item::DBDot:
        undo_stack->push(new CreateDB( ((prim::DBDot*)item)->getSource(),
                                      getLayerIndex(item->layer), this, true));
        break;
      default:
        break;
    }
  }
  undo_stack->endMacro();

}
