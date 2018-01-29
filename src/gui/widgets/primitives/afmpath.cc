// @file:     afmpath.cc
// @author:   Samuel
// @created:  2018.01.16
// @editted:  2018.01.16 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Base class for AFM travel path

#include "afmpath.h"

namespace prim {

AFMPath::AFMPath(int lay_id)
  : prim::Item(prim::Item::AFMPath)
{
  initAFMPath(lay_id, QList<prim::AFMNode*>());
}

AFMPath::AFMPath(int lay_id, const QList<prim::AFMNode*> &nodes)
  : prim::Item(prim::Item::AFMPath)
{
  initAFMPath(lay_id, nodes);
}

AFMPath::AFMPath(QXmlStreamReader *rs, QGraphicsScene *scene)
  : prim::Item(prim::Item::AFMPath)
{
  int lay_id = -1;
  QList<prim::AFMNode*> ld_nodes;

  // TODO read own attributes, e.g. zoffset
  
  while (!rs->atEnd()) {
    if (rs->isStartElement()) {
      if (rs->name() == "layer_id") {
        lay_id = rs->readElementText().toInt();
        rs->readNext();
      } else if (rs->name() == "afmnode") {
        qDebug() << QObject::tr("Adding node to path...");
        ld_nodes.append(new prim::AFMNode(rs, scene));
        qDebug() << QObject::tr("Added node to path");
      } else {
        rs->readNext();
      }
    } else if (rs->isEndElement()) {
      // break out of read stream if the end of this element has been reached
      if (rs->name() == "afmpath") {
        rs->readNext();
        break;
      }
      rs->readNext();
    } else {
      rs->readNext();
    }
  }

  qDebug() << "Done reading path";

  if (rs->hasError()) {
    qCritical() << QObject::tr("XML error: ") << rs->errorString().data();
  }

  // initialise AFM Path with loaded information
  initAFMPath(lay_id, ld_nodes);
  scene->addItem(this);
}

void AFMPath::initAFMPath(int lay_id, const QList<prim::AFMNode*> &nodes)
{
  layer_id = lay_id;

  for (prim::AFMNode* node : nodes)
    insertNode(node);

  setFlag(QGraphicsItem::ItemIsSelectable, true);

  // TODO emit signal declaring this to be the focused path
}


// Save to XML
void AFMPath::saveItems(QXmlStreamWriter *ws) const
{
  ws->writeStartElement("afmpath");
  // TODO save path properties like speed, loop as attributes
  ws->writeTextElement("layer_id", QString::number(layer_id));

  // save included afmnodes
  for (prim::AFMNode *node : path_nodes) {
    node->saveItems(ws);
  }

  ws->writeEndElement();
}


void AFMPath::insertNode(prim::AFMNode *new_node, int index)
{
  if (index == -1)
    index = path_nodes.length(); // append to back if index == -1

  prepareGeometryChange();

  // insert the node to node list
  path_nodes.insert(index, new_node);
  new_node->setParentItem(this);
  new_node->setFlag(QGraphicsItem::ItemIsSelectable, false);

  int last_index = path_nodes.length()-1;

  if (index != 0 && index == last_index) {
    // if the new node is appended to the previous last node, create a segment that
    // originates from the last node to this one.
    qDebug() << QObject::tr("Appending new segment to AFMPath");
    insertSegment(index-1);
  } else {
    // inserting
    qDebug() << QObject::tr("Inserting new segment to AFMPath for node at index %1").arg(index);

    // reconnect preceding segment to this node
    if (index > 0)
      getSegment(index-1)->setDestinationNode(getNode(index));
    // create new segment connecting this node to the next
    if (index != last_index)
      insertSegment(index);
  }
}


void AFMPath::removeNode(int index)
{
  int last_index = path_nodes.length()-1;

  prepareGeometryChange();

  // take the node out of nodes list
  // NOTE might segfault, look into delete behavior when deleting a child item for QGraphicsItem
  prim::AFMNode *rm_node = path_nodes.takeAt(index);
  prim::Emitter::instance()->removeItemFromScene(rm_node);
  delete rm_node;

  // skip removing segments if there aren't any left
  if (segmentCount() == 0)
    return;

  // remove segments associated with that node
  if (index == 0) {
    removeSegment(index);
  } else if (index == last_index) {
    removeSegment(last_index-1);
  } else {
    removeSegment(index);
    // reconnect segment to new neighboring node
    getSegment(index-1)->setDestinationNode(getNode(index));
  }
}


void AFMPath::insertSegment(int index)
{
  prim::AFMSeg *new_segment = new prim::AFMSeg(layer_id, getNode(index), getNode(index+1));
  new_segment->setParentItem(this);
  path_segs.insert(index, new_segment);
}


void AFMPath::removeSegment(int index)
{
  prim::AFMSeg *rm_seg = path_segs.takeAt(index);
  prim::Emitter::instance()->removeItemFromScene(rm_seg);
  delete rm_seg;
}


QList<prim::AFMSeg*> AFMPath::getConnectedSegments(prim::AFMNode *node)
{
  return getConnectedSegments(getNodeIndex(node));
}

QList<prim::AFMSeg*> AFMPath::getConnectedSegments(int node_ind)
{
  QList<prim::AFMSeg*> connected_segs;

  if (node_ind == -1) {
    qCritical() << QObject::tr("Cannot get connected segment for node index -1");
    return connected_segs;
  }

  // get the preceding segment if not the first node
  if (node_ind != 0)
    connected_segs.append(getSegment(node_ind-1));

  // get the proceeding segment if not the last node
  if (node_ind != path_nodes.count()-1)
    connected_segs.append(getSegment(node_ind));

  return connected_segs;
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
  qreal xmin, ymin, xmax, ymax, width, height;
  bool unset = true;

  for (prim::AFMNode *node : path_nodes) {
    QRectF rect = node->boundingRect();
    QPointF pos = node->pos();
    if (unset) {
      unset = false;
      xmin = pos.x()+rect.left();
      ymin = pos.y()+rect.top();
      xmax = pos.x()+rect.right();
      ymax = pos.y()+rect.bottom();
    } else {
      xmin = qMin(xmin, pos.x()+rect.left());
      ymin = qMin(ymin, pos.y()+rect.top());
      xmax = qMax(xmax, pos.x()+rect.right());
      ymax = qMax(ymax, pos.y()+rect.bottom());
    }
  }

  width = xmax-xmin;
  height = ymax-ymin;
  return QRectF(.5*(xmax+xmin-width), .5*(ymax+ymin-height), width, height);
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
  AFMPath *cp = new AFMPath(layer_id, path_nodes);
  cp->setPos(pos());
  return cp;
}


void AFMPath::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  // Show path config dialog that allows users to edit AFM variables, node/path
  // related settings, etc.

  e->ignore();
}


} // end of prim namespace
