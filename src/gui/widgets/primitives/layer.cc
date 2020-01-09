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
#include "electrode.h"
#include "afmarea.h"
#include "afmpath.h"


prim::Layer::Layer(const QString &nm, LayerType cnt_type, float z_offset,
    float z_height, int lay_id, QObject *parent)
  : QObject(parent), layer_id(lay_id), visible(true), active(false)
{
  name = nm.isEmpty() ? QString("Layer %1").arg(lay_id) : nm;
  props.zoffset = z_offset;
  props.zheight = z_height;
  props.content_type = cnt_type;
}


prim::Layer::Layer(QXmlStreamReader *rs, int lay_id)
  : layer_id(lay_id), visible(true), active(false)
{
  QString nm;
  prim::Layer::LayerType type = prim::Layer::NoType;

  while (rs->readNextStartElement()) {
    if (rs->name() == "name") {
      nm = rs->readElementText();
    } else if (rs->name() == "type") {
      type = static_cast<LayerType>(
          QMetaEnum::fromType<LayerType>().keyToValue(
          rs->readElementText().toStdString().c_str()));
    } else if (rs->name() == "zoffset") {
      props.zoffset = rs->readElementText().toFloat();
    } else if (rs->name() == "zheight") {
      props.zheight = rs->readElementText().toFloat();
    } else if (rs->name() == "visible") {
      visible = (rs->readElementText() == "1") ? true : false;
    } else if (rs->name() == "active") {
      active = (rs->readElementText() == "1") ? true : false;
    } else {
      qDebug() << tr("Layer: invalid element encountered on line %1 - %2")
          .arg(rs->lineNumber()).arg(rs->name().toString());
      rs->skipCurrentElement();
    }
  }

  // make layer object using loaded information
  name = nm.isEmpty() ? nm : QString("Layer %1").arg(lay_id);
}


prim::Layer::~Layer()
{
  while (!items.isEmpty()) {
    prim::Item *item = items.pop();
    delete item;
    //prim::Emitter::instance()->removeItemFromScene(item);
  }
}


void prim::Layer::resetLayers()
{
  //layer_count = 0;
}


void prim::Layer::setLayerID(int lay_id){
  layer_id = lay_id;
  for(prim::Item *item : items)
    item->setLayerID(lay_id);
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
    } else {
      qCritical() << tr("Layer item index invalid");
    }
  } else {
    qDebug() << tr("item aleady contained in layer...");
  }
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

void prim::Layer::saveLayer(QXmlStreamWriter *ws) const
{
  ws->writeStartElement("layer_prop");
  saveLayerProperties(ws);
  ws->writeEndElement();
}

void prim::Layer::saveLayerProperties(QXmlStreamWriter *ws) const
{
  int fp = settings::AppSettings::instance()->get<int>("float_prc");
  QString fmt_st = settings::AppSettings::instance()->get<QString>("float_fmt");
  char fmt = fmt_st.at(0).toLatin1();
  QString str;

  ws->writeTextElement("name", getName());
  ws->writeTextElement("type", contentTypeString());
  ws->writeTextElement("zoffset", str.setNum(zOffset(), fmt, fp));
  ws->writeTextElement("zheight", str.setNum(zHeight(), fmt, fp));
  ws->writeTextElement("visible", QString::number(isVisible()));
  ws->writeTextElement("active", QString::number(isActive()));
}

void prim::Layer::saveItems(QXmlStreamWriter *ws, gui::DesignInclusionArea inclusion_area) const
{
  ws->writeStartElement("layer");
  ws->writeAttribute("type", contentTypeString());

  for(prim::Item *item : items){
    switch (inclusion_area) {
      case gui::IncludeSelectedItems:
        // save only selected items
        if (item->isSelected())
          item->saveItems(ws);
        break;
      // TODO implement IncludeAreaOfInterest
      case gui::IncludeEntireDesign:
      default:
        // save entire design
        item->saveItems(ws);
    }
  }

  ws->writeEndElement();
}

void prim::Layer::loadItems(QXmlStreamReader *ws, QGraphicsScene *scene)
{
  qDebug() << QObject::tr("Loading layer items for %1").arg(name);
  // create items according to hierarchy
  while (!ws->atEnd()) {
    if (ws->isStartElement()) {
      if (ws->name() == "dbdot") {
        ws->readNext();
        addItem(new prim::DBDot(ws, scene, layer_id));
      } else if (ws->name() == "aggregate") {
        ws->readNext();
        addItem(new prim::Aggregate(ws, scene, layer_id));
      } else if (ws->name() == "electrode") {
        ws->readNext();
        addItem(new prim::Electrode(ws, scene, layer_id));
      } else if (ws->name() == "afmarea") {
        ws->readNext();
        addItem(new prim::AFMArea(ws, scene, layer_id));
      } else if (ws->name() == "afmpath") {
        ws->readNext();
        addItem(new prim::AFMPath(ws, scene, layer_id));
      } else {
        qDebug() << QObject::tr("Layer load item: invalid element encountered on line %1 - %2").arg(ws->lineNumber()).arg(ws->name().toString());
        ws->readNext();
      }
    } else if (ws->isEndElement()) {
      // break out of stream if the end of this element has been reached
      if (ws->name() == "layer") {
        ws->readNext();
        break;
      }
      ws->readNext();
    } else {
      ws->readNext();
    }
  }

  // show error if any
  if(ws->hasError()){
    qCritical() << QObject::tr("XML error: ") << ws->errorString().data();
  }
}
