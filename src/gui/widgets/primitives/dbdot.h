// @file:     dbdot.h
// @author:   Jake
// @created:  2016.11.15
// @editted:  2017.06.07  - Jake
// @license:  GNU LGPL v3
//
// @desc:     DBDot Widget for functionality of dangling bonds

#ifndef _GUI_PR_DBDOT_H_
#define _GUI_PR_DBDOT_H_


#include <QtWidgets>

#include "item.h"
#include "latdot.h"



namespace prim{

  // Specific Item derived class for a dangling bond on the lattice. Each
  // dangling bond should correspond to a source lattice dot. For
  // generality, each DBDot has its own physical location that will typically
  // be the same as the source LatticeDot.

  class DBDot: public prim::Item
  {
  public:

    // constructor
    DBDot(int lay_id, prim::LatticeDot *src=0, int elec_in=0);
    DBDot(QXmlStreamReader *, QGraphicsScene *);
    void initDBDot(int lay_id, prim::LatticeDot *src=0, int elec_in=0);

    // destructor
    ~DBDot(){}

    // accessors
    QPointF getPhysLoc() const {return phys_loc;}

    void toggleElec();
    void setElec(int e_in);
    int getElec() {return elec;}
    void setShowElec(int se_in);

    void setSource(prim::LatticeDot *src);
    prim::LatticeDot *getSource() const {return source;}

    void setFill(float fill){fill_fact = fill;}
    void setFillCol(QColor col, QColor col_sel){fill_col = col; fill_col_sel = col_sel;}

    // inherited abstract method implementations

    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;

    prim::Item *deepCopy() const;

    // SAVE LOAD
    virtual void saveItems(QXmlStreamWriter *) const;

  protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;

  private:

    // construct static variables
    void constructStatics();

    // VARIABLES
    QPointF phys_loc;         // physical location of dot in angstroms
    int elec;                 // 1=forced electron on this db
    int show_elec;            // simulation result visualization electron, 1=has electron

    prim::LatticeDot *source=0; // lattice site beneath dot

    qreal fill_fact;          // area proportional of dot filled
    QColor fill_col;          // color of fill that is drawn
    QColor fill_col_sel;      // color of fill when selected that is drawn
    static QColor fill_col_default;  // default fill color
    static QColor fill_col_default_sel;  // default fill color (selected)
    static QColor fill_col_drv;      // driver fill color
    static QColor fill_col_drv_sel;  // driver fill color (selected)
    static QColor fill_col_elec;
    static QColor fill_col_elec_sel;

    // static class parameters for painting

    qreal diameter;      // dot diameter in angstroms
    static qreal diameter_m;    // medium sized dot
    static qreal diameter_l;    // large sized dot
    static qreal edge_width;    // proportional width of dot boundary edge

    static QColor edge_col;     // edge colour, unselected
    static QColor selected_col; // edge colour, selected
  };

} // end prim namespace



#endif
