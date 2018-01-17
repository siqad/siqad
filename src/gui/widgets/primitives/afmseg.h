// @file:     afmseg.h
// @author:   Samuel
// @created:  2018.01.16
// @editted:  2018.01.16 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Segment in AFM travel path

#ifndef _PRIM_AFMSEG_H_
#define _PRIM_AFMSEG_H_

#include "item.h"
#include "afmnode.h"

namespace prim{

  class AFMSeg : public Item
  {
  public:

    // Constructor
    AFMSeg(int lay_id, prim::AFMNode *orig_node, prim::AFMNode *dest_node);
    AFMSeg(QXmlStreamReader *rs, QGraphicsScene *scene);
    void initAFMSeg(int lay_id, prim::AFMNode *orig_node, prim::AFMNode *dest_node);

    // Destructor
    ~AFMSeg() {};

    // Save to XML
    virtual void saveItems(QXmlStreamWriter *) const;


    // Segment manipulation

    // change/get segment origin / target
    void setOrigin(prim::AFMNode *orig_node) {origin_node = orig_node;}
    void setDestination(prim::AFMNode *dest_node) {destination_node = dest_node;}
    prim::AFMNode* getOrigin() {return origin_node;}
    prim::AFMNode* getDestination() {return destination_node;}


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

    static qreal seg_width;

  private:

    // initialise static class vars
    void prepareStatics();

    // show path config dialog when selected
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;

    // change visuals when hovered
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *e) Q_DECL_OVERRIDE;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *e) Q_DECL_OVERRIDE;


    // VARS
    prim::AFMNode *origin_node;
    prim::AFMNode *destination_node;
  };

} // end of prim namespace


#endif
