#include "design_widget.h"

#include <QApplication>
#include <QDebug>
#include <QString>
#include <QPointF>

#include <QVariant>
#include <QColor>
#include <QBrush>
#include <QPainter>

#include "src/settings/settings.h"
#include "lattice.h"


gui::DesignWidget::DesignWidget(QWidget *parent)
  :QGraphicsView(parent)
{
  settings::GUISettings gui_settings;
  settings::AppSettings app_settings;

  scene = new QGraphicsScene(this);
  setScene(scene);

  connect(prim::Emitter::instance(), &prim::Emitter::sig_selectClicked, this, &gui::DesignWidget::selectClicked);

  // setup flags
  clicked = false;
  ghosting = false;

  // initialise parameters
  qreal scale_fact = gui_settings.get<qreal>("dbdot/scale_fact");
  snap_diameter = app_settings.get<qreal>("snap/diameter")*scale_fact;
  snap_target = 0;
  ghost = 0;

  tool_type = gui::DesignWidget::SelectTool;

  setDragMode(QGraphicsView::RubberBandDrag);
  setRubberBandSelectionMode(Qt::ContainsItemBoundingRect);
  setStyleSheet("selection-background-color: rgba(100, 100, 255, 10)");

  // set view behaviour
  setTransformationAnchor(QGraphicsView::NoAnchor);
  setResizeAnchor(QGraphicsView::AnchorViewCenter);

  setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
          QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);

  // color scheme
  QColor col;
  setBackgroundBrush(QBrush(gui_settings.get<QColor>("view/bg_col")));
  setFrameShadow(QFrame::Raised);

  // make lattice and surface layer
  buildLattice();

  // surface layer active on init
  top_layer = layers.at(1);
}

gui::DesignWidget::~DesignWidget()
{
  delete scene;
}



void gui::DesignWidget::addLayer()
{
  prim::Layer *layer = new prim::Layer();
  layers.append(layer);
}

void gui::DesignWidget::addLayer(const QString &name)
{
  prim::Layer *layer = new prim::Layer(name);
  layers.append(layer);
}



void gui::DesignWidget::removeLayer(const QString &name)
{
  bool removed=false;
  for(int i=0; i<layers.count(); i++){
    if(layers.at(i)->getName() == name){
      removeLayer(i);
      removed=true;
      break;
    }
  }

  if(!removed)
    qWarning() << QString("Failed to remove layer : %1").arg(name);
}

void gui::DesignWidget::removeLayer(int n)
{
  if(n<0 || n>= layers.count())
    qWarning("Layer index out of bounds...");
  else{
    // remove all items in the layer from the scene and delete
    QList<QGraphicsItem*> items = layers.at(n)->getItems();
    for(int i=0; i<items.count(); i++){
      scene->removeItem(items.at(i));
      delete items.at(i);
    }

    // delete layer
    delete layers.at(n);
    layers.removeAt(n);

    // if top_layer was removed, default to surface if available else NULL
    if(top_layer==layers.at(n)){
      if(layers.count()<2)
        top_layer = 0;
      else
        top_layer = layers.at(1);
    }

  }
}


prim::Layer *gui::DesignWidget::getLayer(const QString &name)
{
  prim::Layer *layer=0;
  for(int i=0; i<layers.count(); i++){
    if(layers.at(i)->getName() == name){
      layer = layers.at(i);
      break;
    }
  }

  if(layer==0)
    qWarning("Failed to find layer");

  return layer;
}

prim::Layer *gui::DesignWidget::getLayer(int n)
{
  prim::Layer *layer=0;
  if(n<0 || n>= layers.count())
    qWarning("Layer index out of bounds...");
  else
    layer = layers.at(n);
  return layer;
}

QList<prim::DBDot*> gui::DesignWidget::getSurfaceDBs()
{
  if(layers.count()<2){
    qWarning("requesting non-existant surface...");
    return QList<prim::DBDot*>();
  }

  QList<QGraphicsItem*> items = layers.at(1)->getItems();
  QList<prim::DBDot*> dbs;
  for(int i=0; i<items.count(); i++)
    dbs.append((prim::DBDot*)items.at(i));
  return dbs;
}

void gui::DesignWidget::setLayer(const QString &name)
{
  top_layer = 0;
  for(int i=0; i<layers.count(); i++){
    if(layers.at(i)->getName() == name){
      top_layer = layers.at(i);
      break;
    }
  }

  if(top_layer==0)
    qWarning("Failed to find layer: top_layer set to NULL");
}

void gui::DesignWidget::setLayer(int n)
{
  top_layer=0;
  if(n<0 || n>= layers.count())
    qWarning("Layer index out of bounds...");
  else
    top_layer = layers.at(n);
}


void gui::DesignWidget::buildLattice(const QString fname)
{
  settings::GUISettings gui_settings;

  // destroy all layers if they exist
  while(layers.count()>0)
    removeLayer(0);

  // build the new lattice
  gui::Lattice *lattice=0;
  if(fname.isEmpty())
    lattice = new Lattice();
  else
    lattice = new Lattice(fname);

  // add lattice dots to the scene
  QGraphicsItem *item=0;
  QList<QGraphicsItem*> items = lattice->getItems();
  for(int i=0; i<items.count(); i++){
    item = items.at(i);
    scene->addItem(item);
  }

  // add the lattice to the layers
  layers.append(lattice);

  // add in the sb-surface layer
  addLayer("Surface");

  // set surface as top layer
  top_layer = layers.at(1);

  // resize the scene with padding
  QRectF rect = scene->sceneRect();
  qreal pad = qMin(rect.width(), rect.height())*gui_settings.get<qreal>("view/padding");
  rect.adjust(-.5*pad, -.5*pad, pad, pad);
  scene->setSceneRect(rect);
}


void gui::DesignWidget::setTool(gui::DesignWidget::ToolType tool)
{
  // if tool is already being used, do nothing
  if(tool==tool_type)
    return;

  // reset selected items
  scene->clearSelection();

  switch(tool){
    case gui::DesignWidget::SelectTool:
      setDragMode(QGraphicsView::RubberBandDrag);
      setInteractive(true);
      break;
    case gui::DesignWidget::DragTool:
      setDragMode(QGraphicsView::ScrollHandDrag);
      setInteractive(false);
      break;
    case gui::DesignWidget::DBGenTool:
      setDragMode(QGraphicsView::RubberBandDrag);
      setInteractive(true);
      break;
    default:
      qCritical("Invalid ToolType... should not have happened");
      return;
      break;
  }

  tool_type=tool;
}


void gui::DesignWidget::setFills(float *fills)
{
  QList<prim::DBDot*> dbs = getSurfaceDBs();
  for(int i=0; i<dbs.count(); i++)
    dbs.at(i)->setFill(fills[i]);
}



// SLOTS


void gui::DesignWidget::selectClicked(QGraphicsItem *item)
{
  if(tool_type == gui::DesignWidget::SelectTool)
    initMove();
}


// INTERRUPTS

void gui::DesignWidget::mousePressEvent(QMouseEvent *e)
{
  // set clicked flag and store current mouse position for move behaviour

  // if(clicked)
  //   qWarning("clicked flag was not reset... possible undesired behaviour");

  clicked=true;
  old_mouse_pos = e->pos();

  if(ghosting){
    QPointF scene_pos = mapToScene(e->pos());
    ghost->setPos(scene_pos);
  }
  else{
    QGraphicsView::mousePressEvent(e);
  }

}


void gui::DesignWidget::mouseMoveEvent(QMouseEvent *e)
{
  QPoint delta_mouse_pos;
  QTransform trans = transform();
  qreal dx, dy;

  if(ghosting){
    QPointF scene_pos = mapToScene(e->pos());
    QPointF offset;
    if(snapGhost(scene_pos, &offset))
      ghost->setPos(ghost->pos()+offset);
  }
  else if(clicked){
    switch(e->buttons()){
      case Qt::LeftButton:
        QGraphicsView::mouseMoveEvent(e);
        break;
      case Qt::MidButton:
        delta_mouse_pos = e->pos()-old_mouse_pos;
        dx = delta_mouse_pos.x()/trans.m11();
        dy = delta_mouse_pos.y()/trans.m22();
        translate(dx, dy);
        old_mouse_pos = e->pos();
        break;
      case Qt::RightButton:
        // currently no behaviour for right click drag
        break;
      case Qt::NoButton:
        // do nothing if no button pressed... display coords?
        break;
      default:
        // multi button behaviour not implemented
        break;
    }
  }

}

void gui::DesignWidget::mouseReleaseEvent(QMouseEvent *e)
{

  QGraphicsView::mouseReleaseEvent(e);
  QPointF scene_pos = mapToScene(e->pos());

  if(ghosting){
    // plant ghost and end ghosting
    plantGhost();
    if(moving)
      deleteSelected();
    destroyGhost();
  }
  else if(clicked){
    switch(e->button()){
      case Qt::LeftButton:
        QGraphicsView::mouseReleaseEvent(e);
        // filter selected items depending on the current tool
        switch(tool_type){
          case gui::DesignWidget::SelectTool:
            filterSelection(true);
            break;
          case gui::DesignWidget::DBGenTool:
            filterSelection(false);
            createDBs();
            break;
          default:
            break;
        }
        break;
      case Qt::MidButton:
        break;
      case Qt::RightButton:
        break;
      default:
        // multi button behaviour not implemented
        break;
    }
  }
  else{
    qWarning("clicked flag was not initialized... going nothing");
  }
}


void gui::DesignWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
  QGraphicsView::mouseDoubleClickEvent(e);
}






void gui::DesignWidget::wheelEvent(QWheelEvent *e)
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


void gui::DesignWidget::keyPressEvent(QKeyEvent *e)
{
  // for now, just use the default functionality
  QGraphicsView::keyPressEvent(e);
}


void gui::DesignWidget::keyReleaseEvent(QKeyEvent *e)
{
  Qt::KeyboardModifiers keymods = QApplication::keyboardModifiers();
  QGraphicsItemGroup *group = 0;

  if(ghosting){
    switch(e->key()){
      case Qt::Key_H:
        ghost->flip(true);
        break;
      case Qt::Key_V:
        ghost->flip(false);
        break;
      case Qt::Key_R:
        ghost->rotate(keymods & Qt::ControlModifier);
        break;
      default:
        break;
    }
  }
  else{
    switch(e->key()){
      case Qt::Key_G:
        // only do grouping behaviour for surface items
        if(tool_type != gui::DesignWidget::SelectTool)
          break;
        if(keymods == Qt::ControlModifier)
          createAggregate();
        else if(keymods == (Qt::ShiftModifier + Qt::ControlModifier))
          destroyAggregates();
        break;
      case Qt::Key_C:
        if((tool_type == gui::DesignWidget::SelectTool) && (keymods == Qt::ControlModifier))
          createGhost(true);
        break;
      case Qt::Key_Delete:
        if(tool_type == gui::DesignWidget::SelectTool)
          deleteSelected();
        break;
      default:
        QGraphicsView::keyReleaseEvent(e);
        break;
    }
  }
}






// ASSIST METHODS


void gui::DesignWidget::wheelZoom(QWheelEvent *e, bool boost)
{
  settings::GUISettings gui_settings;

  if(qAbs(wheel_deg.y())>=120){
    qreal ds;
    if(wheel_deg.y()>0)
      ds = gui_settings.get<float>("view/zoom_factor");
    else
      ds = -gui_settings.get<float>("view/zoom_factor");

    // apply boost
    if(boost)
      ds *= gui_settings.get<qreal>("view/zoom_boost");

    // assert scale limitations
    boundZoom(&ds);

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

void gui::DesignWidget::wheelPan(bool boost)
{
  settings::GUISettings gui_settings;

  QTransform trans = transform();
  qreal dx = 0;
  qreal dy = 0;

  // y scrolling
  if(qAbs(wheel_deg.y())>=120){
    if(wheel_deg.y()>0)
      dy += gui_settings.get<qreal>("view/wheel_pan_step");
    else
      dy -= gui_settings.get<qreal>("view/wheel_pan_step");
    wheel_deg.setY(0);
  }

  // x scrolling
  if(qAbs(wheel_deg.x())>=120){
    if(wheel_deg.x()>0)
      dx += gui_settings.get<qreal>("view/wheel_pan_step");
    else
      dx -= gui_settings.get<qreal>("view/wheel_pan_step");
    wheel_deg.setX(0);
  }

  // apply boost
  if(boost){
    qreal boost_fact = gui_settings.get<qreal>("view/wheel_pan_boost");
    dx *= boost_fact;
    dy *= boost_fact;
  }

  translate(dx/trans.m11(), dy/trans.m22());
}

void gui::DesignWidget::boundZoom(qreal *ds)
{
  settings::GUISettings gui_settings;
  qreal m = transform().m11();  //m = m11 = m22

  // need zoom_min <= m11*(1+ds) <= zoom_max
  if(*ds<0)
    *ds = qMax(*ds, gui_settings.get<qreal>("view/zoom_min")/m-1);
  else
    *ds = qMin(*ds, gui_settings.get<qreal>("view/zoom_max")/m-1);
}


void gui::DesignWidget::filterSelection(bool select_flag)
{
    // should only be here if tool_type is either select or dbgen
    if(tool_type != gui::DesignWidget::SelectTool && tool_type != gui::DesignWidget::DBGenTool){
      qCritical("Filtering selection with invalid tool type...");
      return;
    }

    QList<QGraphicsItem *> items = scene->selectedItems();
    QGraphicsItem *item=0;

    // if select, reject items in the lattice, otherwise keep only items in the lattice
    for(QList<QGraphicsItem*>::iterator it=items.begin(); it!=items.end(); ++it){
      if(inLattice(*it)==select_flag)
        (*it)->setSelected(false);
    }
}

// Items in the lattice will always be DBDots not belonging to a group
// for now, assume items not in groups are DBDots
bool gui::DesignWidget::inLattice(QGraphicsItem *item)
{
  prim::MyItem *p = 0;
  if(item->childItems().count()==0){
    p = (prim::MyItem*)item;
    if(p->layer==0)
      return true;
  }
  return false;
}


void gui::DesignWidget::createDBs()
{
  prim::DBDot *dot=0;
  QList<QGraphicsItem*> items = scene->selectedItems();
  for(int i=0; i<items.count(); i++){
    dot = (prim::DBDot*) items.at(i);
    createDB(dot);
  }
}


void gui::DesignWidget::createDB(prim::DBDot *dot)
{
  QPointF loc = dot->getPhysLoc();

  // create new db and set lattice site as non-selectable
  prim::DBDot *db = new prim::DBDot(loc, 1, dot);
  dot->setFlag(QGraphicsItem::ItemIsSelectable, false);


  // set initial flags for the surface dot
  db->setFlag(QGraphicsItem::ItemIsSelectable, true);

  layers.at(1)->addItem(db);
  scene->addItem(db);
  //qDebug() << QString("DB added at (%1 , %2)").arg(QString::number(loc.x()), QString::number(loc.y()));
}

void gui::DesignWidget::destroyDB(prim::DBDot *dot)
{
  // make source lattice site selectable again
  dot->getSource()->setFlag(QGraphicsItem::ItemIsSelectable, true);

  // destroy dot
  layers.at(1)->removeItem(dot);;
  scene->removeItem(dot);
}


void gui::DesignWidget::createAggregate()
{
  prim::Aggregate *aggregate = new prim::Aggregate();
  QList<QGraphicsItem*> items=scene->selectedItems();

  if(items.count()>0){
    for(int i=0; i<items.count(); i++)
      aggregate->addToGroup(items.at(i));
    scene->addItem(aggregate);
    aggregate->setFlag(QGraphicsItem::ItemIsSelectable, true);
    aggregate->setSelected(true);
  }
}

void gui::DesignWidget::destroyAggregates()
{
  QGraphicsItemGroup *group=0;
  QList<QGraphicsItem *> new_selected;
  QList<QGraphicsItem *> items = scene->selectedItems();

  for(int i=0; i<items.count(); i++){
    if(items.at(i)->childItems().count()>0){
      group = (QGraphicsItemGroup *) items.at(i);
      new_selected.append(group->childItems());
      scene->destroyItemGroup(group);
    }
    else
      new_selected.append(items.at(i));
  }

  // update selected flags
  for(int i=0; i<new_selected.count(); i++){
    new_selected.at(i)->setSelected(false);
    //qDebug() << QString("Item %1 set to selected").arg(i);
  }
}


void gui::DesignWidget::deleteSelected()
{
  QList<QGraphicsItem*> items = scene->selectedItems();

  qDebug() << QString("Deleting %1 items").arg(items.count());
  for(int i=0; i<items.count(); i++)
    deleteItem(items.at(i));
}

// recursively delete all children of a graphics item
void gui::DesignWidget::deleteItem(QGraphicsItem *item)
{
  QList<QGraphicsItem*> children = item->childItems();
  prim::DBDot *dot = 0;

  if(children.count()==0){
    // item is a DBDot
    dot = (prim::DBDot *)item;

    // remove dot from layer
    layers.at(1)->removeItem(dot);

    // make reserved lattice site selectable
    dot->getSource()->setFlag(QGraphicsItem::ItemIsSelectable, true);

    // delete item
    scene->removeItem(dot);
    delete dot;
  }
  else{
    for(int i=0; i<children.count(); i++)
      deleteItem(children.at(i));

    // delete item
    scene->destroyItemGroup((QGraphicsItemGroup*) item);
  }
}




void gui::DesignWidget::createGhost(bool selected)
{
  // delete old ghost if it exists
  destroyGhost();

  // build ghost if valid source items
  if(selected){
    QList<QGraphicsItem*> items = scene->selectedItems();
    if(items.count()>0){
      ghost = new prim::Ghost(scene);
      ghost->prepare(items);
    }
  }
  else if(clipboard != 0){
    ghost = new prim::Ghost(scene);
    ghost->prepare(clipboard);
  }

  // if ghost was successfully created, enter ghosting mode
  if(ghost != 0){
    //qDebug("Beginning ghosting...");
    ghosting = true;
    snap_target = 0;
  }
}



void gui::DesignWidget::plantGhost()
{
  QList<prim::DBDot*> targets = ghost->getTargets();
  QList<QGraphicsItem*> source = ghost->getSource();

  // create dots on all non-null targets
  for(int i=0; i<targets.count(); i++){
    if(targets.at(i) != 0)
      createDB(targets.at(i));
  }

}


void gui::DesignWidget::destroyGhost()
{
  // delete old ghost
  if(ghost != 0){
    scene->removeItem(ghost);
    delete ghost;
    ghost = 0;
  }

  // default flags
  moving = false;
  ghosting = false;
}




bool gui::DesignWidget::snapGhost(QPointF scene_pos, QPointF *offset)
{
  QGraphicsItem *new_target = 0;

  QRectF rect;
  QPointF old_anchor = ghost->pos()+ghost->getAnchor();
  QPointF free_anchor = scene_pos+ghost->getAnchor();

  // get all possible new anchors near the free_anchor
  rect.moveCenter(free_anchor);
  rect.setSize(QSize(snap_diameter, snap_diameter));
  QList<QGraphicsItem*> near_items = scene->items(rect);

  // if no items nearby, change nothing
  if(near_items.count()==0)
    return false;

  // SPEED UP:
  // - only consider nearest selectable lattice site (don't pick minimum)
  // - don't need to check validity of sites checked before (hash table?)
  //

  // select the nearest lattice point to the free_anchor
  prim::DBDot *target=0;
  qreal dist, min_dist=-1;

  for(QList<QGraphicsItem*>::iterator it=near_items.begin(); it!=near_items.end(); ++it){
    if(inLattice(*it) && ((*it)->flags() & QGraphicsItem::ItemIsSelectable)){
      dist = ((*it)->pos()-free_anchor).manhattanLength();
      if(min_dist < 0 || dist < min_dist){
        target = (prim::DBDot*)(*it);
        min_dist = dist;
      }
    }
  }

  // if there is no valid target or the target hasn't changed, change nothing
  if(!target || (target == snap_target))
    return false;

  bool change_flag = false;
  *offset = target->pos() - old_anchor;
  if(ghost->valid_hash.contains(target)){
    // if target known to be valid, update ghost
    if(ghost->valid_hash[target]){
      snap_target = target;
      ghost->setValid(true);
      change_flag = true;
    }
  }
  else{
    // determine if target is valid
    bool valid = ghost->checkValid(*offset);
    if(valid){
      snap_target = target;
      ghost->setValid(true);
      change_flag = true;
    }
    // insert new key into hash table
    ghost->valid_hash[target] = valid;
  }

  return change_flag;
}



void gui::DesignWidget::initMove()
{

  // create ghost
  createGhost(true);

  // set lattice sites of object to be moved as selectable
  QList<prim::DBDot*> dbs = ghost->getLattice();
  for(QList<prim::DBDot*>::iterator it=dbs.begin(); it!=dbs.end(); ++it)
    (*it)->setFlag(QGraphicsItem::ItemIsSelectable, true);

  // set moving flag
  moving=true;
}

void gui::DesignWidget::completeMove()
{
}
