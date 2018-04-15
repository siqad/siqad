/** @file:     aggregate.h
 *  @author:   Jake
 *  @created:  2017.05.16
 *  @editted:  2017.06.07  - Jake
 *  @license:  GNU LGPL v3
 *
 *  @brief:     Base class for Aggregate Item type
 */

#include "item.h"

namespace prim{

  // forward declaration of prim::Layer
  class Layer;

  //! custom class which is both derived from Item and acts as a container class
  //! for collections of Item objects.
  class Aggregate : public Item
  {
  public:
    //! constructor, takes a list of children Items
    Aggregate(int lay_id, QStack<Item*> &items, QGraphicsItem *parent=0);
    //! constructor, creates an aggregate from the design file
    Aggregate(QXmlStreamReader *stream, QGraphicsScene *scene);
    void initAggregate(QStack<Item*> &items, QGraphicsItem *parent=0);

    //! destructor, makes all children belong to Aggregates parent
    ~Aggregate();

    //! set given items as children
    void addChildren(QStack<Item*> &items);

    //! get all items of the aggregate
    QStack<prim::Item*> &getChildren() {return items;}

    // necessary derived class member functions
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;

    virtual Item *deepCopy() const override;

    static QColor edge_col;
    static QColor edge_col_hovered;

    // save to file
    virtual void saveItems(QXmlStreamWriter *) const override;

  private:

    QPointF p0; // center position

    QStack<prim::Item*> items;

    // initialise the static class variables
    void prepareStatics();

    // aggregates should have no inherrent click behaviour. Rather, child Items
    // should inform the aggregate that they have been clicked.
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *e) override;

    // handle hover events for highlighing aggregate boundary
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *e) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *e) override;
  };

} // end prim namespace
