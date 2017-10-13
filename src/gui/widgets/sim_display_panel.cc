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
  showing_job = job;
  // TODO if(curr_job->isComplete()), update display option ranges
  return true;
}




// PRIVATE

void SimDisplay::initSimDisplay()
{
  // TODO for now, only show options to choose which result to show, maybe get rid of duplicated ones but keep the count of occurance for duplicated results
  // TODO next, show energy level, and maybe sorting feature
  // TODO that's about time to make slides

}


} // end gui namespace
