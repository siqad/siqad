// @file:     item.cc
// @author:   Jake
// @created:  2016.11.21
// @editted:  2017.05.09  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Definitions for Item classes

#include <QApplication>

#include "item.h"


qreal prim::Item::scale_factor = -1;


// CLASS::Item

prim::Item::Item(ItemType type, prim::Layer *lay, QGraphicsItem *parent)
  : QGraphicsItem(parent), item_type(type), layer(lay)
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  if(scale_factor<0)
    scale_factor = gui_settings->get<qreal>("view/scale_fact");
}



bool prim::Item::upSelected()
{
  prim::Item *parent = (prim::Item*) parentItem();
  return parent==0 ? isSelected() : parent->upSelected();
}


// current functionality:
// items that are selected emit a signal when left clicked if control not pressed
void prim::Item::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  Qt::KeyboardModifiers keymods = QApplication::keyboardModifiers();

  switch(e->buttons()){
    case Qt::LeftButton:
      if(keymods & Qt::ControlModifier)
        e->setAccepted(false);
      else if(upSelected())
        prim::Emitter::instance()->selectClicked(this);
      break;
    default:
      break;
  }

  qDebug() << QObject::tr("Item clicked: %1 :: (%2 , %3)").arg((size_t)this).arg(x()).arg(y());
}
