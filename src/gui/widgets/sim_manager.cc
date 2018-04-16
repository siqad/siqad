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
  initEngines();
  initSimManager();
  initSimSetupDialog();
}

SimManager::~SimManager()
{
  for(prim::SimEngine *engine : sim_engines)
    delete engine;
  sim_engines.clear();

  for(prim::SimJob *job : sim_jobs)
    delete job;
  sim_jobs.clear();
}


void SimManager::showSimSetupDialog()
{
  sim_setup_dialog->show();
  updateJobNameDateTime();
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


prim::SimEngine *SimManager::getEngine(const QString &name)
{
  for (prim::SimEngine *engine : sim_engines) {
    if (engine->name() == name)
      return engine;
  }
  // not found
  return 0;
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
  sim_setup_dialog = new QWidget(this, Qt::Dialog);

  // Engine Select Group, can be written here since only done once.
  QGroupBox *engine_sel_group = new QGroupBox(tr("Engine Selection"));
  QLabel *label_eng_sel = new QLabel(tr("Engine:"));
  QLabel *label_job_nm = new QLabel(tr("Job Name:"));

  QString job_nm_default = "SIM_" + QDateTime::currentDateTime().toString("yyMMdd_HHmmss"); // TODO SA to engine short name

  combo_eng_sel = new QComboBox();
  combo_eng_sel->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  updateEngineSelectionList();
  le_job_nm = new QLineEdit(job_nm_default);

  label_eng_sel->setBuddy(combo_eng_sel);
  label_job_nm->setBuddy(le_job_nm);

  QHBoxLayout *eng_sel_hl = new QHBoxLayout;
  QHBoxLayout *job_nm_hl = new QHBoxLayout;

  eng_sel_hl->addWidget(label_eng_sel);
  eng_sel_hl->addWidget(combo_eng_sel);
  job_nm_hl->addWidget(label_job_nm);
  job_nm_hl->addWidget(le_job_nm);

  QVBoxLayout *engine_sel_vl = new QVBoxLayout;
  engine_sel_vl->addLayout(eng_sel_hl);
  engine_sel_vl->addLayout(job_nm_hl);

  engine_sel_group->setLayout(engine_sel_vl);
  connect(combo_eng_sel, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSimParams())); //updates parameter list based on selection

  // Sim Params Group, this will change depending on combo box selection.
  sim_params_group = new QGroupBox(tr("Simulation Parameters"));
  sim_params_vl = new QVBoxLayout;
  updateSimParams();
  sim_params_group->setLayout(sim_params_vl);


  // Buttons
  button_run = new QPushButton(tr("&Run"));
  //QPushButton *button_export = new QPushButton(tr("&Export"));
  //QPushButton *button_import = new QPushButton(tr("&Import"));
  button_cancel = new QPushButton(tr("Cancel"));

  connect(button_run, &QAbstractButton::clicked, this, &gui::SimManager::submitSimSetup);
  // TODO connect export and import buttons
  connect(button_cancel, &QAbstractButton::clicked, sim_setup_dialog, &QWidget::hide);

  button_cancel->setShortcut(tr("Esc"));

  bottom_buttons_hl = new QHBoxLayout;
  bottom_buttons_hl->addStretch(1);
  bottom_buttons_hl->addWidget(button_run);
  //bottom_buttons_hl->addWidget(button_export);
  //bottom_buttons_hl->addWidget(button_import);
  bottom_buttons_hl->addWidget(button_cancel);


  // Combine into one dialog
  new_setup_dialog_l = new QVBoxLayout;
  new_setup_dialog_l->addWidget(engine_sel_group);
  new_setup_dialog_l->addWidget(sim_params_group);
  new_setup_dialog_l->addLayout(bottom_buttons_hl);

  sim_setup_dialog->setLayout(new_setup_dialog_l);
}

//only called when combo_eng_sel selection is changed.
void SimManager::updateSimParams()
{
  // clear out existing sim params layout
  QLayoutItem *child;
  while ((child = sim_params_vl->takeAt(0)) != 0) {
    child->widget()->hide();
    delete child->widget();
    delete child;
  }

  // add the property form of the currently selected engine
  QString curr_eng_name = combo_eng_sel->currentText();
  PropertyMap sim_params_map = getEngine(curr_eng_name)->sim_params_map;
  if (!sim_params_map.isEmpty()) {
    curr_sim_params_form = new PropertyForm(sim_params_map, this);
    curr_sim_params_form->show();
    sim_params_vl->addWidget(curr_sim_params_form);
  } else {
    sim_params_vl->addWidget(new QLabel("No simulation parameters available for this engine."));
  }

  /* TODO delete
  // clear out existing sim params layout, don't delete the widget if we're using the one provided by the engine
  QLayoutItem *child;
  while ((child = sim_params_vl->takeAt(0)) != 0) {
    if (using_engine_sim_param_dialog) {
      child->widget()->hide();
    } else {
      delete child->widget();
      delete child;
    }
  }

  // add the currently focused layout
  QString curr_eng_name = combo_eng_sel->currentText();
  QWidget *sim_param_dialog = getEngine(curr_eng_name)->simParamDialog();
  if (sim_param_dialog) {
    sim_param_dialog->show();
    sim_params_vl->addWidget(sim_param_dialog);
    using_engine_sim_param_dialog = true;
  } else {
    sim_params_vl->addWidget(new QLabel("No simulation parameters available for this engine."));
    using_engine_sim_param_dialog = false;
  }*/

  sim_setup_dialog->adjustSize();
}


void SimManager::quickRun()
{
  submitSimSetup();
}


void SimManager::updateEngineSelectionList()
{
  if(!combo_eng_sel)
    return;
  combo_eng_sel->clear();

  if(sim_engines.isEmpty())
    combo_eng_sel->addItem("No Engines");
  else
    for(auto eng : sim_engines)
      combo_eng_sel->addItem(eng->name());
}


void SimManager::updateJobNameDateTime()
{
  if (!le_job_nm)
    return;
  le_job_nm->clear();

  le_job_nm->setText("SIM_" + QDateTime::currentDateTime().toString("yyMMdd_HHmmss"));
}


void SimManager::submitSimSetup()
{
  prim::SimEngine *curr_engine = getEngine(combo_eng_sel->currentIndex());

  if (!curr_engine) {
    qCritical() << tr("Invalid engine selection");
    return;
  }

  // hide setup dialog
  sim_setup_dialog->hide();

  // create job object and extract simulation parameters from the engine's parameter widget
  prim::SimJob *new_job = new prim::SimJob(le_job_nm->text(), curr_engine);
  //new_job->loadSimParamsFromEngineDialog(); TODO remove
  qDebug() << "good?1";
  new_job->addSimParams(curr_sim_params_form->finalProperties());
  qDebug() << "good?2";

  // engine
    // auto filled in: job export path, job result path
    // TODO option to change job export/result paths and option to keep the files after

  addJob(new_job);
  emit emitSimJob(new_job);
}


void SimManager::initEngines()
{
  QString engine_lib_dir_path = settings::AppSettings::instance()->getPath("phys/eng_lib_dir");
  qDebug() << tr("Engine lib path: %1").arg(engine_lib_dir_path);

  QDir engine_lib_dir(engine_lib_dir_path);
  QStringList engine_dir_paths = engine_lib_dir.entryList(QStringList({"*"}),
      QDir::AllDirs | QDir::NoDotAndDotDot);

  // find all existing engines in the engine library
  // TODO edit this part to store file names rather than QDirs
  QList<QDir> engine_dirs;
  QStringList engine_declaration_files;
  QStringList engine_filter(QStringList() << "*.physeng" << "engine_description.xml");
  for (QString engine_dir_path : engine_dir_paths) {
    qDebug() << tr("SimManager: Checking %1 for engine description file").arg(engine_dir_path);
    QDir eng_dir(engine_lib_dir.filePath(engine_dir_path));
    QStringList matched_eng_files = eng_dir.entryList(engine_filter, QDir::Files);

    //qDebug() << tr("Found %1 engine files").arg(matched_eng_files.length());

    // add engine declaration files to a list
    for (QString matched_eng_file : matched_eng_files) {
      engine_declaration_files << eng_dir.absoluteFilePath(matched_eng_file);
      qDebug() << tr("Found engine file: %1").arg(engine_declaration_files.back());
    }
  }

  // import engines found above
  for (QString eng_dec_file : engine_declaration_files) {
    // read each engine description file and add to list
    sim_engines.append(new prim::SimEngine(eng_dec_file));
  }

  qDebug() << tr("Successfully read physics engine files");
}

bool SimManager::exportSimProblem()
{
  // call save function in application.cc with path going to appropriate directory (still need to finalize directory)
  // returns whether export is successful
  //return static_cast<gui::ApplicationGUI*>(parent())->saveToFile(parent()->SaveFlag::Simulation, "problem_export.xml"); // TODO change file name
  return true;
}

} // end gui namespace
