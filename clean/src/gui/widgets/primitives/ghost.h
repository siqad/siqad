// @file:     ghost.h
// @author:   Jake
// @created:  2016.11.24
// @editted:  2017.05.31  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Graphical objects for moving Items and copy/paste

#ifndef _PRIM_GHOST_H_
#define _PRIM_GHOST_H_


#include "items.h"
#include "src/settings/settings.h"

namespace prim{

  // node structure for describing which sources belong to which aggregate
  struct AggNode{
    int index;              // index of source if not an Aggregate
    QList<AggNode*> nodes;  // children of the Aggregate if any

    AggNode(int index=-1) : index(index) {}
    ~AggNode() {for(AggNode* node : nodes) delete node;}

    void reset()
    {
      index=-1;
      for(AggNode *node : nodes)
        delete node;
      nodes.clear();
    }

  };

  class GhostDot : public Item
  {
  public:

    // constructor
    GhostDot(Item *item, Item *parent, QColor *pcol);

    // destructor
    ~GhostDot(){}

    // virtual methods
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) Q_DECL_OVERRIDE;

  private:

    void constructStatics();

    static qreal diameter;  // constant GhostDot diameter, same for all dots
    QColor *pcol;           // pointer to the GhostDot color
  };


  // collection of GhostDot objects for moving Items or copy/paste, singleton
  class Ghost : public Item
  {
  public:

    QHash<prim::LatticeDot*, bool> valid_hash;  // hash table for valid snap points

    // get or create the static Ghost instance
    static Ghost *instance();

    // destructor, potentially useful later
    ~Ghost(){}

    void cleanGhost();  // reset Ghost to its empty state

    void setScene(QGraphicsScene *sc){sc->addItem(this);}

    // create a ghost image from a list of Item objects or a single Item
    void prepare(const QList<prim::Item*> &items);
    void prepare(Item *item);

    // move center of Ghost to the given position
    void moveTo(QPointF pos);

    QList<prim::Item*>& getSources() {return sources;}
    QList<prim::GhostDot*> getDots() {return dots;}

    // get a list of the highest level Items associated with the sources
    QList<prim::Item*> getTopItems() const;

    // get a nested list of the items included in each high level item, must
    // free IndexList pointer after use
    prim::AggNode &getTopIndices(){return aggnode;}

    // get a list corresponding to the LatticeDot associated with each GhostDot.
    // If the Item associated with the GhostDot is not a dangling bond the list
    // will contain a 0.
    QList<prim::LatticeDot*> getLattice(const QPointF &offset = QPointF()) const;

    // get the GhostDot nearest to the center of the Ghost. If db is set, will
    // return the neatest dangling bond GhostDot if any exists else 0.
    prim::GhostDot *snapAnchor() {return anchor;}

    // location of the anchor dot if the Ghost were centered at the given scene
    // position.
    QPointF freeAnchor(QPointF scene_pos);


    // accessors for ghost color
    QColor* getCol() {return &col;}
    void setCol(QColor &color) {col = color;}

    // manual set the validity of the current position
    void setValid(bool val);

    // check if the current position is valid.
    bool checkValid(const QPointF &offset = QPointF());

    // change in position between the first source and GhostDot, for moving Items
    QPointF moveOffset() const;


    // virtual methods
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) Q_DECL_OVERRIDE;


    // testing
    void echoTopIndices();
    void echoNode(QString &s, AggNode *node);

  private:

    static Ghost *inst;     // static pointer to the singleton instance

    // private constructor, singleton
    Ghost();

    // create and add a ghost dot for the given non-Aggregate Item. Possibly add
    // ItemType-specific GhostDots in future... for now just circles
    void createGhostDot(Item *item);

    // prepare a single item. If item is an Aggregate, recursively prepare children
    void prepareItem(Item *item, prim::AggNode *node);

    // check the current position for validity and set if changed
    void updateValid();


    // compute the offset such that the Ghost remains under the cursor
    void zeroGhost();

    // set the snap anchor to the Dangling Bond GhostDot nearest the center of
    // the Ghost. If there is no Dangling Bond GhostDot, returns 0
    void setAnchor();

    // get the Item associated with the given AggNode
    prim::Item *getNodeItem(prim::AggNode *node) const;

    QList<Item*> sources;         // list of Item objects for each GhostDot
    QList<prim::GhostDot*> dots;  // list of GhostDots
    prim::AggNode aggnode;        // nested structure of sources

    QColor col;             // current dot color
    bool valid;             // current placement is valid

    prim::GhostDot *anchor; // snap anchor

    QPointF anchor_offset;  // offset of the anchor from the Ghost center
    QPointF zero_offset;    // stored offset for center of Ghost

  };

} // end prim namespace



#endif
