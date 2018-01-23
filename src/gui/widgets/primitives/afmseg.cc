// @file:     afmseg.cc
// @author:   Samuel
// @created:  2018.01.17
// @editted:  2018.01.17 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Segment in AFM travel path

#include "afmseg.h"

namespace prim{

// TODO static stuff

// constructors
AFMSeg::AFMSeg(int lay_id, prim::AFMNode *orig_node, prim::AFMNode *dest_node)
  : prim::Item(prim::Item::AFMSeg)
{
  initAFMSeg(lay_id, orig_node, dest_node);
}

AFMSeg::AFMSeg(QXmlStreamReader *rs, QGraphicsScene *scene)
  : prim::Item(prim::Item::AFMSeg)
{
  // TODO call initAFMSeg from loaded values
}

void AFMSeg::initAFMSeg(int lay_id, prim::AFMNode *orig_node, prim::AFMNode *dest_node)
{
  layer_id = lay_id;
  setOrigin(orig_node);
  setDestination(dest_node);

  updatePoints();

  // TODO prepareStatics if not valid
}


// Save to XML
void AFMSeg::saveItems(QXmlStreamWriter *ws) const
{

}


// Segment manipulation
void AFMSeg::setOrigin(prim::AFMNode *orig_node)
{
  origin_node = orig_node;
  updatePoints();
}

void AFMSeg::setDestination(prim::AFMNode *dest_node)
{
  destination_node = dest_node;
  updatePoints();
}


// Graphics
void AFMSeg::updatePoints()
{
  prepareGeometryChange();
  origin_loc = originNode()->scenePos();
  destination_loc = destinationNode()->scenePos();
}


QRectF AFMSeg::boundingRect() const
{
  qreal x_min = qMin(origin_loc.x(), destination_loc.x());
  qreal y_min = qMin(origin_loc.y(), destination_loc.y());
  qreal dx = qMax(origin_loc.x(), destination_loc.x()) - x_min;
  qreal dy = qMax(origin_loc.y(), destination_loc.y()) - y_min;
  return QRectF(x_min, y_min, dx, dy);
}

void AFMSeg::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  painter->setPen(Qt::SolidLine);
  painter->setBrush(QColor(255,255,255));
  painter->drawLine(QLineF(originNode()->scenePos(), destinationNode()->scenePos()));
}

Item *AFMSeg::deepCopy() const
{

}


// PRIVATE

void AFMSeg::prepareStatics()
{

}


void AFMSeg::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  // Show path config dialog when selected, with the two connected nodes highlighted
  // TODO
}

void AFMSeg::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
  qDebug() << QObject::tr("AFMSeg has seen the hoverEnterEvent");
  setHovered(true);
  update();
}

void AFMSeg::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
  qDebug() << QObject::tr("AFMSeg has seen the hoverLeaveEvent");
  setHovered(false);
  update();
}


} // end of prim namespace
