// @file:     sim_handler.cc
// @author:   Samuel
// @created:  2017.09.27
// @editted:  2017.09.27 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     window that allows users to setup and dispatch new simulations,
//            as well as manage ongoing or completed simulations.

#include "sim_handler.h"


namespace gui{

// Qt::Dialog makes the main window unclickable. Use Qt::Window if this behavior should be changed.
SimHandler::SimHandler(QWidget *parent)
  : QWidget(parent, Qt::Dialog)
{
  initSimHandler();
}

SimHandler::~SimHandler()
{}

void SimHandler::initSimHandler()
{
  // init simulation handler GUI
}

void SimHandler::simUserOptions()
{
  // take user options for simulation parameters
}

bool SimHandler::exportSimProblem()
{
  // call save function in application.cc with path going to appropriate directory (still need to finalize directory)
}

void SimHandler::invokeSimulator()
{
  // invoke simulator binary and display text outputs here
}

bool SimHandler::checkSimCompletion()
{
  // ideally, should be able to parse the output from the simulation for indication of succeess
  // if not, just check when the output file is complete

  // also, detect error from the simulator, alert the user if error occurs during simulation
}

} // end gui namespace
