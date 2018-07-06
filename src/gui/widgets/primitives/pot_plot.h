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

  //! An item that implements a colour map of the electrostatic potential due to
  //! electrodes in the system.
  class PotPlot: public prim::Item
  {
  public:
    //! constructor, create a PotPlot given a QPixmap of the plot,
    //! and a QRectF to contain it.
    PotPlot(QString pot_plot_path, QRectF graph_container, QString pot_anim_path);

    //! destructor
    ~PotPlot();

    //initializer
    void initPotPlot(QString pot_plot_path, QRectF graph_container_in, QString pot_anim_path);


    QImage getPotentialPlot(void){return potential_plot;}
    QMovie *getPotentialAnimation(void){return potential_animation;}
    QRectF getGraphContainer(void){return graph_container;}
    QString getPotPlotPath(void){return pot_plot_path;}
    QString getAnimPath(void){return pot_anim_path;}
    void updateSimMovie();
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
    QImage potential_plot;
    QRectF graph_container;
    QMovie *potential_animation;
    QString pot_plot_path;
    QString pot_anim_path;
    static qreal edge_width;  // proportional width of dot boundary edge
    static QColor fill_col;   // dot fill color (same for all lattice dots)
    static QColor edge_col;     // edge colour, unselected
    static QColor selected_col; // edge colour, selected
  };

} // end prim namespace

#endif
//////
