/** @file:     dbdot.h
 *  @author:   Jake
 *  @created:  2016.11.15
 *  @editted:  2017.06.07  - Jake
 *  @license:  GNU LGPL v3
 *
 *  @brief:     DBDot Widget for functionality of dangling bonds
 */

#ifndef _GUI_PR_DBDOT_H_
#define _GUI_PR_DBDOT_H_


#include <QtWidgets>
#include "item.h"
#include "lattice.h"

namespace prim{

  class DBDot: public prim::Item
  {
  public:

    //! constructor, creating DBDot using the DBGenTool.
    DBDot(prim::LatticeCoord l_coord, int lay_id, bool cp=false);
    //! constructor, creating DBDot from a design file.
    DBDot(QXmlStreamReader *, QGraphicsScene *, int lay_id);

    // initializer
    void initDBDot(prim::LatticeCoord coord, int lay_id, bool cp);

    // destructor
    ~DBDot(){}

    // accessors

    //! Set the physical location of the DB (Design panel calls this when the
    //! moveDBToLatticeCoord signal is emitted, don't call this otherwise).
    void setPhysLoc(QPointF loc) {physloc = loc;}

    //! Get the physical location of the DB
    QPointF physLoc() const {return physloc;}

    //! Get the lattice coordinates of the DB
    prim::LatticeCoord latticeCoord() {return lat_coord;}

    //! Set the lattice coordinates of the DB
    void setLatticeCoord(prim::LatticeCoord l_coord);

    //! Set electron occupant visibility
    void setShowElec(float se_in);

    //! Set the graphical fill of the DB
    void setFill(float fill){fill_fact = fill;}

    // inherited abstract method implementations
    virtual void setColor(QColor color) override;
    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;

    prim::Item *deepCopy() const override;

    // SAVE LOAD
    virtual void saveItems(QXmlStreamWriter *) const override;
    
    virtual QColor getCurrentFillColor() override {return fill_col.normal;}

  protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *e) override;

  private:

    // construct static variables
    void constructStatics();

    // VARIABLES
    prim::LatticeCoord lat_coord; // lattice coordinates of the DB
    QPointF physloc;             // physical location
    float show_elec=0;            // simulation result visualization electron, 1=has electron

    qreal fill_fact;          // area proportional of dot filled

    prim::Item::StateColors fill_col;
    // static class parameters for painting

    static prim::Item::StateColors fill_col_def;            // normal dbdot
    static prim::Item::StateColors fill_col_electron;   // DB- site
    static prim::Item::StateColors fill_col_hole;       // DB+ site
    static prim::Item::StateColors fill_col_neutral;    // DB+ site
    static prim::Item::StateColors edge_col;            // edge of the dbdot
    static prim::Item::StateColors edge_col_electron;   // edge of the dbdot
    static prim::Item::StateColors edge_col_hole;       // edge of the dbdot
    static prim::Item::StateColors edge_col_neutral;    // edge of the dbdot


    qreal diameter;      // dot diameter in angstroms
    static qreal diameter_m;    // medium sized dot
    static qreal diameter_l;    // large sized dot
    static qreal edge_width;    // proportional width of dot boundary edge
    static qreal publish_scale; // size scaling factor when in publish screenshot mode

  };


  class DBDotPreview : public prim::Item
  {
  public:

    //! Contruct a DB dot preview.
    DBDotPreview(prim::LatticeCoord l_coord);

    //! Destructor.
    ~DBDotPreview() {}

    // Accessors

    //! Get the lattice coordinates of the DB Preview
    prim::LatticeCoord latticeCoord() {return lat_coord;}

    // Graphics
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;

  private:
    //! Construct static variables on first creation.
    void constructStatics();

    // Variables
    prim::LatticeCoord lat_coord; // lattice coordinates of the DB Preview

    // Static class variables
    static QColor fill_col;
    static QColor edge_col;

    static qreal diameter;
    static qreal edge_width;
    static qreal fill_fact;
  };

} // end prim namespace



#endif
