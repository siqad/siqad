// @file:     aggregate.cc
// @author:   Jake
// @created:  2017.05.16
// @editted:  2017.05.16  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Base class for Aggregate Item type

#include "aggregate.h"

QColor prim::Aggregate::edge_col;

prim::Aggregate::Aggregate(prim::Layer *layer, QList<Item*> items, QGraphicsItem *parent)
  : prim::Item(prim::Item::Aggregate, layer, parent), items(items)
{
  // set all given items as children
  for(prim::Item *item : items)
    item->setParentItem(this);

  // set up static edge color if invalid
  if(!edge_col.isValid())
    prepareStatics();
}

prim::Aggregate::~Aggregate()
{
  // migrate children to Aggregate's parent Item
  for(prim::Item *item : items)
    item->setParentItem(parentItem());
}

QRectF prim::Aggregate::boundingRect() const
{
  // smallest bounding box around all children items
  qreal xmin=-1, xmax=-1, ymin=-1, ymax=-1;
  for(QGraphicsItem *item : childItems()){
    QRectF rect = item->boundingRect();
    xmin = xmin<0 ? rect.left() : qMin(xmin, rect.left());
    xmax = qMax(xmax, rect.right());
    ymin = ymin<0 ? rect.top() : qMin(ymin, rect.top());
    ymax = qMax(ymax, rect.bottom());
  }

  qreal width = xmax-xmin;
  qreal height = ymax-ymin;
  return QRectF(-.5*width, -.5*height, width, height);
}

void prim::Aggregate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
  // draw each child object

  for(QGraphicsItem *item : childItems())
    item->paint(painter, option);

  // draw bounding box
  if(isSelected()){
    QRectF rect = boundingRect();

    painter->setPen(QPen(edge_col));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect);
  }
}


void prim::Aggregate::prepareStatics()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  edge_col = gui_settings->get<QColor>("aggregate/edge_col");
}


void prim::Aggregate::mousePressEvent(QGraphicsSceneMouseEvent *)
{
  qDebug() << QObject::tr("Aggregate has seen the mousePressEvent");
  // do nothing,
  // ignore();
}
