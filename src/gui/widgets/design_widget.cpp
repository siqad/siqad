#include "design_widget.h"

#include <QDebug>
#include <QString>

#include <QVariant>
#include <QColor>
#include <QBrush>

#include "src/settings/settings.h"

gui::DesignWidget::DesignWidget(QWidget *parent)
  :QGraphicsView(parent)
{
  settings::GUISettings gui_settings;

  scene = new QGraphicsScene(this);
  setScene(scene);

  // setup flags
  clicked = false;
  zoom = 1.;

  // set view behaviour
  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  setResizeAnchor(QGraphicsView::AnchorUnderMouse);

  // color scheme
  QColor col;
  setBackgroundBrush(QBrush(gui_settings.get<QColor>("view/bg_col")));

  addLayer("Surface");
  top_layer = layers.at(0);
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
      layers.removeAt(i);
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
  else
    layers.removeAt(n);
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



void gui::DesignWidget::addDB(qreal x, qreal y)
{
  qDebug() << QString("Adding DB at x=%1, y=%2...").arg(QString::number(x), QString::number(y));
  prim::DBDot *db = new prim::DBDot(x, y);
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

  if(clicked){
    switch(e->buttons()){
      case Qt::LeftButton:
        QGraphicsView::mouseMoveEvent(e);
        break;
      case Qt::MidButton:
        delta_mouse_pos = e->pos()-old_mouse_pos;
        translate(-delta_mouse_pos.x(), -delta_mouse_pos.y());
        old_mouse_pos = e->pos();
        qDebug() << QString("delta: (%1,%2)").arg(QString::number(delta_mouse_pos.x()), QString::number(delta_mouse_pos.y()));
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
    addDB(e->x(), e->y());
  }
  else if(e->button()==Qt::RightButton){
    qDebug() << QString("mouse position: (%1,%2)").arg(QString::number(e->x()), QString::number(e->y()));
    qDebug() << QString("scene position: (%1,%2)").arg(QString::number(scene_pos.x()), QString::number(scene_pos.y()));
  }
}
