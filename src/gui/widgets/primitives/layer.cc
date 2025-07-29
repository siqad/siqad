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
#include "dblayer.h"
#include "lattice.h"


// statics
uint prim::Layer::layer_count = 0;


prim::Layer::Layer(const QString &nm, const LayerType &cnt_type, 
    const LayerRole &role, float z_offset, float z_height, int lay_id, 
    QObject *parent)
  : QObject(parent), layer_id(lay_id), zoffset(z_offset), zheight(z_height),
        content_type(cnt_type), layer_role(role), visible(true), 
        active(false)
{
  name = nm.isEmpty() ? QString("Layer %1").arg(layer_count++) : nm;
}


prim::Layer::Layer(QXmlStreamReader *rs, int lay_id)
  : layer_id(lay_id), visible(true), active(false)
{
  QString nm;
  LayerType type = NoType;
  layer_role = LayerRole::Design;

  while (rs->readNextStartElement()) {
    QString elem_name = rs->name().toString();
    if (elem_name == "name") {
      nm = rs->readElementText();
    } else if (elem_name == "type") {
      type = static_cast<LayerType>(
          QMetaEnum::fromType<LayerType>().keyToValue(
            rs->readElementText().toStdString().c_str()));
    } else if (elem_name == "role") {
      // added in SiQAD v0.2.2
      layer_role = static_cast<LayerRole>(
          QMetaEnum::fromType<LayerRole>().keyToValue(
            rs->readElementText().toStdString().c_str()));
    } else if (elem_name == "zoffset") {
      zoffset = rs->readElementText().toFloat();
    } else if (elem_name == "zheight") {
      zheight = rs->readElementText().toFloat();
    } else if (elem_name == "visible") {
      visible = (rs->readElementText() == "1") ? true : false;
    } else if (elem_name == "active") {
      active = (rs->readElementText() == "1") ? true : false;
    } else {
      qDebug() << tr("Layer: invalid element encountered on line %1 - %2")
          .arg(rs->lineNumber()).arg(elem_name);
      rs->skipCurrentElement();
    }
  }

  // make layer object using loaded information
  name = nm.isEmpty() ? nm : QString("Layer %1").arg(layer_count++);
}


prim::Layer::~Layer()
{
  while (!items.isEmpty()) {
    prim::Item *item = items.pop();
    delete item;
  }
}


void prim::Layer::resetLayers()
{
  layer_count = 0;
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
  emit sig_visibilityChanged(vis);
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
  if (layer_role == LayerRole::Result) {
    qDebug() << tr("Skipping layer %1: %2 as the role is Simulation Result.")
      .arg(layer_id).arg(name);
    return;
  }

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
  ws->writeTextElement("role", roleString()); // added in SiQAD v0.2.2
  ws->writeTextElement("zoffset", str.setNum(zOffset(), fmt, fp));
  ws->writeTextElement("zheight", str.setNum(zHeight(), fmt, fp));
  ws->writeTextElement("visible", QString::number(isVisible()));
  ws->writeTextElement("active", QString::number(isActive()));
}

void prim::Layer::saveItems(QXmlStreamWriter *ws, gui::DesignInclusionArea inclusion_area) const
{
  if (layer_role == LayerRole::Result) {
    qDebug() << tr("Skipping layer %1: %2 as the role is Simulation Result.")
      .arg(layer_id).arg(name);
    return;
  }

  ws->writeComment(getName());

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

void prim::Layer::loadItems(QXmlStreamReader *rs, QGraphicsScene *scene)
{
  qDebug() << QObject::tr("Loading layer items for %1").arg(name);
  // create items according to hierarchy
  while(rs->readNextStartElement()) {
    QString elem_name = rs->name().toString();
    if (elem_name == "dbdot") {
      //rs->readNext();
      prim::DBDot *dbdot = new prim::DBDot(rs, scene, layer_id);
      addItem(dbdot);
      prim::Emitter::instance()->addItemToScene(dbdot);
      static_cast<prim::DBLayer*>(this)->getLattice()->setOccupied(dbdot->latticeCoord(), dbdot);
      prim::LatticeCoord lc = dbdot->latticeCoord();
      prim::Emitter::instance()->sig_moveDBToLatticeCoord(dbdot, lc.n, lc.m, lc.l);
    } else if (elem_name == "aggregate") {
      // TODO pass a blank list to Aggregate 
      QList<prim::Item*> new_items;
      addItem(new prim::Aggregate(rs, scene, new_items, layer_id));
      for (prim::Item *item : new_items) {
        if (item->item_type == prim::Item::DBDot) {
          prim::DBDot *dbdot = static_cast<prim::DBDot*>(item);
          static_cast<prim::DBLayer*>(this)->getLattice()->setOccupied(dbdot->latticeCoord(), dbdot);
          prim::LatticeCoord lc = dbdot->latticeCoord();
          prim::Emitter::instance()->sig_moveDBToLatticeCoord(dbdot, lc.n, lc.m, lc.l);
        }
      }
      new_items.clear();
    } else if (elem_name == "electrode") {
      rs->readNext();
      addItem(new prim::Electrode(rs, scene, layer_id));
    } else {
      qDebug() << QObject::tr("Layer load item: invalid element encountered on line %1 - %2").arg(rs->lineNumber()).arg(elem_name);
      rs->skipCurrentElement();
    }
  }

  // show error if any
  if(rs->hasError()){
    qCritical() << QObject::tr("XML error: ") << rs->errorString().data();
  }
}
