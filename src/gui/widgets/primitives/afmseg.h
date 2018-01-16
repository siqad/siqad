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
    AFMSeg(int lay_id, AFMNode *node_from, AFMNode *node_to, QGraphicsItems *parent=0);
    AFMSeg(QXmlStreamReader *rs, QGraphicsScene *scene);
    void init AFMSeg(int lay_id, AFMNode *node_from, AFMNode *node_to);

    // Destructor
    ~AFMSeg() {};

    // Save to XML
    virtual void saveItems(QXmlStreamWriter *) const;


    // Segment manipulation

    // change segment speed
    void setSpeed(float new_speed) {};

    // change segment travel profile
    void setTravelProfile() {}; // TODO not sure what input param to take yet, maybe enum?


    // Graphics
    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWiget*);

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
    float speed;
    // TODO travel profile, maybe enum?
  };

} // end of prim namespace


#endif
