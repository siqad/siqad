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
    bool setJob(int job_ind);
    bool setJob(prim::SimJob *job);
    void updateJobList();
    void updateOptions();

    // TODO generate a list of DBDot* with the same order as physlocs in curr_job


  private:
    void initSimVisualize();
    SimManager *sim_manager;
    prim::SimJob *curr_job;   // current job result being shown
    // TODO struct of display options that users can change, as well as the available range of each option

    // variables
    QComboBox *combo_job_sel;
  };

} // end of gui namespace

#endif
