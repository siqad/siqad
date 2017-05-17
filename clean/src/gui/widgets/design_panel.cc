// @file:     design_panel.cc
// @author:   Jake
// @created:  2016.11.02
// @editted:  2017.05.11  - Jake
// @license:  GNU LGPL v3
//
// @desc:     DesignPanel implementation




#include "design_panel.h"
#include "src/settings/settings.h"

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
  delete scene;
}


// ACCESSORS

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
    // remove all items in the layer from the scene and delete
    QList<prim::Item *> items = layers.at(n)->getItems();
    for(int i=0; i<items.count();i++){
      scene->removeItem(items.at(i));
      delete items.at(i);
    }

    // delete layer
    delete layers.at(n);
    layers.removeAt(n);

    // if top_layer was remove, default to surface if available else NULL
    if(top_layer==layers.at(n))
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


int gui::DesignPanel::getTopLayerIndex() const
{
  return top_layer==0 ? -1 : layers.indexOf(top_layer);
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
  // Qt::KeyboardModifiers keymods = QApplication::keyboardModifiers();

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
      case Qt::Key_Delete:
        // delete selected items
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

  // if select_flag, deselect all items in the lattice. Otherwise, keep only items in the lattice
  for(QGraphicsItem *gitem : scene->selectedItems()){
    if( ( ((prim::Item*) gitem)->layer == layers.at(0)) == select_flag)
      gitem->setSelected(false);
  }
}


// UNDO/REDO STACK METHODS

// CreateDB class

gui::DesignPanel::CreateDB::CreateDB(prim::LatticeDot *dot, prim::Layer *layer,
                                            QGraphicsScene *scene, QUndoCommand *parent)
  : QUndoCommand(parent), scene(scene), layer(layer), ldot(dot), dbdot(0)
{}

void gui::DesignPanel::CreateDB::undo()
{
  cleanDB();
}

void gui::DesignPanel::CreateDB::redo()
{
  // clean the dangling bond... should do nothing
  cleanDB();

  // create the new dangling bond
  dbdot = new prim::DBDot(layer, ldot);
  ldot->setFlag(QGraphicsItem::ItemIsSelectable, false);
  dbdot->setFlag(QGraphicsItem::ItemIsSelectable, true);

  // add dangling bond to layer and scene
  layer->addItem(dbdot);
  scene->addItem(dbdot);

  // debug echo
  QPointF ploc = dbdot->getPhysLoc();
  qDebug() << tr("DB added at (%1 , %2)").arg(ploc.x()).arg(ploc.y());
}

void gui::DesignPanel::CreateDB::cleanDB()
{
  if(dbdot != 0){
    // make source lattice site selectable again
    dbdot->getSource()->setFlag(QGraphicsItem::ItemIsSelectable, true);
    // destroy dot
    layer->removeItem(dbdot);
    scene->removeItem(dbdot); // should call free dbdot
    dbdot = 0;
  }
}


// CreateAggregate class

gui::DesignPanel::CreateAggregate::CreateAggregate(QList<prim::Item *> items,
                                            prim::Layer *layer, QUndoCommand *parent)
  : QUndoCommand(parent), items(items), layer(layer), agg(0)
{}

void gui::DesignPanel::CreateAggregate::undo()
{
  cleanAgg();
}

void gui::DesignPanel::CreateAggregate::redo()
{
  // should only create aggregates out of items with no parents
  for(prim::Item *item : items)
    if(item->parentItem())
      qCritical() << tr("Item already has a parent...");

  agg = new prim::Aggregate(layer, items);

  // other stuff to make sure scene and layers is informed
}

void gui::DesignPanel::CreateAggregate::cleanAgg()
{
  if(agg!=0){
    // reassign children's parent to aggregate's parent

    // inform scene and layers
  }
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
  undo_stack->beginMacro(tr("create dangling bonds at selected sites"));
  for(QGraphicsItem *gitem : scene->selectedItems())
    undo_stack->push(new CreateDB((prim::LatticeDot *)gitem, top_layer, scene));
  undo_stack->endMacro();
}
