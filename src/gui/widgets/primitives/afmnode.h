// @file:     afmnode.h
// @author:   Samuel
// @created:  2018.01.16
// @editted:  2018.01.16 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Node in AFM travel path

#ifndef _PRIM_AFMNODE_H_
#define _PRIM_AFMNODE_H_

#include "item.h"

namespace prim{

  class AFMNode : public Item
  {
  public:

    // Constructor
    AFMNode(int lay_id, QPointF sceneloc, float z_offset);
    AFMNode(QXmlStreamReader *rs, QGraphicsScene *scene);
    void initAFMNode(int lay_id, QPointF sceneloc, float z_offset);

    // Destructor
    ~AFMNode() {};

    // Save to XML
    virtual void saveItems(QXmlStreamWriter *) const;


    // Node manipulation

    // change node zoffset (relative to surface) if it is within bounds of the layer
    void setZOffset(float z_offset);
    float getZOffset() {return zoffset;}


    // Graphics
    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget*);

    virtual Item *deepCopy() const;

    // params
    QColor fill_col;
    QColor bd_col;

    // static params
    static QColor fill_col_default;
    static QColor fill_col_hovered;
    static QColor fill_col_sel;
    static QColor bd_col_default;
    static QColor bd_col_hovered;
    static QColor bd_col_sel;

    static qreal diameter;
    static qreal edge_width;


  private:

    // initialise static class variables
    void prepareStatics();

    // show path config dialog when selected
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;

    // change visuals when hovered
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *) Q_DECL_OVERRIDE;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *) Q_DECL_OVERRIDE;


    // VARS
    QPointF phys_loc;   // physical location in angstrom
    float zoffset;
  };

} // end of prim namespace


#endif
