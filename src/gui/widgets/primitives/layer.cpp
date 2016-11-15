#include "layer.h"
#include <QDebug>


prim::Layer::Layer()
{
  // initialise control parameters
  visible=true;
  active=true;
}

prim::Layer::~Layer()
{
    // items in layer should also be owned by some QGraphicsView
    // no need to explicitly free
}



void prim::Layer::add_item(QGraphicsItem *item)
{
  // add item if not already in items list
  if(!items.contains(item))
    items->append(item);
  else
    qDebug("item already contained in layer...");
}


void prim::Layer::remove_item(QGraphicsItem *item)
{
  bool found = items->removeOne(item);
  if(!found)
    qDebug("item not found in layer...");
}


void prim::Layer::set_visible(bool vis)
{
  if(vis!=visible){
    visible=vis;
    for(QGraphicsItem *item : items){
      item->setVisible(vis);
    }
  }
}


bool prim::Layer::is_visible()
{
  return visible;
}

void prim::Layer::set_active(bool act)
{
  if(act!=active){
    active=act;
    for(QGraphicsItem *item : items){
      item->setActive(act);
    }
  }
}


bool prim::Layer::is_active()
{
  return active;
}
