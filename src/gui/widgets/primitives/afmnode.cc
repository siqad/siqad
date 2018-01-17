// @file:     afmnode.cc
// @author:   Samuel
// @created:  2018.01.17
// @editted:  2018.01.17 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Node in AFM travel path

#include "afmnode.h"

namespace prim{

// TODO static stuff

// constructors
AFMNode::AFMNode(int lay_id, QPointF physloc, float z_offset)
  : prim::Item(prim::Item::AFMNode)
{
  initAFMNode(lay_id, physloc, z_offset);
}

AFMNode::AFMNode(QXmlStreamReader *rs, QGraphicsScene *scene)
  : prim::Item(prim::Item::AFMNode)
{

  // TODO call initAFMNode with read values
}

void AFMNode::initAFMNode(int lay_id, QPointF physloc, float z_offset)
{
  layer_id = lay_id;
  setZOffset(z_offset);
  setPhysLoc(physloc);
}


// Save to XML
void AFMNode::saveItems(QXmlStreamWriter *ws) const
{
  // TODO
}


// Node manipulation
void AFMNode::setZOffset(float z_offset)
{
  // Check whether the z_offset is within bounds of the layer. If not, expand
  // the layer's Z-Offset or Z-Height to make it fit, or alert the user if the
  // desired z_offset is not valid, e.g. runs into the surface.

  // TODO
}


// Graphics
QRectF AFMNode::boundingRect() const
{

}

void AFMNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{

}

Item *AFMNode::deepCopy() const
{

}


// PRIVATE

void AFMNode::prepareStatics()
{
  // TODO prepare static vars
}


// Mouse events

void AFMNode::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  // Show path config dialog when selected, with this node highlighted on the list
  // TODO
}

void AFMNode::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
  qDebug() << QObject::tr("AFMNode  has seen the hoverEnterEvent");
  setHovered(true);
  update();
}

void AFMNode::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
  qDebug() << QObject::tr("AFMNode has seen the hoverLeaveEvent");
  setHovered(false);
  update();
}



} // end of prim namespace
