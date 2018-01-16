// @file:     afmpath.h
// @author:   Samuel
// @created:  2018.01.16
// @editted:  2018.01.16 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Base class for AFM travel path

#include "item.h"

namespace prim{

  class AFMPath : public Item
  {
  public:

    // Constructor
    AFMPath(int lay_id, QList<QPointF> nodes, QGraphicsItems *parent=0);
    AFMPath(QXmlStreamReader *rs, QGraphicsScene *scene);
    void initAFMPath(QStack<QPointF>);

    // Destructor
    ~AFMPath() {};

    // Nodes

    // insert new node at specified index, or the last index if not specified/out of bound
    void addNode(QPointF loc, int index=path_nodes.length()) {path_nodes.insert(index, loc);}

    // remove node at indicated index, or the last index if not specified. Error is thrown if out of bound
    void removeNode(int index=path_nodes.length()-1) {path_nodes.removeAt(index);}

    // create loops between two indices on the list with a loop count, the greater index loops back to the smaller one. reset_counter_post determines whether the loop counter is reset after the end of this loop, in case a future loop causes this loop to be encountered again.
    void setLoop(int index_a, int index_b, int loop_count, bool reset_counter_post=false);

    // Simulation TODO maybe move simulation code somewhere else

    // unfold path that will be used during simulation
    QList<QPointF> unfoldedPath(int index_a=0, int index_b=path_nodes.length()-1);

    // Graphics
    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

    virtual Item *deepCopy() const;

    // params for this path
    QList<QColor> node_fill_cols; // fill color of each node
    QList<QColor> node_bd_col;    // border color of each node
    QList<QColor> seg_fill_col;   // fill color of each segment
    QList<QColor> seg_bd_col;     // border color of each segment

    // static params for painting
    struct StateColors
    {
      QColor def;
      QColor hovered;
      QColor sel;
    };
    static StateColors node_fill;
    static StateColors node_bd;
    static StateColors seg_fill;
    static StateColors seg_bd;

    static qreal node_diameter;
    static qreal seg_width;


    // Save to file
    virtual void saveItems(QXmlStreamWriter *) const;

  private:

    // initialize static class variables
    void parepareStatics();

    // show path config dialog when selected
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;

    // change visuals when hovered
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *e) Q_DECL_OVERRIDE;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *e) Q_DECL_OVERRIDE;

    QList<QPointF> path_nodes;
  };

}
