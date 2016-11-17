#include "layer.h"
#include <QDebug>

uint prim::Layer::layer_cnt = 0;

prim::Layer::Layer()
{
  // initialise control parameters
  visible=true;
  active=true;
  name = QString("Layer %1").arg(layer_cnt++);
}

prim::Layer::Layer(const QString &nm)
{
  // initialise control parameters
  visible=true;
  active=true;
  name = nm;
  layer_cnt++;
}

prim::Layer::~Layer()
{
    // items in layer should also be owned by some QGraphicsView
    // no need to explicitly free
}



void prim::Layer::addItem(QGraphicsItem *item)
{
  // add item if not already in items list
  if(!items.contains(item))
    items.append(item);
  else
    qDebug("item already contained in layer...");
}


void prim::Layer::removeItem(QGraphicsItem *item)
{
  bool found = items.removeOne(item);
  if(!found)
    qDebug("item not found in layer...");
}


void prim::Layer::setVisible(bool vis)
{
  if(vis!=visible){
    visible=vis;
    for(QGraphicsItem *item : items){
      item->setVisible(vis);
    }
  }
}


bool prim::Layer::isVisible()
{
  return visible;
}

void prim::Layer::setActive(bool act)
{
  if(act!=active){
    active=act;
    for(QGraphicsItem *item : items){
      item->setActive(act);
    }
  }
}


bool prim::Layer::isActive()
{
  return active;
}


const QString prim::Layer::getName()
{
  return name;
}
