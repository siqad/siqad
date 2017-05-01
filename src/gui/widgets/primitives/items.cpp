// @file:     items.cpp
// @author:   Jake
// @created:  2016.11.21
// @editted:  2017.05.01  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Definitions for Item classes

#include "items.h"

#include <QApplication>


// MyItem CLASS

void prim::MyItem::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  Qt::KeyboardModifiers keymods = QApplication::keyboardModifiers();

  switch(e->button()){
    case Qt::LeftButton:
      if(keymods & Qt::ControlModifier)
        e->setAccepted(false);
      else if(isSelected())
        prim::Emitter::instance()->selectClicked(this);
      break;
    default:
      break;
  }
}



// Aggregate CLASS

void prim::Aggregate::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  Qt::KeyboardModifiers keymods = QApplication::keyboardModifiers();

  switch(e->button()){
    case Qt::LeftButton:
      if(keymods & Qt::ControlModifier)
        e->setAccepted(false);
      else if(isSelected())
        prim::Emitter::instance()->selectClicked(this);
      break;
    default:
      break;
  }
}
