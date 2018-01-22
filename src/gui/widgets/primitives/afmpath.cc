// @file:     afmpath.cc
// @author:   Samuel
// @created:  2018.01.16
// @editted:  2018.01.16 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Base class for AFM travel path

#include "afmpath.h"

namespace prim {

AFMPath::AFMPath(int lay_id, prim::AFMNode *node)
  : prim::Item(prim::Item::AFMPath)
{
  QList<prim::AFMNode*> nodes;
  QList<prim::AFMSeg*> segs;
  nodes.append(node);
  initAFMPath(lay_id, nodes, segs);
}

AFMPath::AFMPath(int lay_id, QList<prim::AFMNode*> nodes, QList<prim::AFMSeg*> segs)
  : prim::Item(prim::Item::AFMPath)
{
  initAFMPath(lay_id, nodes, segs);
}

AFMPath::AFMPath(QXmlStreamReader *rs, QGraphicsScene *scene)
  : prim::Item(prim::Item::AFMPath)
{
  // TODO load from file
}

void AFMPath::initAFMPath(int lay_id, QList<prim::AFMNode*> nodes, QList<prim::AFMSeg*> segs)
{
  layer_id = lay_id;
  path_nodes = nodes;
  path_segs = segs;
  // TODO check that segments match up with the nodes
  // TODO regenerate segs if not

  // TODO init GUI elements related to path
}


AFMPath::~AFMPath()
{
  for (auto node : path_nodes)
    delete node;
}


// Save to XML
void AFMPath::saveItems(QXmlStreamWriter *) const
{
  // TODO save included afmnodes and afmsegs
  // TODO save path properties like speed, loop
}

/*void AFMPath::insertNode(QPointF new_loc, int index, float z_offset)
{
  if (z_offset == 0)
    z_offset = getNode(index-1)->getZOffset();

  insertNode(new prim::AFMNode(layer_id, new_loc, z_offset), index);
}*/


void AFMPath::insertNode(prim::AFMNode *new_node, int index)
{
  // insert the node to node list
  path_nodes.insert(index, new_node);

  int last_index = path_nodes.length()-1;

  if (index == last_index) {
    qDebug() << QObject::tr("Appending new segment to AFMPath");
    // appending
    insertSegment(index-1);
  } else {
    // inserting
    qDebug() << QObject::tr("Inserting new segment to AFMPath for node at index %1").arg(index);

    // reconnect preceding segment to this node
    if (index > 0)
      getSegment(index-1)->setDestination(getNode(index));
    // create new segment connecting this node to the next
    if (index != last_index)
      insertSegment(index);
  }
}


void AFMPath::removeNode(int index)
{
  int last_index = path_nodes.length()-1;

  // take the node out of nodes list
  prim::AFMNode *rm_node = path_nodes.takeAt(index);
  delete rm_node;

  // remove segments associated with that node
  if (index == 0) {
    removeSegment(index);
  } else if (index == last_index) {
    removeSegment(last_index-1);
  } else {
    removeSegment(index);
    // reconnect segment to new neighboring node
    getSegment(index-1)->setDestination(getNode(index));
  }
}


void AFMPath::insertSegment(int index)
{
  path_segs.insert(index, new prim::AFMSeg(layer_id, getNode(index), getNode(index+1)));
}


void AFMPath::removeSegment(int index)
{
  prim::AFMSeg *rm_seg = path_segs.takeAt(index);
  delete rm_seg;
}


void AFMPath::setLoop(bool loop_state)
{
  // Check if the first node and the end node are at the same location. If yes,
  // create the loop. If not, prompt the user whether they want a node to be
  // automatically added at the end.

  // TODO
}


QRectF AFMPath::boundingRect() const
{

}


QList<QPointF> AFMPath::unfoldedPath()
{

}


void AFMPath::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{

}

Item *AFMPath::deepCopy() const
{
  /*TODO look into aggregate's deep copy to figure out how this should be implemented.
     Keep in mind that when pasting, all off the path_nodes and path_segs have to take
     the new location, electrode code might have that implemented.*/
  AFMPath *cp = new AFMPath(layer_id, path_nodes, path_segs);
  cp->setPos(pos());
  return cp;
}


/* could be reused in afmnode and afmseg
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

}*/


void AFMPath::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  // Show path config dialog that allows users to edit AFM variables, node/path
  // related settings, etc.
  // TODO
}


} // end of prim namespace
