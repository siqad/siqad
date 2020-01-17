/** @file:     ghost.h
 *  @author:   Jake
 *  @created:  2016.11.24
 *  @editted:  2017.06.07  - Jake
 *  @license:  GNU LGPL v3
 *
 *  @brief:     Graphical objects for moving Items and copy/paste
 */

#ifndef _PRIM_GHOST_H_
#define _PRIM_GHOST_H_


#include "items.h"
#include "settings/settings.h"
#include "lattice.h"

namespace prim{

  //! node structure for describing which sources belong to which aggregate
  struct AggNode{
    int index;              //!< index of source if not an Aggregate
    QList<AggNode*> nodes;  //!< children of the Aggregate if any
    enum SourceType{DBDot, Aggregate, Electrode, AFMArea, TextLabel};

    SourceType source_type;

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

  //! The ghost associated with DBDots and LatticeDots. Used during move and previews.
  class GhostDot : public Item
  {
  public:

    //! constructor
    GhostDot(Item *item, Item *parent, QColor *pcol);

    //! destructor
    ~GhostDot(){}

    //! Return the lattice coordinates of the ghost dot
    prim::LatticeCoord latticeCoord() {return lat_coord;}

    //! Set the lattice coordinate
    void setLatticeCoord(prim::LatticeCoord l_coord) {lat_coord = l_coord;}

    // virtual methods
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) Q_DECL_OVERRIDE;

    Item* deepCopy() const override {return 0;}

  private:

    void constructStatics();

    prim::LatticeCoord lat_coord;
    static qreal diameter;  // constant GhostDot diameter, same for all dots
    QColor *pcol;           // pointer to the GhostDot color
  };

  //! The ghost associated with boxes. Used during move and previews.
  class GhostBox : public Item
  {
  public:

    //! constructor
    GhostBox(Item *item, Item *parent);

    //! destructor
    ~GhostBox(){}

    // virtual methods
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) Q_DECL_OVERRIDE;

    Item* deepCopy() const override {return 0;}

  private:

    qreal width; //width of electrode that was passed
    qreal height; //height of electrode that was passed
    void constructStatics();

    QColor ghost_box_color;
  };

  //! The ghost associated with polygons. Used during move and previews.
  class GhostPolygon : public Item
  {
  public:

    //! constructor
    GhostPolygon(Item *item, Item *parent);

    //! destructor
    ~GhostPolygon(){}

    // virtual methods
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) Q_DECL_OVERRIDE;

    Item* deepCopy() const override {return 0;}

  private:
    QPolygonF poly;
    void constructStatics();

    QColor ghost_poly_color;
  };


  //! collection of GhostDot and GhostBox objects for moving Items or copy/paste, singleton
  class Ghost : public Item
  {
  public:

    QHash<prim::LatticeCoord, bool> valid_hash;  //!< hash table for valid snap points

    //! get or create the static Ghost instance
    static Ghost *instance();

    //! destructor, potentially useful later
    ~Ghost(){inst=0;}

    void cleanGhost();  //!< reset Ghost to its empty state

    //! Add the current Ghost to sc.
    void setScene(QGraphicsScene *sc){sc->addItem(this);}

    //! create a ghost image from a list of Item objects
    //! if moving, scene_pos gives the current mouse location
    void prepare(const QList<prim::Item*> &items, int count=1, QPointF scene_pos=QPointF());

    //! create a ghost image from a single Item
    //! if moving, scene_pos gives the current mouse location
    void prepare(Item *item, int count=1, QPointF scene_pos=QPointF())
    {
      prepare(QList<prim::Item*>({item}), count, scene_pos);
    }

    //! move center of Ghost to the given position
    void moveTo(QPointF pos);

    //! move the ghost by the given lattice coordinate offset
    void moveByCoord(prim::LatticeCoord coord_offset, prim::Lattice *lattice);

    //! get the items associated with the GhostDots
    QList<prim::Item*>& getSources() {return sources;}

    //! get the number of GhostDot sets
    int getCount() const {return sets.count();}

    //! get the items associated with the GhostBoxes
    QList<prim::Item*>& getBoxSources() {return box_sources;}

    //! get the GhostBoxes
    QList<prim::GhostBox*> getBoxes() {return boxes;}

    //! get a list of the highest level Items associated with the sources
    QList<prim::Item*> getTopItems() const;

    //! get a nested list of the items included in each high level item, must
    //! free IndexList pointer after use
    prim::AggNode &getTopIndices(){return aggnode;}

    QList<bool> getLatticeAvailability(const prim::LatticeCoord &offset,
        prim::Lattice *lattice) const;

    //! Gets the lattice coordinates of the GhostDot in the given set that
    //! corresponds to the given DBDot; otherwise return invalid LatticeCoord
    prim::LatticeCoord getLatticeCoord(prim::DBDot *db, int n=0) const;

    //! get the GhostDot nearest to the center of the Ghost. If db is set, will
    //! return the neatest dangling bond GhostDot if any exists else 0.
    prim::GhostDot *snapAnchor() {return anchor;}

    //! location of the anchor dot if the Ghost were centered at the given scene
    //! position.
    QPointF freeAnchor(QPointF scene_pos);


    //! getter for ghost color
    QColor* getCol() {return &col;}
    //! setter for ghost color
    void setCol(QColor &color) {col = color;}

    //! manual set for the validity of the current position
    void setValid(bool val);

    //! check if the current position is valid.
    bool checkValid(const prim::LatticeCoord &offset, prim::Lattice *lattice);

    //! change in position between the first source and GhostDot, for moving Items
    QPointF moveOffset() const;


    // virtual methods
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) Q_DECL_OVERRIDE;

    prim::Item *deepCopy() const override {return 0;}


    // testing
    void echoTopIndices();
    void echoNode(QString &s, AggNode *node);

  private:

    static Ghost *inst;       // static pointer to the singleton instance

    prim::Lattice *lattice=0; // current lattice being used

    // private constructor, singleton
    Ghost();

    // create and add a ghost dot for the given non-Aggregate Item. Possibly add
    // ItemType-specific GhostDots in future... for now just circles
    void createGhostDot(Item *item);
    void createGhostBox(Item *item); //box for electrodes.
    void createGhostPolygon(Item *item); //box for electrodes.

    // prepare a single item. If item is an Aggregate, recursively prepare children
    void prepareItem(Item *item, prim::AggNode *node);

    // check the current position for validity and set if changed
    void updateValid();

    // compute the offset such that the Ghost remains under the cursor
    // unless a scene_pos is given, then that becomes the zero offset
    void zeroGhost(QPointF scene_pos=QPointF());

    // set the snap anchor to the Dangling Bond GhostDot nearest the center of
    // the Ghost. If there is no Dangling Bond GhostDot, returns 0
    void setAnchor();

    // get the Item associated with the given AggNode
    prim::Item *getNodeItem(prim::AggNode *node) const;

    // Move the ghost by the given pixel values
    void translate(qreal dx, qreal dy);

    QList<Item*> sources;         // list of Item objects for each GhostDot
    prim::AggNode aggnode;        // nested structure of sources

    QList< QList<prim::GhostDot*> > sets; // sets of ghost dots

    QList<Item*> box_sources; // list of Item objects for each GhostBox
    QList<prim::GhostBox*> boxes; // list of GhostBoxes

    QList<Item*> poly_sources; // list of Item objects for each GhostBox
    QList<prim::GhostPolygon*> polygons; // list of GhostBoxes


    QColor col;             // current dot color
    bool valid;             // current placement is valid

    prim::GhostDot *anchor; // snap anchor

    QPointF anchor_offset;  // offset of the anchor from the Ghost center
    QPointF zero_offset;    // stored offset for center of Ghost

  };

} // end prim namespace

#endif
