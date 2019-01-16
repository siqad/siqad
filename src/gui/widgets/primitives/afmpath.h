// @file:     afmpath.h
// @author:   Samuel
// @created:  2018.01.16
// @editted:  2018.01.16 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Base class for AFM travel path

#ifndef _PRIM_AFMPATH_H_
#define _PRIM_AFMPATH_H_

#include "item.h"
#include "afmseg.h"
#include "afmnode.h"

namespace prim{

  class AFMPath : public Item
  {
  public:

    // Constructor
    // TODO consider QList types
    AFMPath(int lay_id);
    AFMPath(int lay_id, const QList<prim::AFMNode*> &nodes);
    AFMPath(QXmlStreamReader *rs, QGraphicsScene *scene, int lay_id);
    void initAFMPath(int lay_id, const QList<prim::AFMNode*> &);

    // Destructor
    ~AFMPath() {}; // AFMNode and AFMSeg pointers are deleted by DesignPanel on scene clear

    // Save to XML
    virtual void saveItems(QXmlStreamWriter *) const override;

    // Nodes

    // insert new node at specified index
    //void insertNode(QPointF new_loc, int index, float z_offset=0); // make node at new_loc and add to path
    void insertNode(prim::AFMNode *new_node, int index=-1);
    //void appendNode(QPointF new_loc, float z_offset=0) {insertNode(new_loc, z_offset, path_nodes.length());}
    //void appendNode(prim::AFMNode *new_node) {insertNode(new_node, path_nodes.length());}

    // remove node at indicated index
    void removeNode(int index);

    // get node at index
    prim::AFMNode *getNode(int index) const {return (index < nodeCount()) ? path_nodes.at(index) : 0;}
    prim::AFMNode *getLastNode() const {return (nodeCount() > 0) ? getNode(path_nodes.length()-1): 0;}

    // get index position of node pointer in path_nodes list
    // returns -1 if none exists
    int getNodeIndex(prim::AFMNode *node) const {return path_nodes.indexOf(node);}
    int getLastNodeIndex() const {return path_nodes.length()-1;}

    // node count
    int nodeCount() const {return path_nodes.length();}

    // segment count
    int segmentCount() const {return path_segs.length();}

    // insert segment to index position with path_node[index] as origin and
    // path_node[index+1] as destination
    void insertSegment(int index);

    // remove segment at index, doesn't do anything else
    void removeSegment(int index);

    // get segment at index, or the last index if not specified
    prim::AFMSeg *getSegment(int index) const {return path_segs.at(index);}

    // get segments connected to the supplied node pointer / index
    QList<prim::AFMSeg*> getConnectedSegments(prim::AFMNode *node);
    QList<prim::AFMSeg*> getConnectedSegments(int node_ind);

    // create loop between the nodes at first and last indices, they must be at the same physical location
    void setLoop(bool loop_state);

    // Simulation TODO maybe move simulation code somewhere else

    // unfold path that will be used during simulation
    QList<QPointF> unfoldedPath();

    // Graphics
    virtual QRectF boundingRect() const override; // conform to segment shapes
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;

    virtual Item *deepCopy() const override;

  /* TODO remove
  signals:
    void addSegmentToScene(prim::AFMSeg *add_segment);
    void removeSegmentFromScene(prim::AFMSeg *remove_segment);*/

  private:

    // initialise static class vars
    void prepareStatics();

    // show path config dialog when selected
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *e) override;


    // VARS
    QList<prim::AFMNode*> path_nodes;
    QList<prim::AFMSeg*> path_segs;

    QColor line_col;

    // static
    static QColor line_col_default;
    static QColor line_col_hovered;
    static QColor line_col_sel;
    static qreal line_width;
  };

}


#endif
