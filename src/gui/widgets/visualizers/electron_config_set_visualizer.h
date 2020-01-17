// @file:     electron_config_set_visualizer.h
// @author:   Samuel
// @created:  2019.03.20
// @license:  GNU LGPL v3
//
// @desc:     Widgets for visualizing charge config sets.

#ifndef _GUI_CHRG_CONFIG_SET_VIS_H_
#define _GUI_CHRG_CONFIG_SET_VIS_H_

#include <QtWidgets>
#include "../design_panel.h"
#include "gui/widgets/components/job_results/electron_config_set.h"
#include "gui/widgets/primitives/dbdot.h"

namespace gui{

  //! Visualize charge config set simulation results.
  class ChargeConfigSetVisualizer : public QWidget
  {
    Q_OBJECT

  public:

    enum PreferredSelection{LowestPhysicallyValidState, LowestInMostPopularNetCharge};

    //! Constructor.
    ChargeConfigSetVisualizer(DesignPanel *design_pan, QWidget *parent=nullptr);

    //! Destructor.
    ~ChargeConfigSetVisualizer() {};

    //! Reset the widget, clearing out all existing information.
    void clearVisualizer();

    //! Set a new ChargeConfigSet (which contains all charge configurations).
    //! most_popular_elec_count instructs whether to default to filtering for 
    //! the most popular charge count in the results.
    //! bool value show_results_now instructs visualizer whether to immediately
    //! take focus and show results.
    void setChargeConfigSet(comp::ChargeConfigSet *t_set,
                            bool show_results_now=true,
                            PreferredSelection preferred_sel=LowestPhysicallyValidState);

    //! Set a new charge config list (which contains charge configurations
    //! with applied filters, sort rules, etc.) Without filter, the list would
    //! just be the list returned by charge_config_set->chargeConfigs().
    void setChargeConfigList(const QList<comp::ChargeConfigSet::ChargeConfig> &ec);

    //! Show the charge config specified by the current slider location.
    void showChargeConfigResultFromSlider();

    //! Show the specified charge config. db_fill indicates DB fill state for
    //! showing partial fills in the case of degenerate state visualization, 
    //! leave empty to show just the charge_config.
    void showChargeConfigResult(const comp::ChargeConfigSet::ChargeConfig &charge_config,
                                  const QList<QPointF> &db_phys_locs,
                                  const QList<float> &db_fill=QList<float>());

    //! Color in degenerate states.
    void visualizeDegenerateStates(const comp::ChargeConfigSet::ChargeConfig &charge_config);

    //! Apply an charge count filter with the given net charge count. 
    //! If use_slider is set to true, then the slider value is applied.
    //! Otherwise, the specified net_charge filter is spplied.
    void applyNetChargeFilter(const bool &use_slider=true, const int &net_charge=0);

    //! Clear the charge configurations from DB sites currently under this 
    //! widget's influence.
    void clearChargeConfigResult();

    //! Create a scatter plot for the charge config set (energy vs net_charge count).
    //! NOTE only a proof of concept implementation so far, not shown on the GUI
    //! for now. Proper implementation should subclass QChartView and have the
    //! chart interact with SimVisualizer through signals.
    QWidget *scatterPlotChargeConfigSet();


  private:

    //! Update GUI in response to a config set change.
    void updateGUIConfigSetChange();

    //! Update GUI in response to a config list change.
    void updateGUIConfigListChange();

    //! Update GUI in response to a config selection change.
    void updateGUIConfigSelectionChange(const int &charge_config_list_ind);

    //! Update GUI in response to a filter enable state change.
    void updateGUIFilterStateChange();

    //! Update GUI in response to a filter selection change.
    void updateGUIFilterSelectionChange(const int &net_charge);


    // non-GUI variables
    DesignPanel *design_pan;                    // pointer to the design panel
    // current charge config set (contains all information about this config)
    comp::ChargeConfigSet *charge_config_set=nullptr;
    // current charge config list (filtered/sorted/etc.)
    QList<comp::ChargeConfigSet::ChargeConfig> charge_config_list;
    // current charge config being shown
    comp::ChargeConfigSet::ChargeConfig curr_charge_config;
    QList<prim::DBDot*> showing_db_sites;       // DB sites currently controlled by visualizer

    // GUI variables
    QLabel *l_energy_val;                     // energy of a configuration
    QLabel *l_net_charge_val;               // charge count of a configuration
    QLabel *l_is_valid;                       // is physically valid
    QLabel *l_pop_occ;                        // population occurances
    QLabel *l_config_occ;                     // configuration occurances

    // config set selection
    QLabel *l_charge_config_set_ind;          // charge config set index within slider
    QWidget *w_config_slider_complex;         // widget storing config slider complex (slider and buttons)
    QSlider *s_charge_config_list;            // slider to choose which config to show

    // filter selection
    QPushButton *pb_degenerate_states;        // show degenerate states
    QCheckBox *cb_net_charge_filter;        // checkbox for enabling charge count filter
    QWidget *w_net_charge_slider_complex;   // widget storing filter slider complex (slider and buttons)
    QSlider *s_net_charge_filter;           // slider to choose charge count filter

    // filter physically valid states
    QCheckBox *cb_phys_valid_filter;          // checkbox for enabling physically valid state filter

  };  // end of ChargeConfigSetVisualizer

} // end of gui namespace

#endif
