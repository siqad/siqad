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
    bool showElecDist(int dist_ind);
    void updateElecDistOptions();
    void updateOptions();

    // TODO generate a list of DBDot* with the same order as physlocs in show_job

  signals:
    void showElecDistOnScene(prim::SimJob *job, int dist_ind);
    void showPotPlotOnScene(QPixmap potential_plot, QRectF graph_container);

  private:
    void initSimVisualize();
    void jobSelUpdate();
    void distSelUpdate();
    void distPrev();
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

    QSlider *slider_dist_sel;
    QLabel *text_dist_selected;
  };

} // end of gui namespace

#endif
