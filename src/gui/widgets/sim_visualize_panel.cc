// @file:     sim_visualize_panel.h
// @author:   Samuel
// @created:  2017.10.03
// @editted:  2017.10.03 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     SimVisualize classes

#include "sim_visualize_panel.h"


namespace gui{

// Qt::Dialog makes the main window unclickable. Use Qt::Window if this behavior should be changed.
SimVisualize::SimVisualize(prim::SimJob *job, QWidget *parent)
  : QWidget(parent, Qt::Widget), curr_job(job)
{
  initSimVisualize();
}



bool SimVisualize::setJob(prim::SimJob *job)
{
  if(!job)
    return false;
  curr_job = job;
  updateOptions();
  return true;
}


void SimVisualize::updateOptions()
{
  if(!curr_job){
    // TODO don't show any options
  }
  else if(!curr_job->isComplete()){
    // TODO show that the job is not complete
    // maybe also set this up to catch job completion signals, so when a job is complete this panel also updates?
  }
  else{
    // TODO actually update the options
  }
}




// PRIVATE

void SimVisualize::initSimVisualize()
{
  // always show job selection
  // if no job is selected, show nothing else - easiest for now. In the future maybe still show options but grayed out
  // if an incomplete job is selected, say it's incomplete
  // else show actual stuff

  // TODO for now, only show options to choose which result to show, maybe get rid of duplicated ones but keep the count of occurance for duplicated results
  // TODO next, show energy level, and maybe sorting feature
  // TODO that's about time to make slides

}


} // end gui namespace
