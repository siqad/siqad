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
}


// Save to XML
void AFMSeg::saveItems(QXmlStreamWriter *ws) const
{

}


// Graphics
QRectF AFMSeg::boundingRect() const
{

}

void AFMSeg::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{

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
