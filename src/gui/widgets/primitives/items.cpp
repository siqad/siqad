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
      break;
    default:
      break;
  }
}
