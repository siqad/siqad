// @file:     aggregate.cc
// @author:   Jake
// @created:  2017.05.16
// @editted:  2017.06.07  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Base class for Aggregate Item type

#include "aggregate.h"

QColor prim::Aggregate::edge_col;

prim::Aggregate::Aggregate(prim::Layer *layer, QStack<Item*> &items, QGraphicsItem *parent)
  : prim::Item(prim::Item::Aggregate, layer, parent), items(items)
{
  // set all given items as children
  for(prim::Item *item : items){
    item->setParentItem(this);
    item->setFlag(QGraphicsItem::ItemIsSelectable, false);
  }

  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setSelected(true);

  setAcceptHoverEvents(true);

  // set up static edge color if invalid
  if(!edge_col.isValid())
    prepareStatics();
}

prim::Aggregate::~Aggregate()
{
  // migrate children to Aggregate's parent Item
  // for(prim::Item *item : items){
  //   //item->setParentItem(parentItem());
  //   // item->setFlag(QGraphicsItem::ItemIsSelectable, true);
  //   // item->setSelected(true);
  // }
}

QRectF prim::Aggregate::boundingRect() const
{
  // smallest bounding box around all children items
  bool unset = true;
  qreal xmin=-1, ymin=-1, xmax=-1, ymax=-1;
  for(QGraphicsItem *item : childItems()){
    QRectF rect = item->boundingRect();
    QPointF pos = item->pos();
    if(unset){
      unset=false;
      xmin = pos.x()+rect.left();
      ymin = pos.y()+rect.top();
      xmax = pos.x()+rect.right();
      ymax = pos.y()+rect.bottom();
    }
    else{
      xmin = qMin(xmin, pos.x()+rect.left());
      ymin = qMin(ymin, pos.y()+rect.top());
      xmax = qMax(xmax, pos.x()+rect.right());
      ymax = qMax(ymax, pos.y()+rect.bottom());
    }
  }

  qreal width = xmax-xmin;
  qreal height = ymax-ymin;
  return QRectF(.5*(xmax+xmin-width), .5*(ymax+ymin-height), width, height);
}

void prim::Aggregate::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  // Scene will handle drawing the children, just draw the bounding box
  if(upSelected()){
    QRectF rect = boundingRect();

    painter->setPen(QPen(edge_col));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect);
  }
}


prim::Item *prim::Aggregate::deepCopy() const
{
  QStack<prim::Item*> cp_items;
  for(prim::Item *item : items)
    cp_items.append(item->deepCopy());
  return new prim::Aggregate(layer, cp_items, 0);
}




void prim::Aggregate::prepareStatics()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  edge_col = gui_settings->get<QColor>("aggregate/edge_col");
}


void prim::Aggregate::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  // QGraphicsItem order precedence will trigger the children before the Aggregate
  // The following will only be triggered if the children pass the event up.
  qDebug() << QObject::tr("Aggregate has seen the mousePressEvent");
  prim::Item::mousePressEvent(e);
}

void prim::Aggregate::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
  qDebug() << QObject::tr("Aggregate has seen the hoverEnterEvent");
  //prim::Item::hoverEnterEvent(e);
  // TODO set a flag that indicates mouse has entered, hence aggregate border should be highlighted
}

void prim::Aggregate::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
  qDebug() << QObject::tr("Aggregate has seen the hoverLeaveEvent");
  // TODO unset the aggregate highlight flag
}
