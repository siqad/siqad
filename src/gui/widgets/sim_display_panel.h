// @file:     sim_display.h
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
#include "src/settings/settings.h" // TODO probably need this later

namespace gui{

  class SimDisplay : public QWidget
  {
    Q_OBJECT
  public:
    // constructor
    SimDisplay(prim::SimJob *job=0, QSomething *parent=0);

    // destructor
    ~SimDisplay();

    // set job
    bool setJob(prim::SimJob *job) {return job ? curr_job = job : false;} // TODO not sure if this actually works




  private:
    prim::SimJob *curr_job;   // current job result being shown
    // TODO struct of display options that users can change, as well as the available range of each option
  };

} // end of gui namespace

#endif
