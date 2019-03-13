// @file:     sim_visualizer.h
// @author:   Samuel
// @created:  2017.10.12
// @license:  GNU LGPL v3
//
// @desc:     Widget that controls simulator output display

#ifndef _GUI_SIM_VISUALIZER_H_
#define _GUI_SIM_VISUALIZER_H_

#include <QtWidgets>
#include <QtCore>
#include "../components/sim_job.h"
#include "../design_panel.h"
#include "electron_config_set_visualizer.h"
#include "potential_landscape_visualizer.h"


namespace gui{

  //! Visualize simulation results supplied by job manager.
  class SimVisualizer : public QWidget
  {
    Q_OBJECT

  public:

    //! Constructor.
    SimVisualizer(gui::DesignPanel *design_pan, QWidget *parent=nullptr);

    //! Destructor.
    ~SimVisualizer() {};

    //! Show the simulation results of the provided job.
    void showJob(comp::SimJob *job);

    //! Clear the job result from this and children visualizers.
    void clearJob();

    //! Return the result types supported by this class.
    QList<comp::JobResult::ResultType> supportedResultTypes()
    {
      return QList<comp::JobResult::ResultType>({
            comp::JobResult::DBLocationsResult,
            comp::JobResult::ElectronConfigsResult,
            comp::JobResult::PotentialLandscapeResult
          });
    }

    // TODO ability to choose job step

    // TODO ability to combine results from multiple job steps (e.g. electron
    // configuration overlaid on potential landscape).

    // TODO further stretch - quick step (animation) from multiple job steps of 
    // the same result type.

    // TODO for real time potential interaction, the potential plot object can 
    // point to results in the PotentialResults object and show the potential 
    // at the indicated location using a tooltip or something similar.

    /* TODO offload job terminal to job manager
    void showJobTerminalOutput();
    void saveJobTerminalOutput();
    */

    //! TODO plot paths should be returned through the sim result file.
    /*
    void showPotPlot();

    void updateJobSelCombo();
    bool showElecDist(int dist_ind);  // show elec charges on design panel
    void showElecCountFilter(int);    // show or hide electron count filter
    void showAverageElecDist();       // average the elec charges when showing
    void showAverageElecDistDegen();  // average the elec charges of degenerate states
    void updateElecDistOptions();
    void updateViewPotentialOptions();
    void updateOptions();
    void showPotential();
    // TODO generate a list of DBDot* with the same order as physlocs in show_job
    */

  signals:
    void showElecDistOnScene(comp::SimJob *job, int dist_ind, bool avg_degen=false);
    void showPotPlotOnScene(QString pot_plot_path, QRectF graph_container, QString pot_anim_plot);
    void clearPotPlots();

  private:

    gui::DesignPanel *design_pan;             // pointer to the design panel
    comp::SimJob *sim_job=nullptr;            // current job result being shown

    ElectronConfigSetVisualizer *elec_config_set_visualizer;
    PotentialLandscapeVisualizer *pot_landscape_visualizer;

    QGroupBox *gb_job_info;                   // group box containing job information elements
    QGroupBox *gb_elec_configs;               // group box containing electron config elements
    QGroupBox *gb_pot_landscape;              // group box containing potential landscape elements

    QTableView *tv_job_info;                  // table view showing job details
    QStandardItemModel *job_info_model;       // model storing the job's details

    //QComboBox *cb_job_steps_db_locs;          // job steps containing DB locations
    QComboBox *cb_job_steps_elec_configs;     // job steps containing electron configurations
    QComboBox *cb_job_steps_pot_landscape;    // job steps containing potential landscape

    /*
    void initSimVisualizer();

    //! Job selection update function.
    void jobSelUpdate();

    //! Electron count filter update.
    void elecCountFilterUpdate(bool apply_filter);

    //! Choose previous electron count.
    void elecCountPrev();

    //! Choose next electron count.
    void elecCountNext();

    //! Electron distribution update.
    void distSelUpdate();

    //! Choose previous electron distribution.
    void distPrev();

    //! Choose next electron distribution.
    void distNext();

    // variables
    QComboBox *combo_job_sel;
    QLabel *text_job_engine;
    QLabel *text_job_start_time;
    QLabel *text_job_end_time;
    QPushButton *button_show_term_out;

    // electron distribution
    QGroupBox *dist_group;
    QSlider *slider_elec_count_sel;
    QSlider *slider_dist_sel;
    QLabel *text_elec_count;
    QLabel *text_dist_selected;
    QLabel *text_dist_energy;
    QGroupBox *elec_count_filter_group;
    QCheckBox *cb_elec_count_filter;

    // potential viewer
    QGroupBox *view_potential_group;
    QLabel *potential_window;
    */
  };

} // end of gui namespace

#endif
