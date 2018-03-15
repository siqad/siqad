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
bool prim::Item::select_mode = false;
bool prim::Item::db_gen_mode = false;
bool prim::Item::electrode_mode = false;


// CLASS::Item

void prim::Item::init()
{
  scale_factor = settings::GUISettings::instance()->get<qreal>("view/scale_fact");
}

//prim::Item::Item(ItemType type, prim::Layer *lay, QGraphicsItem *parent)
prim::Item::Item(ItemType type, int lay_id, QGraphicsItem *parent)
  : QGraphicsItem(parent), item_type(type), layer_id(lay_id), hovered(false)
{
  if(scale_factor<0)
    init();
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



QColor prim::Item::getCurrentStateColor(const prim::Item::StateColors &state_colors)
{
  if (select_mode && upSelected()) {
    qDebug() << "Selected";
    qDebug() << QObject::tr("%1, %2, %3")
        .arg(state_colors.selected.red())
        .arg(state_colors.selected.green())
        .arg(state_colors.selected.blue());
    return state_colors.selected;
  } else if (upHovered()) {
    qDebug() << "hovered";
    qDebug() << QObject::tr("%1, %2, %3")
        .arg(state_colors.hovered.red())
        .arg(state_colors.hovered.green())
        .arg(state_colors.hovered.blue());
    return state_colors.hovered;
  } else {
    qDebug() << "normal";
    qDebug() << QObject::tr("%1, %2, %3")
        .arg(state_colors.normal.red())
        .arg(state_colors.normal.green())
        .arg(state_colors.normal.blue());
    return state_colors.normal;
  }
}


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
