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


void SimManager::newSimSetup()
{
  // TODO dialog with simulation parameter settings

  // for now, just jump directly to export stage
  exportSimProblem(); // todo: indicate path
}



// PRIVATE

void SimManager::initSimManager()
{
  // init simulation manager GUI

  // simulator manager panes
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


  setWindowTitle(tr("Simulation Manager"));
}

void SimManager::initMenu()
{}

void SimManager::initListPan()
{}

void SimManager::initSimActionsPan()
{
  QPushButton *new_simulation = new QPushButton(tr("&New Simulation"));
  QPushButton *close_button = new QPushButton(tr("Close"));

  connect(new_simulation, &QAbstractButton::clicked, this, &gui::SimManager::newSimSetup);
  connect(close_button, &QAbstractButton::clicked, this, &QWidget::hide);

  close_button->setShortcut(tr("Esc"));

  sim_actions_pan->addWidget(new_simulation);
  sim_actions_pan->addWidget(close_button);
  sim_actions_pan->addStretch(1);
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
  // returns whether export is successful
  //return static_cast<gui::ApplicationGUI*>(parent())->saveToFile(parent()->SaveFlag::Simulation, "problem_export.xml"); // TODO change file name
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

  return true; // placeholder
}

} // end gui namespace
