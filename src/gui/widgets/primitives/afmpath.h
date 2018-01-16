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

    // constructor
    AFMPath(int lay_id, QList<QPointF> nodes, QGraphicsItems *parent=0);
    AFMPath(QXmlStreamReader *rs, QGraphicsScene *scene);
    void initAFMPath(QStack<QPointF>);

    // destructor
    ~AFMPath() {};

    // graphics
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


    // save to file
    virtual void saveItems(QXmlStreamWriter *) const;

  private:

    QList<QPointF> path_nodes;
  };

}
