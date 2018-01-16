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
#include "afmpath.h"

namespace prim{

  class AFMPath : public Item
  {
  public:

    // Constructor
    // TODO consider QList types
    AFMPath(int lay_id, QList<AFMNode*> nodes, QList<AFMSeg*> segs, QGraphicsItems *parent=0);
    AFMPath(QXmlStreamReader *rs, QGraphicsScene *scene);
    void initAFMPath(QList<AFMNode*>, QList<AFMSeg*>);

    // Destructor
    ~AFMPath();

    // Save to XML
    virtual void saveItems(QXmlStreamWriter *) const;

    // Nodes

    // insert new node at specified index, or the last index if not specified/out of bound
    void addNode(AFMNode *new_node, int index=path_nodes.length()) {path_nodes.insert(index, new_node);}
    void addNode(QPointF new_loc, int index=path_nodes.length()); // make node at new_loc and add to path

    // remove node at indicated index, or the last index if not specified. Error is thrown if out of bound
    void removeNode(int index=path_nodes.length()-1) {path_nodes.removeAt(index);} // TODO have to deal with change in segments

    // create loops between two indices on the list with a loop count, the greater index loops back to the smaller one. reset_counter_post determines whether the loop counter is reset after the end of this loop, in case a future loop causes this loop to be encountered again.
    void setLoop(int index_a, int index_b, int loop_count, bool reset_counter_post=false);

    // Simulation TODO maybe move simulation code somewhere else

    // unfold path that will be used during simulation
    QList<QPointF> unfoldedPath(int index_a=0, int index_b=path_nodes.length()-1);

    // Graphics
    virtual QRectF boundingRect() const; // conform to segment shapes
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

    virtual Item *deepCopy() const;


  private:

    // initialise static class variables
    void parepareStatics();

    // show path config dialog when selected
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;

    // change visuals when hovered
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *e) Q_DECL_OVERRIDE;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *e) Q_DECL_OVERRIDE;
    

    // VARS
    QList<AFMNode*> path_nodes;
    QList<AFMSeg*> path_segs;
  };

}


#endif
