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
  initSimSetupDialog();
}

SimManager::~SimManager()
{}


void SimManager::showSimSetupDialog()
{
  //updateSimSetupDialog();
  sim_setup_dialog->show();
}

void SimManager::newSimSetup()
{
  // TODO dialog with simulation parameter settings

  // for now, just jump directly to export stage
  exportSimProblem(); // todo: indicate path
}


bool SimManager::addJob(prim::SimJob *job)
{
  if(!job)
    return false;
  sim_jobs.prepend(job);
  return true;
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


void SimManager::initSimSetupDialog()
{
  qDebug() << tr("Entered initSimSetupDialog");
  //if(sim_setup_dialog)
  //  delete sim_setup_dialog;

  sim_setup_dialog = new QWidget(this, Qt::Dialog);
  qDebug() << tr("Created sim setup dialog");
  
  // Engine Select Group
  QGroupBox *engine_sel_group = new QGroupBox(tr("Engine Selection"));
  QLabel *label_eng_sel = new QLabel(tr("Engine:"));
  /* for now, hard code SimAnneal as the only available engine
  combo_eng_sel = new QComboBox();
  combo_eng_sel->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  updateEngSelCombo();*/
  QLabel *text_eng_sel = new QLabel(tr("SimAnneal"));
  label_eng_sel->setBuddy(text_eng_sel);
  QHBoxLayout *eng_sel_hl = new QHBoxLayout;
  eng_sel_hl->addWidget(label_eng_sel);
  eng_sel_hl->addWidget(text_eng_sel);

  QVBoxLayout *engine_sel_vl = new QVBoxLayout;
  engine_sel_vl->addLayout(eng_sel_hl);

  engine_sel_group->setLayout(engine_sel_vl);
  qDebug() << tr("engine_sel_group done");

  // Sim Params Group
  QGroupBox *sim_params_group = new QGroupBox(tr("Simulation Parameters"));

  // hard coded fields for now
  QLabel *label_preanneal_cycles = new QLabel("Preanneal Cycles:");
  QLabel *label_anneal_cycles = new QLabel("Anneal Cycles:");
  QLabel *label_global_v0 = new QLabel("Global Bias v_0:");

  QLineEdit *le_preanneal_cycles = new QLineEdit("1000"); // TODO these default values should be read from the engine description file
  QLineEdit *le_anneal_cycles = new QLineEdit("10000");
  QLineEdit *le_global_v0 = new QLineEdit("0");

  label_preanneal_cycles->setBuddy(le_preanneal_cycles);
  label_anneal_cycles->setBuddy(le_anneal_cycles);
  label_global_v0->setBuddy(le_global_v0);

  QHBoxLayout *preanneal_cycles_hl = new QHBoxLayout;
  QHBoxLayout *anneal_cycles_hl = new QHBoxLayout;
  QHBoxLayout *global_v0_hl = new QHBoxLayout;
  preanneal_cycles_hl->addWidget(label_preanneal_cycles);
  preanneal_cycles_hl->addWidget(le_preanneal_cycles);
  anneal_cycles_hl->addWidget(label_anneal_cycles);
  anneal_cycles_hl->addWidget(le_anneal_cycles);
  global_v0_hl->addWidget(label_global_v0);
  global_v0_hl->addWidget(le_global_v0);

  QVBoxLayout *sim_params_vl = new QVBoxLayout;
  sim_params_vl->addLayout(preanneal_cycles_hl);
  sim_params_vl->addLayout(anneal_cycles_hl);
  sim_params_vl->addLayout(global_v0_hl);

  sim_params_group->setLayout(sim_params_vl);
  qDebug() << tr("sim_params_group done");

  // Bottom Buttons
  QPushButton *button_run = new QPushButton(tr("&Run"));
  QPushButton *button_close = new QPushButton(tr("Close"));

  connect(button_run, &QAbstractButton::clicked, this, &gui::SimManager::submitSimSetup);
  connect(button_close, &QAbstractButton::clicked, sim_setup_dialog, &QWidget::hide);

  button_close->setShortcut(tr("Esc"));

  QHBoxLayout *bottom_buttons_hl = new QHBoxLayout;
  bottom_buttons_hl->addStretch(1);
  bottom_buttons_hl->addWidget(button_close);
  bottom_buttons_hl->addWidget(button_run);

  // bring it together
  QVBoxLayout *new_setup_dialog_l = new QVBoxLayout;
  new_setup_dialog_l->addWidget(engine_sel_group);
  new_setup_dialog_l->addWidget(sim_params_group);
  new_setup_dialog_l->addLayout(bottom_buttons_hl);

  sim_setup_dialog->setLayout(new_setup_dialog_l);
}


void SimManager::updateSimSetupDialog()
{
  // update dialog content with engine selection
  // useless for now as there's only one engine
}


void SimManager::submitSimSetup()
{
  // TODO change this to: make some dictionary that contains everything that should be exported in XML,
  //      when application.cc exports save file, it calls sim_manager to add appropriate simulation params.

  // TODO add parameters to QStringList according according to input text
  QStringList arguments;
  arguments << "-p"; // preanneal cycles
  
  arguments << "-a"; // anneal cycles

  arguments << "-v"; // v_0

  emit runSimWithArg(arguments);
}


void SimManager::fetchEngineList()
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
  return true;
}

} // end gui namespace
