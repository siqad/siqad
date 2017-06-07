// @file:     design_panel.cc
// @author:   Jake
// @created:  2016.11.02
// @editted:  2017.05.31  - Jake
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
  // set-up scale_factor in prim::Item
  prim::Item::init();

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
  qDebug() << tr("SD: %1").arg(snap_diameter);
  snap_target = 0;

  tool_type = gui::DesignPanel::None;     // now setTool will update the tool
  setTool(gui::DesignPanel::SelectTool);

  // rubber band selection
  setRubberBandSelectionMode(Qt::IntersectsItemBoundingRect);
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

  // initialise the Ghost and set the scene
  prim::Ghost::instance()->setScene(scene);

  // set up test objects
  tdot = new QGraphicsEllipseItem();
  trect = new QGraphicsRectItem();

  scene->addItem(tdot);
  scene->addItem(trect);
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
    // do nothing is the lattce has previously been defined
    if(!layers.isEmpty())
      return;
  }

  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  // NOTE: probably want a prompt to make sure user want to change the lattice

  // destroy all layers if they exist
  while(layers.count()>0)
    removeLayer(0);

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
  // for now, if an item is clicked in selection mode, act as though the highest
  // level aggregate was clicked
  if(tool_type == gui::DesignPanel::SelectTool){

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
    QPointF scene_pos = mapToScene(e->pos());
    QPointF offset;
    if(snapGhost(scene_pos, offset))
      prim::Ghost::instance()->moveBy(offset.x(), offset.y());
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
    moveToGhost();
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
        if(keymods == (Qt::ControlModifier | Qt::ShiftModifier))
          splitAggregates();
        else if(keymods == Qt::ControlModifier)
          formAggregate();
        break;
      case Qt::Key_C:
        // copy selected items to the clipboard
        break;
      case Qt::Key_V:
        // create ghost for clipboard if any
        createGhost();
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
    if( ( static_cast<prim::Item*>(gitem)->layer == layers.at(0)) == select_flag)
      gitem->setSelected(false);
  }
}


void gui::DesignPanel::createGhost()
{
  qDebug() << tr("Creating ghost...");

  prim::Ghost *ghost = prim::Ghost::instance();

  //get QList of selected Item object
  filterSelection(true);
  QList<prim::Item*> items;
  for(QGraphicsItem *qitem : scene->selectedItems())
    items.append(static_cast<prim::Item*>(qitem));

  ghost->prepare(items);
  ghosting=true;
  snap_cache = QPointF();
}


void gui::DesignPanel::clearGhost()
{
  qDebug() << tr("Clearing ghost...");
  prim::Ghost::instance()->cleanGhost();
  ghosting=false;
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
  moving = true;
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
  // add dangling bond to layer and scene, index in layer item stack will be
  // equal to layer->getItems().size()
  dp->addItem(new prim::DBDot(dp->getLayer(layer_index), ldot), layer_index, index);
}

void gui::DesignPanel::CreateDB::destroy()
{
  prim::DBDot *dbdot = static_cast<prim::DBDot*>(dp->getLayer(layer_index)->getItem(index));

  if(dbdot != 0){
    // make source lattice site selectable again
    dbdot->getSource()->setDBDot(0);

    // destroy dbdot
    dp->removeItem(dbdot, dbdot->layer);  // deletes dbdot
    dbdot = 0;
  }
}




// FromAggregate class
gui::DesignPanel::FormAggregate::FormAggregate(QList<prim::Item *> &items,
                                            DesignPanel *dp, QUndoCommand *parent)
  : QUndoCommand(parent), invert(false), dp(dp), agg(0), agg_index(-1)
{
  if(items.count()==0){
    qWarning() << tr("Aggregate contains no items");
    return;
  }

  // get layer_index, and check that it is the same for all items
  prim::Layer *layer = items.at(0)->layer;
  layer_index = dp->getLayerIndex(layer);
  for(prim::Item *item : items)
    if(item->layer != layer){
      qWarning() << tr("Aggregates can only be formed from items in the same layer");
      return;
    }

  // format the input items to a pointer invariant form, could be done more efficiently
  QStack<prim::Item*> layer_items = layer->getItems();
  for(prim::Item *item : items)
    item_inds.append(layer_items.indexOf(item));
  std::sort(item_inds.begin(), item_inds.end());
}

gui::DesignPanel::FormAggregate::FormAggregate(prim::Aggregate *agg, int offset,
                                          DesignPanel *dp, QUndoCommand *parent)
  : QUndoCommand(parent), invert(true), dp(dp), agg(agg)
{
  // get layer index, assumes aggregate was formed using FormAggregate
  prim::Layer *layer = agg->layer;
  layer_index = dp->getLayerIndex(layer);

  // aggregate index
  QStack<prim::Item*> layer_items = layer->getItems();
  agg_index = layer_items.indexOf(agg);

  // doesn't really matter where we add the items from the aggregate to the layer
  // item stack... add to the end
  for(int i=layer_items.count()-1, j=0; j < agg->getChildren().count(); j++)
    item_inds.append(i+j+offset);

  qDebug() << tr("Splitting Agg :: Layer index: %1").arg(layer_index);
  for(const int& ind : item_inds)
    qDebug() << tr("   item %1").arg(ind);
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
    if(item->layer != layer || item->parentItem() != 0)
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

  // create new aggregate
  agg = new prim::Aggregate(layer, items);

  // add aggregate to system
  dp->addItem(agg, layer_index, agg_index);
}


void gui::DesignPanel::FormAggregate::split()
{
  prim::Layer *layer = dp->getLayer(layer_index);

  prim::Item *item = layer->takeItem(agg_index);

  if(item->item_type == prim::Item::Aggregate)
    agg = static_cast<prim::Aggregate*>(item);
  else
    qFatal("Undo/Redo mismatch... something went wrong");

  // remove aggregate and all children from the scene
  agg->scene()->removeItem(agg);

  // destroy the aggregate
  QStack<prim::Item*> items = agg->getChildren();
  delete agg;

  // re-insert the component items into the Layer from the item stack
  for(const int& ind : item_inds)
    dp->addItem(items.pop(), layer_index, ind);
}


// MoveItem class
gui::DesignPanel::MoveItem::MoveItem(prim::Item *item, const QPointF &offset,
                                      DesignPanel *dp, QUndoCommand *parent)
  : QUndoCommand(parent), dp(dp), offset(offset)
{
  layer_index = dp->getLayerIndex(item->layer);
  item_index = item->layer->getItems().indexOf(item);
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

  // redraw old and new bounding rects to handle artifacts
  item->scene()->update(old_rect);
  item->scene()->update(item->boundingRect());
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
  else{
    dot->setSource(ldot);
  }
}


void gui::DesignPanel::MoveItem::moveAggregate(prim::Aggregate *agg, const QPointF &delta)
{
  // for Aggregates, move only the contained Items
  for(prim::Item *item : agg->getChildren()){
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
                                      getLayerIndex(item->layer), this, true));
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
    if(items.last()->layer != layers.at(1)){
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

  if(aggs.count()==0){
    qWarning() << tr("No aggregates selected to ungroup");
    return;
  }

  undo_stack->beginMacro(tr("Split %1 aggregates").arg(aggs.count()));
  int offset=0;
  for(prim::Aggregate *agg : aggs){
    undo_stack->push(new FormAggregate(agg, offset, this));
    offset += agg->getChildren().count()-1;
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
                                      getLayerIndex(item->layer), this, true));
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
  return false;
}

// NOTE: currently item move relies on there being a snap target (i.e. at least
//       one dangling bond is being moved). Should modify in future to be more
//       general.
bool gui::DesignPanel::moveToGhost()
{
  prim::Ghost *ghost = prim::Ghost::instance();

  // return False if move is invalid
  if(!ghost->valid_hash[snap_target])
    return false;

  // otherwise move is valid, get offset
  QPointF offset = ghost->moveOffset();

  undo_stack->beginMacro(tr("Move items"));

  // move each source item by the offset.
  for(prim::Item *item : ghost->getTopItems())
    undo_stack->push(new MoveItem(item, offset, this));

  undo_stack->endMacro();
  return true;
}
