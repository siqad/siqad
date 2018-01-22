// @file:     afmnode.cc
// @author:   Samuel
// @created:  2018.01.17
// @editted:  2018.01.17 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Node in AFM travel path

#include "afmnode.h"

namespace prim{

// Initialise statics
QColor AFMNode::fill_col_default;
QColor AFMNode::fill_col_hovered;
QColor AFMNode::fill_col_sel;
QColor AFMNode::bd_col_default;
QColor AFMNode::bd_col_hovered;
QColor AFMNode::bd_col_sel;

qreal AFMNode::diameter = -1;
qreal AFMNode::edge_width = -1;

// constructors
AFMNode::AFMNode(int lay_id, QPointF sceneloc, float z_offset)
  : prim::Item(prim::Item::AFMNode)
{
  initAFMNode(lay_id, sceneloc, z_offset);
}

AFMNode::AFMNode(QXmlStreamReader *rs, QGraphicsScene *scene)
  : prim::Item(prim::Item::AFMNode)
{

  // TODO call initAFMNode with read values
}

void AFMNode::initAFMNode(int lay_id, QPointF sceneloc, float z_offset)
{
  // initialise static variables if not already
  if (diameter < 0)
    prepareStatics();

  layer_id = lay_id;
  setZOffset(z_offset);
  setPos(sceneloc);

  setAcceptHoverEvents(true);

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
  qreal width = diameter+edge_width;
  return QRectF(-.5*width, -.5*width, width, width);
}

void AFMNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  if (select_mode && upSelected()) {
    fill_col = fill_col_sel;
    bd_col = bd_col_sel;
  } else if (isHovered()) {
    fill_col = fill_col_hovered;
    bd_col = bd_col_hovered;
  } else {
    fill_col = fill_col_default;
    bd_col = bd_col_default;
  }

  QRectF rect = boundingRect();
  qreal dxy = .5*edge_width;
  rect.adjust(dxy,dxy,-dxy,-dxy);
  
  painter->setPen(Qt::NoPen);
  painter->setBrush(fill_col);
  painter->drawEllipse(rect);
}

Item *AFMNode::deepCopy() const
{
  // TODO
}


// PRIVATE

void AFMNode::prepareStatics()
{
  // Initialize statics
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  fill_col_default = gui_settings->get<QColor>("afmnode/fill_col_default");
  fill_col_hovered = gui_settings->get<QColor>("afmnode/fill_col_hovered");
  fill_col_sel = gui_settings->get<QColor>("afmnode/fill_col_sel");
  bd_col_default = gui_settings->get<QColor>("afmnode/bd_col_default");
  bd_col_hovered = gui_settings->get<QColor>("afmnode/bd_col_hovered");
  bd_col_sel = gui_settings->get<QColor>("afmnode/bd_col_sel");
  diameter = gui_settings->get<qreal>("afmnode/diameter")*scale_factor;
  edge_width = gui_settings->get<qreal>("afmnode/edge_width")*diameter;
}


// Mouse events

void AFMNode::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  // Show path config dialog when selected, with this node highlighted on the list
  // TODO
}

void AFMNode::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
  setHovered(true);
  update();
}

void AFMNode::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
  setHovered(false);
  update();
}



} // end of prim namespace
