// @file:     aggregate.cc
// @author:   Jake
// @created:  2017.05.16
// @editted:  2017.06.07  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Base class for Aggregate Item type

#include "aggregate.h"
#include "dbdot.h"
#include "hull/hulls.h"

qreal prim::Aggregate::edge_width;
QColor prim::Aggregate::edge_col;
QColor prim::Aggregate::edge_col_hovered;

prim::Aggregate::Aggregate(int lay_id, QStack<Item*> &items, QGraphicsItem *parent)
  : prim::Item(prim::Item::Aggregate, lay_id, parent), items(items)
{
  initAggregate(items, parent);
}

prim::Aggregate::Aggregate(QXmlStreamReader *stream, QGraphicsScene *scene, int lay_id)
  : prim::Item(prim::Item::Aggregate)
{
  //qDebug() << QObject::tr("Aggregate: constructing aggregate from XML");
  QStack<Item*> ld_children;

  // read from XML stream (children will be created recursively, add those children to stack)
  while(!stream->atEnd()){
    if(stream->isStartElement()){
      if(stream->name() == "dbdot"){
        //stream->readNext();
        ld_children.push(new prim::DBDot(stream, scene, lay_id));
      }
      else if(stream->name() == "aggregate"){
        stream->readNext();
        ld_children.push(new prim::Aggregate(stream, scene, lay_id));
      }
      else{
        qDebug() << QObject::tr("Aggregate: invalid element encountered on line %1 - %2").arg(stream->lineNumber()).arg(stream->name().toString());
        stream->readNext();
      }
    }
    else if(stream->isEndElement()){
      // break out of stream if the end of this element has been reached
      if(stream->name() == "aggregate"){
        stream->readNext();
        break;
      }
      stream->readNext();
    }
    else {
      stream->readNext();
    }
  }

  // show error if any
  if(stream->hasError()){
    qCritical() << QObject::tr("XML error: ") << stream->errorString().data();
  }

  // fill in aggregate properties
  setLayerID(lay_id);
  items = ld_children;
  initAggregate(ld_children);

  scene->addItem(this);
}

void prim::Aggregate::initAggregate(QStack<Item*> &items, QGraphicsItem *parent)
{
  // set parent
  setParentItem(parent);

  // set all given items as children
  addChildren(items);

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

void prim::Aggregate::addChildren(QStack<Item*> &items)
{
  // set all given items as children
  for(prim::Item *item : items){
    item->setParentItem(this);
    item->setFlag(QGraphicsItem::ItemIsSelectable, false);

    // count DBs
    if (item->item_type == prim::Item::DBDot)
      db_count++;
    else if (item->item_type == prim::Item::Aggregate)
      db_count += static_cast<prim::Aggregate*>(item)->dbCount();
  }
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

  qreal width = xmax-xmin+edge_width;
  qreal height = ymax-ymin+edge_width;
  return QRectF(.5*(xmax+xmin-width), .5*(ymax+ymin-height), width, height);
}

void prim::Aggregate::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  QPainterPath path = shape();

  // resize the rectangle to omit the edge width
  // QRectF rect = boundingRect();
  // QSizeF size(rect.size() - QSizeF(edge_width, edge_width));
  // QPointF center = rect.center();
  // rect.setSize(size);
  // rect.moveCenter(center);

  // Scene will handle drawing the children, just draw the bounding box
  if(tool_type == gui::SelectTool && upSelected()){
    painter->setPen(QPen(edge_col, edge_width));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path);
  }
  else if(upHovered()){
    painter->setPen(QPen(edge_col_hovered, edge_width));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path);
  }
}


QPainterPath prim::Aggregate::shape() const
{
  QPainterPath path;

  hull::ConvexHull hull(items);
  hull.solve();
  path.addPolygon(hull.getPolygon());
  path.closeSubpath();

  return path;
}


prim::Item *prim::Aggregate::deepCopy() const
{
  QStack<prim::Item*> cp_items;
  for(prim::Item *item : items)
    cp_items.append(item->deepCopy());
  return new prim::Aggregate(layer_id, cp_items, 0);
}


void prim::Aggregate::saveItems(QXmlStreamWriter *stream) const {
  // write open tag
  stream->writeStartElement("aggregate");

  // write child items
  for(prim::Item *item : items){
    item->saveItems(stream);
  }

  // write close tag
  stream->writeEndElement();
}


void prim::Aggregate::prepareStatics()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  edge_width = gui_settings->get<qreal>("aggregate/edge_width") * prim::Item::scale_factor;
  edge_col = gui_settings->get<QColor>("aggregate/edge_col");
  edge_col_hovered = gui_settings->get<QColor>("aggregate/edge_col_hovered");
}


void prim::Aggregate::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  // QGraphicsItem order precedence will trigger the children before the Aggregate
  // The following will only be triggered if the children pass the event up.

  if(parentItem() != 0)
    e->ignore();
  else
    prim::Item::mousePressEvent(e);
}

void prim::Aggregate::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
  //qDebug() << QObject::tr("Aggregate has seen the hoverEnterEvent");
  setHovered(true);
  update();
}

void prim::Aggregate::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
  //qDebug() << QObject::tr("Aggregate has seen the hoverLeaveEvent");
  setHovered(false);
  update();
}
