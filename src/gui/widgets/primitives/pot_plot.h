// @file:     pot_plot.h
// @author:   Nathan
// @created:  2017.10.27
// @editted:  2018.01.17 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     pot_plot objects for creation of potential plots on design panel.

#ifndef _GUI_PR_POT_PLOT_H_
#define _GUI_PR_POT_PLOT_H_

#include <QtWidgets>
#include "item.h"

namespace prim{

  // forward declarations
  class Layer;

  class PotPlot: public prim::Item
  {
  public:
    // constructor, create an electrode given two points
    PotPlot(int lay_id, QPixmap potential_plot, QRectF graph_container);

    // destructor
    ~PotPlot(){}

    //initializer
    void initPotPlot(int lay_id, QPixmap potential_plot, QRectF graph_container);

    // inherited abstract method implementations
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;
    Item *deepCopy() const;

  // protected:
  //   virtual void mousePressEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;
  //   virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;

  private:
    // construct static variables
    void constructStatics();

    // VARIABLES
    QPixmap potential_plot;
    QRectF graph_container;

    static qreal edge_width;  // proportional width of dot boundary edge
    static QColor fill_col;   // dot fill color (same for all lattice dots)
    static QColor edge_col;     // edge colour, unselected
    static QColor selected_col; // edge colour, selected
  };

} // end prim namespace

#endif
//////
