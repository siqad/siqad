/** @file:     pot_plot.h
 *  @author:   Nathan
 *  @created:  2017.10.27
 *   @editted:  2018.01.17 - Nathan
 *  @license:  GNU LGPL v3
 *
 *  @brief:     Function prototypes for the PotPlot object.
 */

#ifndef _GUI_PR_POT_PLOT_H_
#define _GUI_PR_POT_PLOT_H_

#include <QtWidgets>
#include "item.h"

namespace prim{

  // forward declarations
  class Layer;

  //! An item that implements a colour map of the electrostatic potential due to
  //! electrodes in the system.
  class PotPlot: public prim::Item
  {
  public:
    //! constructor, create a PotPlot given a QPixmap of the plot,
    //! and a QRectF to contain it.
    PotPlot(int lay_id, QPixmap potential_plot, QRectF graph_container);

    //! destructor
    ~PotPlot(){}

    //initializer
    void initPotPlot(int lay_id, QPixmap potential_plot, QRectF graph_container);
    QPixmap getPotentialPlot(void){return potential_plot;}
    QRectF getGraphContainer(void){return graph_container;}
    // inherited abstract method implementations
    QRectF boundingRect() const override;
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;
    Item *deepCopy() const override;

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
