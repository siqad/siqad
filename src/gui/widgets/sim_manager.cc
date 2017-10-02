// @file:     sim_manager.cc
// @author:   Samuel
// @created:  2017.09.27
// @editted:  2017.09.27 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     window that allows users to setup and dispatch new simulations,
//            as well as manage ongoing or completed simulations.

#include "sim_manager.h"


namespace gui{

// Qt::Dialog makes the main window unclickable. Use Qt::Window if this behavior should be changed.
SimManager::SimManager(QWidget *parent)
  : QWidget(parent, Qt::Dialog)
{
  initSimManager();
}

SimManager::~SimManager()
{}

bool SimManager::toggleManagerVisibility()
{
  // show or hide the manager dialog window
  // use QWidget::hide() and QWidget::show() I think
}

bool SimManager::installSim()
{}


// PRIVATE

void SimManager::initSimManager()
{
  // init simulation manager GUI

  // simulation list pane
  sim_list_pan = new QListWidget;
  // TODO blank for now


  // simulation actions pane
  sim_actions_pan = new QListWidget;


  // combine widgets into main layout
  QHBoxLayout *sim_man_main = new QHBoxLayout;
  sim_man_main->addWidget(sim_list_pan);
  sim_man_main->addWidget(sim_actions_pan);
  setLayout(sim_man_main);

  setWindowTitle(tr("Simulation Manager"));
}

void SimManager::fetchSimList()
{
  // fetch a list of available simulators
  // save to simulators stack
}

void SimManager::simParamSetup()
{
  // take user options for simulation parameters
}

bool SimManager::exportSimProblem()
{
  // call save function in application.cc with path going to appropriate directory (still need to finalize directory)
}

void SimManager::invokeSimulator()
{
  // invoke simulator binary and display text outputs here
}

bool SimManager::checkSimCompletion()
{
  // ideally, should be able to parse the output from the simulation for indication of succeess
  // if not, just check when the output file is complete

  // also, detect error from the simulator, alert the user if error occurs during simulation
}

} // end gui namespace
