// @file:     layer.cc
// @author:   Jake
// @created:  2016.11.15
// @editted:  2017.05.10  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Layer implementations

#include "layer.h"


// statics
uint prim::Layer::layer_count = 0;


prim::Layer::Layer(const QString &nm, QObject *parent)
  : QObject(parent), visible(true), active(false)
{
  name = nm.isEmpty() ? nm : QString("Layer %1").arg(layer_count++);
}


// NOTE: in future, it might be worth keeping the items in a binary tree, sorted
// by pointer address (which should not be modifiable).

void prim::Layer::addItem(prim::Item *item, int index)
{
  if(!items.contains(item)){
    if(index <= items.size()){
      items.insert(index < 0 ? items.size() : index, item);

      // set item flags to agree with layer
      item->setActive(active);
      item->setVisible(visible);
    }
    else
      qCritical() << tr("Layer item index invalid");
  }
  else
    qDebug() << tr("item aleady contained in layer...");
}



bool prim::Layer::removeItem(prim::Item *item)
{
  bool found = items.removeOne(item);
  if(!found)
    qDebug() << tr("item not found in layer...");
  return found;
}



void prim::Layer::setVisible(bool vis)
{
  if(vis!=visible){
    visible = vis;
    for(prim::Item *item : items)
      item->setVisible(vis);
  }
}


void prim::Layer::setActive(bool act)
{
  if(act!=active){
    active=act;
    for(prim::Item *item : items)
      item->setActive(act);
  }
}
