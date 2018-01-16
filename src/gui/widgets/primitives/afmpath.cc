// @file:     afmpath.cc
// @author:   Samuel
// @created:  2018.01.16
// @editted:  2018.01.16 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Base class for AFM travel path

#ifndef _PRIM_AFMPATH_H_
#define _PRIM_AFMPATH_H_

#include "afmpath.h"

namespace prim {

StateColors AFMPath::node_fill;
StateColors AFMPath::node_bd;
StateColors AFMPath::seg_fill;
StateColors AFMPath::seg_bd;

qreal AFMPath::node_diameter;
qreal AFMPath::seg_width;

AFMPath::AFMPath(int lay_id, QList<AFMNode*> nodes, QList<AFMSeg*> segs, QGraphicsItems *parent=0)
  : prim::AFMPath(prim::Item::AFMPath, lay_id, parent)
{
  initAFMPath(nodes, segs);
}

AFMPath::AFMPath(QXmlStreamReader *rs, QGraphicsScene *scene)
  : prim::Item(prim::Item::AFMPath)
{
  // TODO load from file
}

void AFMPath::initAFMPath(QList<AFMNode*> nodes, QList<AFMSeg*> segs)
{
  path_nodes = nodes;
  path_segs = segs;
  // TODO check that segments match up with the nodes
  // TODO ask Jake if there's a better way to store nodes and segments

  // TODO init GUI elements related to path
}


void AFMPath::~AFMPath()
{
  for (auto node : path_nodes)
    delete node;
}


void AFMPath::addNode(QPointF new_loc, int index)
{
  // make node at new_loc and add to current path
  // TODO
}


void AFMPath::setLoop(int index_a, int index_b, int loop_count, bool reset_counter_post)
{
  // The greater index always points back to the smaller one. Each node may only be the
  // starting point of one loop, but they are allowed to be the target of multiple loops.

  // TODO
}


QRectF AFMPath::boundingRect() const
{

}


QList<QPointF> unfoldedPath(int index_a, int index_b)
{

}


void AFMPath::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{

}

Item *AFMPath::deepCopy() const
{
  AFMPath *cp = new AFMPath(layer_id, path_nodes); // TODO might be missing stuff
  cp->setPos(pos());
  return cp;
}


void AFMPath::prepareStatics()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  // node properties
  node_diameter = gui_settings->get<qreal>("afmpath/node_diameter");
  node_fill.def = gui_settings->get<QColor>("afmpath/node_fill_col_default");
  node_fill.hovered = gui_settings->get<QColor>("afmpath/node_fill_col_hovered");
  node_fill.sel = gui_settings->get<QColor>("afmpath/node_fill_col_sel");
  node_bd.def = gui_settings->get<QColor>("afmpath/node_bd_col_default");
  node_bd.hovered = gui_settings->get<QColor>("afmpath/node_bd_col_hovered");
  node_bd.sel = gui_settings->get<QColor>("afmpath/node_bd_col_sel");

  // segment properties
  seg_width = gui_settings->get<qreal>("afmpath/seg_width");
  seg_fill.def = gui_settings->get<QColor>("afmpath/seg_fill_col_default");
  seg_fill.hovered = gui_settings->get<QColor>("afmpath/seg_fill_col_hovered");
  seg_fill.sel = gui_settings->get<QColor>("afmpath/seg_fill_col_sel");
  seg_bd.def = gui_settings->get<QColor>("afmpath/seg_bd_col_default");
  seg_bd.hovered = gui_settings->get<QColor>("afmpath/seg_bd_col_hovered");
  seg_bd.sel = gui_settings->get<QColor>("afmpath/seg_bd_col_sel");

}


void AFMPath::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  // Show path config dialog that allows users to edit AFM variables, node/path
  // related settings, etc.
  // TODO
}


void AFMPath::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
  setHovered(true);
  update();
}


void AFMPath::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
  setHovered(false);
  update();
}



} // end of prim namespace


#endif
