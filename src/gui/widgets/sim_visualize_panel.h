// @file:     sim_visualize_panel.h
// @author:   Samuel
// @created:  2017.10.12
// @editted:  2017.10.12 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Widget that controls simulator output display

#ifndef _PRIM_SIM_DIS_H_
#define _PRIM_SIM_DIS_H_

#include <QtWidgets>
#include <QtCore>
#include "sim_manager.h"
#include "primitives/sim_job.h"

#include "src/settings/settings.h" // TODO probably need this later

namespace gui{

  class SimVisualize : public QWidget
  {
    Q_OBJECT
  public:
    // constructor
    SimVisualize(SimManager *sim_man=0, QWidget *parent=0);

    // destructor
    ~SimVisualize() {};

    // set job
    bool setManager(SimManager *sim_man);
    bool showJob(int job_ind);
    bool showJob(prim::SimJob *job);

    void showJobTerminalOutput();
    void saveJobTerminalOutput();

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

  signals:
    void showElecDistOnScene(prim::SimJob *job, int dist_ind, bool avg_degen=false);
    void showPotPlotOnScene(QImage potential_plot, QRectF graph_container);
    void clearPotPlots();

  private:
    void initSimVisualize();

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

    SimManager *sim_manager;
    prim::SimJob *show_job;   // current job result being shown
    // TODO struct of display options that users can change, as well as the available range of each option

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
  };

} // end of gui namespace

#endif
