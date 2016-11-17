#include "dbdot.h"



prim::DBDot::DBDot(qreal x, qreal y)
 : QGraphicsItem()
{
  setX(x);
  setY(y);
}

prim::DBDot::~DBDot()
{}


QRectF prim::DBDot::boundingRect() const
{
  // outer most edges
  return QRectF(x(), y(), 10, 10);
}


void prim::DBDot::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  QRectF rect = boundingRect();

  QPen pen(Qt::black, 3);
  painter->setPen(pen);
  painter->drawEllipse(rect);
}
