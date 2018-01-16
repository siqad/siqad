// @file:     afmnode.h
// @author:   Samuel
// @created:  2018.01.16
// @editted:  2018.01.16 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Node in AFM travel path

#include "item.h"

namespace prim{

  class AFMNode : public Item
  {
  public:

    // Constructor
    AFMNode(int lay_id, QPointF loc, float z_offset, QGraphicsItems *parent=0);
    AFMNode(QXmlStreamReader *rs, QGraphicsScene *scene);
    void init AFMNode(int lay_id, float z_offset, QPointF loc);

    // Destructor
    ~AFMNode() {};

    // Save to XML
    virtual void saveItems(QXmlStreamWriter *) const;


    // Node manipulation

    // change node location
    void setLocation(QPointF new_loc); // TODO maybe also allow dragging

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

    static qreal node_diameter;


  private:

    // initialise static class variables
    void prepareStatics();

    // show path config dialog when selected
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;

    // change visuals when hovered
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *e) Q_DECL_OVERRIDE;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *e) Q_DECL_OVERRIDE;


    // VARS
    float zoffset;
  };

} // end of prim namespace
