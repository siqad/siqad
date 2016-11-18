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

  scene = new QGraphicsScene(this);
  setScene(scene);

  // setup flags
  clicked = false;

  // set view behaviour
  setTransformationAnchor(QGraphicsView::NoAnchor);
  setResizeAnchor(QGraphicsView::AnchorViewCenter);

  setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
          QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);

  // color scheme
  QColor col;
  setBackgroundBrush(QBrush(gui_settings.get<QColor>("view/bg_col")));

  // make lattice and surface layer
  buildLattice();

  // surface layer active on init
  top_layer = layers.at(1);
}

gui::DesignWidget::~DesignWidget()
{}



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
    QList<QGraphicsItem*> *items = layers.at(n)->getItems();
    for(int i=0; i<items->count(); i++){
      scene->removeItem(items->at(i));
      delete items->at(i);
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
  QList<QGraphicsItem*> *items = lattice->getItems();
  for(int i=0; i<items->count(); i++){
    item = items->at(i);
    scene->addItem(item);
  }

  // add the lattice to the layers
  layers.append(lattice);

  // add in the sb-surface layer
  addLayer("Surface");

  // set surface as top layer
  top_layer = layers.at(1);
}



void gui::DesignWidget::addDB(qreal x, qreal y)
{
  qDebug() << QString("Adding DB at x=%1, y=%2...").arg(QString::number(x), QString::number(y));
  prim::DBDot *db = new prim::DBDot(QPointF(x,y));
  qDebug("dot created...");
  scene->addItem(db);
  qDebug("dot added to scene");
  top_layer->addItem(db);
}

// INTERRUPTS

void gui::DesignWidget::mousePressEvent(QMouseEvent *e)
{
  // set clicked flag and store current mouse position for move behaviour
  if(clicked)
    qWarning("clicked flag was not reset... possible undesired behaviour");
  clicked=true;
  old_mouse_pos = e->pos();

  switch(e->button()){
    case Qt::LeftButton:
      QGraphicsView::mousePressEvent(e);
      break;
    default:
      break;
  }
}


void gui::DesignWidget::mouseMoveEvent(QMouseEvent *e)
{
  QPoint delta_mouse_pos;
  QTransform trans = transform();
  qreal dx, dy;

  if(clicked){
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
        // do nothing if no button pressed
        break;
      default:
        qWarning("No multi-button mouse behaviour implemented");
        break;
    }
  }

}

void gui::DesignWidget::mouseReleaseEvent(QMouseEvent *e)
{
  if(!clicked){
    qWarning("clicked flag was not initialised... doing nothing");
    return;
  }
  clicked=false;

  QGraphicsView::mouseReleaseEvent(e);
  QPointF scene_pos = mapToScene(e->pos());

  if(e->button()==Qt::LeftButton){
    qDebug() << QString("left clicked at: (%1,%2)").arg(QString::number(e->x()), QString::number(e->y()));
  }
  else if(e->button()==Qt::RightButton){
    qDebug() << QString("mouse position: (%1,%2)").arg(QString::number(e->x()), QString::number(e->y()));
    qDebug() << QString("scene position: (%1,%2)").arg(QString::number(scene_pos.x()), QString::number(scene_pos.y()));
  }
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
      wheelPan(e, keymods & Qt::ShiftModifier);
  }
}


void gui::DesignWidget::wheelZoom(QWheelEvent *e, bool boost)
{
  settings::GUISettings gui_settings;
  QTransform trans = transform();

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

void gui::DesignWidget::wheelPan(QWheelEvent *e, bool boost)
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
