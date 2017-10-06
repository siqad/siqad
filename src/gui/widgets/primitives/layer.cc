// @file:     layer.cc
// @author:   Jake
// @created:  2016.11.15
// @editted:  2017.06.07  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Layer implementations

#include "layer.h"
#include "aggregate.h"
#include "dbdot.h"


// statics
uint prim::Layer::layer_count = 0;


prim::Layer::Layer(const QString &nm, const QString &cnt_type, int lay_id, QObject *parent)
  : QObject(parent), visible(true), active(false)
{
  layer_id = lay_id;
  name = nm.isEmpty() ? nm : QString("Layer %1").arg(layer_count++);
  content_type = cnt_type;
}


prim::Layer::Layer(QXmlStreamReader *stream)
{
  int lay_id;
  QString name_ld;
  bool visible_ld, active_ld;

  while(!stream->atEnd()){
    if(stream->isStartElement()){
      if(stream->name() == "id")
        lay_id = stream->readElementText().toInt();
      else if(stream->name() == "name")
        name_ld = stream->readElementText();
      else if(stream->name() == "visible")
        visible_ld = (stream->readElementText() == "1")?1:0;
      else if(stream->name() == "active")
        active_ld = (stream->readElementText() == "1")?1:0;
      else
        qDebug() << QObject::tr("Layer: invalid element encountered on line %1 - %2").arg(stream->lineNumber()).arg(stream->name().toString());
      stream->readNext();
    }
    else if(stream->isEndElement()){
      // break out of stream if the end of this element has been reached
      if(stream->name() == "layer_prop"){
        stream->readNext();
        break;
      }
    }
    stream->readNext();
  }

  if(stream->hasError()){
    qCritical() << tr("XML error: ") << stream->errorString().data();
  }

  // make layer object using loaded information
  layer_id = lay_id;
  name = name_ld.isEmpty() ? name_ld : QString("Layer %1").arg(layer_count++);
  setVisible(visible_ld);
  setActive(active_ld);
}


void prim::Layer::resetLayers()
{
  layer_count = 0;
}


void prim::Layer::setLayerIndex(int lay_id){
  layer_id = lay_id;
  for(prim::Item *item : items)
    item->setLayerIndex(lay_id);
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


prim::Item *prim::Layer::takeItem(int ind)
{
  if(ind==-1 && !items.isEmpty())
    return items.takeLast();
  if(ind < 0 || ind >= items.count()){
    qCritical() << tr("Invalid item index...");
    return 0;
  }
  return items.takeAt(ind);
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

void prim::Layer::saveLayer(QXmlStreamWriter *stream) const
{
  stream->writeStartElement("layer_prop");

  stream->writeTextElement("name", name);
  stream->writeTextElement("visible", QString::number(visible));
  stream->writeTextElement("active", QString::number(active));

  stream->writeEndElement();
}

void prim::Layer::saveItems(QXmlStreamWriter *stream) const
{
  stream->writeStartElement("layer");
  stream->writeAttribute("type", content_type);

  for(prim::Item *item : items){
    item->saveItems(stream);
  }

  stream->writeEndElement();
}

void prim::Layer::loadItems(QXmlStreamReader *stream, QGraphicsScene *scene)
{
  qDebug() << QObject::tr("Loading layer items for %1").arg(name);
  // create items according to hierarchy
  while(!stream->atEnd()){
    if(stream->isStartElement()){
      if(stream->name() == "dbdot"){
        stream->readNext();
        addItem(new prim::DBDot(stream, scene));
      }
      else if(stream->name() == "aggregate"){
        stream->readNext();
        addItem(new prim::Aggregate(stream, scene));
      }
      else{
        qDebug() << QObject::tr("Layer: invalid element encountered on line %1 - %2").arg(stream->lineNumber()).arg(stream->name().toString());
        stream->readNext();
      }
    }
    else if(stream->isEndElement()){
      // break out of stream if the end of this element has been reached
      if(stream->name() == "layer"){
        stream->readNext();
        break;
      }
      stream->readNext();
    }
    else
      stream->readNext();
  }
  
  // show error if any
  if(stream->hasError()){
    qCritical() << QObject::tr("XML error: ") << stream->errorString().data();
  }
}
