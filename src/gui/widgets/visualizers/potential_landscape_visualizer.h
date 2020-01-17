// @file:     potential_landscape_visualizer.h
// @author:   Samuel
// @created:  2019.03.22
// @license:  GNU LGPL v3
//
// @desc:     Widgets for visualizing potential landscape.

#ifndef _GUI_POT_LANDSCAPE_VIS_H_
#define _GUI_POT_LANDSCAPE_VIS_H_

#include <QtWidgets>
#include "../design_panel.h"
#include "gui/widgets/primitives/items.h"
#include "gui/widgets/components/job_results/potential_landscape.h"

namespace gui{

  //! Visualize potential landscape simulation results.
  class PotentialLandscapeVisualizer : public QWidget
  {
    Q_OBJECT

  public:

    //! Constructor.
    PotentialLandscapeVisualizer(DesignPanel *design_pan, QWidget *parent=nullptr);

    //! Destructor.
    ~PotentialLandscapeVisualizer() {};

    //! Reset the widget, clearing out all existing information.
    void clearVisualizer();

    //! Set the current potential landscape
    void setPotentialLandscape(comp::PotentialLandscape *t_pot_landscape);

    //! Show the potential landscape image / GIF on design panel.
    void showPotentialResultOverlay();

    //! Clear the potential landscape image / GIF from design panel.
    void clearPotentialResultOverlay();

  private:

    // non-widget variables
    DesignPanel *design_pan;                    // pointer to the design panel
    comp::PotentialLandscape *pot_landscape;    // currently active potential landscape result
    QList<prim::PotPlot> pot_plots;             // potential plots currently shown on screen

    // widget variables
    QLabel *l_has_static_plot;
    QLabel *l_has_animation;

  };


} // end of gui namespace

#endif
