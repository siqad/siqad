// @file:     sim_engine.h
// @author:   Samuel
// @created:  2017.10.03
// @editted:  2017.10.03 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     SimDisplay classes

#include "sim_display.h"


namespace gui{

// Qt::Dialog makes the main window unclickable. Use Qt::Window if this behavior should be changed.
SimDisplay::SimDisplay(prim::SimJob *job, QSomething *parent)
  : QWidget(parent, Qt::Dialog), curr_job(job) // maybe not Qt::Dialog
{
  initSimDisplay();
}

SimDisplay::~SimDisplay()
{}


bool SimDisplay::setJob(prim::SimJob *job)
{
  if(!job)
    return false;
  curr_job = job;
  // TODO if(curr_job->isComplete()), update display option ranges
  return true;
}



// PRIVATE

void SimDisplay::initSimDisplay()
{
  // init simulation manager GUI

  /*// simulator manager panes
  sim_list_pan = new QListWidget();
  sim_actions_pan = new QVBoxLayout();

  // populate panes
  initMenu();
  initListPan();
  initSimActionsPan();

  // simulator manager layout
  QHBoxLayout *man_main = new QHBoxLayout();
  man_main->addWidget(sim_list_pan);
  man_main->addLayout(sim_actions_pan);

  setLayout(man_main);


  setWindowTitle(tr("Simulation Manager"));*/
}


} // end gui namespace
