// @file:     afmpath.cc
// @author:   Samuel
// @created:  2018.01.16
// @editted:  2018.01.16 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Base class for AFM travel path

#include "afmpath.h"

namespace prim {

StateColors AFMPath::node_fill;
StateColors AFMPath::node_bd;
StateColors AFMPath::seg_fill;
StateColors AFMPath::seg_bd;

qreal AFMPath::node_diameter;
qreal AFMPath::seg_width;

AFMPath::AFMPath(int lay_id, QList<QPointF> nodes, QGraphicsItems *parent=0)
  : prim::AFMPath(prim::Item::AFMPath, lay_id, parent)
{
  initAFMPath(nodes);
}

AFMPath::AFMPath(QXmlStreamReader *rs, QGraphicsScene *scene)
  : prim::Item(prim::Item::AFMPath)
{

}

void AFMPath::initAFMPath(QList<QPointF> nodes)
{
  path_nodes = nodes;
  // TODO init GUI elements related to path
}



} // end of prim namespace
