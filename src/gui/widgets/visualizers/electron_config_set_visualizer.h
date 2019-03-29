// @file:     electron_config_set_visualizer.h
// @author:   Samuel
// @created:  2019.03.20
// @license:  GNU LGPL v3
//
// @desc:     Widgets for visualizing electron config sets.

#ifndef _GUI_ELEC_CONFIG_SET_VIS_H_
#define _GUI_ELEC_CONFIG_SET_VIS_H_

#include <QtWidgets>
#include "../design_panel.h"
#include "src/gui/widgets/components/job_results/electron_config_set.h"
#include "src/gui/widgets/primitives/dbdot.h"

namespace gui{

  //! Visualize electron config set simulation results.
  class ElectronConfigSetVisualizer : public QWidget
  {
    Q_OBJECT

  public:

    //! Constructor.
    ElectronConfigSetVisualizer(DesignPanel *design_pan, QWidget *parent=nullptr);

    //! Destructor.
    ~ElectronConfigSetVisualizer() {};

    //! Reset the widget, clearing out all existing information.
    void clearVisualizer();

    //! Set a new ElectronConfigSet (which contains all electron configurations).
    //! most_popular_elec_count instructs whether to default to filtering for 
    //! the most popular electron count in the results.
    //! bool value show_results_now instructs visualizer whether to immediately
    //! take focus and show results.
    void setElectronConfigSet(comp::ElectronConfigSet *t_set,
                              bool most_popular_elec_count=true,
                              bool show_results_now=true);

    //! Set a new electron config list (which contains electron configurations
    //! with applied filters, sort rules, etc.) Without filter, the list would
    //! just be the list returned by elec_config_set->electronConfigs().
    void setElectronConfigList(const QList<comp::ElectronConfigSet::ElectronConfig> &ec);

    //! Show the electron config specified by the current slider location.
    void showElectronConfigResultFromSlider();

    //! Show the specified electron config.
    void showElectronConfigResult(const comp::ElectronConfigSet::ElectronConfig &elec_config,
                                  const QList<QPointF> &db_phys_locs);

    //! Apply an electron count filter with the given electron count. If the 
    //! specified count doesn't agree with the current count value on the 
    //! slider, the slider is updated to match with the given count.
    //! If elec_count is negative, use the slider value.
    void applyElectronCountFilter(const int &elec_count=-1);

    //! Clear the electron configurations from DB sites currently under this 
    //! widget's influence.
    void clearElectronConfigResult();

    //! Create a scatter plot for the electron config set (energy vs elec count).
    //! NOTE only a proof of concept implementation so far, not shown on the GUI
    //! for now. Proper implementation should subclass QChartView and have the
    //! chart interact with SimVisualizer through signals.
    QWidget *scatterPlotElectronConfigSet();


  private:

    //! Update GUI in response to a config set change.
    void updateGUIConfigSetChange();

    //! Update GUI in response to a config list change.
    void updateGUIConfigListChange();

    //! Update GUI in response to a config selection change.
    void updateGUIConfigSelectionChange(const int &elec_config_list_ind);

    //! Update GUI in response to a filter enable state change.
    void updateGUIFilterStateChange();

    //! Update GUI in response to a filter selection change.
    void updateGUIFilterSelectionChange(const int &elec_count);




    //! Update the electron config set GUI elements following a change in the 
    //! current config set.
    //void updateElectronConfigSetSelectionGUI();

    //! Update l_elec_config_set_ind text. Provide the elec_config_list_ind, 
    //! the plus 1 (so index starts at 1 from the user's point of view) is done
    //! inside this function.
    //void updateElectronConfigSetIndexLabel(const int &elec_config_list_ind);

    //! Update l_elec_count_filter_curr_num.
    //void updateElectronCountFilterLabel(const int &elec_count);

    // non-GUI variables
    DesignPanel *design_pan;                    // pointer to the design panel
    // current electron config set (contains all information about this config)
    comp::ElectronConfigSet *elec_config_set=nullptr;
    // current electron config list (filtered/sorted/etc.)
    QList<comp::ElectronConfigSet::ElectronConfig> elec_config_list;
    // current electron config being shown
    comp::ElectronConfigSet::ElectronConfig curr_elec_config;
    QList<prim::DBDot*> showing_db_sites;       // DB sites currently controlled by visualizer

    // GUI variables
    QLabel *l_energy_val;                     // energy of a configuration
    QLabel *l_elec_count_val;                 // electron count of a configuration
    QLabel *l_pop_occ;                        // population occurances
    QLabel *l_config_occ;                     // configuration occurances

    // config set selection
    QLabel *l_elec_config_set_ind;            // electron config set index within slider
    QWidget *w_config_slider_complex;         // widget storing config slider complex (slider and buttons)
    QSlider *s_elec_config_list;              // slider to choose which config to show

    // filter selection
    QCheckBox *cb_elec_count_filter;          // checkbox for enabling electron count filter
    QWidget *w_elec_count_slider_complex;     // widget storing filter slider complex (slider and buttons)
    QSlider *s_elec_count_filter;             // slider to choose electron count filter


  };  // end of ElectronConfigSetVisualizer

} // end of gui namespace

#endif
