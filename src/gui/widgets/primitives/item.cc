// @file:     item.cc
// @author:   Jake
// @created:  2016.11.21
// @editted:  2017.06.07  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Definitions for Item classes

#include <QApplication>

#include "item.h"


qreal prim::Item::scale_factor = -1;

gui::DisplayMode prim::Item::display_mode;
gui::ToolType prim::Item::tool_type;


// CLASS::Item

void prim::Item::init()
{
  scale_factor = settings::GUISettings::instance()->get<qreal>("view/scale_fact");
}

prim::Item::Item(ItemType type, int lay_id, QGraphicsItem *parent)
  : QGraphicsItem(parent), item_type(type), layer_id(lay_id), hovered(false)
{
  if(scale_factor<0)
    init();
}

QString prim::Item::getQStringItemType()
{
  switch (item_type) {
    case prim::Item::Aggregate: return "Aggregate";
    case prim::Item::DBDot: return "DBDot";
    case prim::Item::LatticeDot: return "LatticeDot";
    case prim::Item::Ghost: return "Ghost";
    case prim::Item::GhostDot: return "GhostDot";
    case prim::Item::Electrode: return "Electrode";
    case prim::Item::GhostBox: return "GhostBox";
    case prim::Item::AFMArea: return "AFMArea";
    case prim::Item::AFMPath: return "AFMPath";
    case prim::Item::AFMNode: return "AFMNode";
    case prim::Item::AFMSeg: return "AFMSeg";
    case prim::Item::PotPlot: return "PotPlot";
    case prim::Item::ResizeFrame: return "ResizeFrame";
    case prim::Item::ResizeHandle: return "ResizeHandle";
    default: return "Erroneous Item";
  }
}

prim::Item::ItemType prim::Item::getEnumItemType(QString type)
{
  if (type == "Aggregate") return prim::Item::Aggregate;
  else if (type == "DBDot") return prim::Item::DBDot;
  else if (type == "LatticeDot") return prim::Item::LatticeDot;
  else if (type == "Ghost") return prim::Item::Ghost;
  else if (type == "GhostDot") return prim::Item::GhostDot;
  else if (type == "Electrode") return prim::Item::Electrode;
  else if (type == "GhostBox") return prim::Item::GhostBox;
  else if (type == "AFMArea") return prim::Item::AFMArea;
  else if (type == "AFMPath") return prim::Item::AFMPath;
  else if (type == "AFMNode") return prim::Item::AFMNode;
  else if (type == "AFMSeg") return prim::Item::AFMSeg;
  else if (type == "PotPlot") return prim::Item::PotPlot;
  else if (type == "ResizeFrame") return prim::Item::ResizeFrame;
  else if (type == "ResizeHandle") return prim::Item::ResizeHandle;
}

bool prim::Item::upSelected()
{
  prim::Item *parent = static_cast<prim::Item*>(parentItem());
  return parent==0 ? isSelected() : parent->upSelected();
}

bool prim::Item::upHovered()
{
  prim::Item *parent = static_cast<prim::Item*>(parentItem());
  return parent==0 ? isHovered() : parent->upHovered();
}

gui::PropertyMap prim::Item::properties()
{
  gui::PropertyMap all_props;

  // assume that there aren't local properties if there's no class property
  if (!classPropertyMap()){
    return gui::PropertyMap();
  }
  for (const QString &key : classPropertyMap()->keys()) {
    if (local_props.contains(key)) {
      // use the local value + default attributes
      all_props[key] = gui::Property(local_props.value(key), classPropertyMap()->value(key));
    } else {
      // use the default value + attributes
      all_props[key] = classPropertyMap()->value(key);
    }
  }

  return all_props;
}

void prim::Item::propMapFromXml(QXmlStreamReader *rs)
{
  gui::PropertyMap temp_prop = properties();
  temp_prop.readValsFromXML(rs);
  for(auto key : temp_prop.keys()){
    qDebug() << QObject::tr("key: %1, val: %2").arg(key).arg(temp_prop.value(key).value.toString());
    setProperty(key, temp_prop.value(key).value);
  }
}


gui::PropertyMap prim::Item::properties() const
{
  gui::PropertyMap all_props;

  // assume that there aren't local properties if there's no class property
  if (!classPropertyMap())
    return gui::PropertyMap();

  for (const QString &key : classPropertyMap()->keys()) {
    if (local_props.contains(key)) {
      // use the local value + default attributes
      all_props[key] = gui::Property(local_props.value(key), classPropertyMap()->value(key));
    } else {
      // use the default value + attributes
      all_props[key] = classPropertyMap()->value(key);
    }
  }

  return all_props;
}

gui::Property prim::Item::getProperty(const QString &key)
{
  if (local_props.contains(key)) {
    if (classPropertyMap()) {
      return gui::Property(local_props.value(key), classPropertyMap()->value(key));
    } else {
      return gui::Property(local_props.value(key));
    }
  }

  if (classPropertyMap() && classPropertyMap()->contains(key))
    return classPropertyMap()->value(key);

  // property not found
  qCritical() << QObject::tr("Can't find property with key %1 in item type %2")
      .arg(key).arg(item_type);
  return gui::Property();
}



QColor prim::Item::getCurrentStateColor(const prim::Item::StateColors &state_colors)
{
  if (display_mode == gui::ScreenshotMode)
    return state_colors.publish; // high contrast when taking a screenshot

  if (tool_type == gui::SelectTool && upSelected())
    return state_colors.selected;

  if (upHovered())
    return state_colors.hovered;

  return state_colors.normal;
}


// void prim::Item::showProps()
// {
//   prim::Emitter::instance()->sig_showProperty(this);
// }

// current functionality:
// items that are selected emit a signal when left clicked if control not pressed
void prim::Item::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  Qt::KeyboardModifiers keymods = QApplication::keyboardModifiers();
  switch(e->buttons()){
    case Qt::LeftButton:
      if(keymods & Qt::ControlModifier)
        e->ignore();
      else if(upSelected()){
        prim::Emitter::instance()->selectClicked(this);
      }
      else{
        if(parentItem()==0)
          QGraphicsItem::mousePressEvent(e);
        else
          e->ignore();
      }
      break;
    default:
      e->ignore();
      break;
  }

  qDebug() << QObject::tr("Item clicked: %1 :: (%2 , %3)").arg((size_t)this).arg(x()).arg(y());
  qDebug() << QObject::tr("Selectability: %1").arg(flags() & QGraphicsItem::ItemIsSelectable);
}
