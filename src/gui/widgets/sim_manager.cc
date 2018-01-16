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
  sim_setup_dialog = new QWidget(this, Qt::Dialog);

  // Engine Select Group, can be written here since only done once.
  QGroupBox *engine_sel_group = new QGroupBox(tr("Engine Selection"));
  QLabel *label_eng_sel = new QLabel(tr("Engine:"));
  QLabel *label_job_nm = new QLabel(tr("Job Name:"));

  QString job_nm_default = "SIM_" + QDateTime::currentDateTime().toString("yyMMdd_HHmmss"); // TODO SA to engine short name

  combo_eng_sel = new QComboBox();
  combo_eng_sel->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  updateEngSelCombo();
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

  createParamGroup();

  // Buttons, these will have to be re-added every time the sim params are updated

  button_run = new QPushButton(tr("&Run"));
  //QPushButton *button_export = new QPushButton(tr("&Export"));
  //QPushButton *button_import = new QPushButton(tr("&Import"));
  button_cancel = new QPushButton(tr("Cancel"));

  connect(button_run, &QAbstractButton::clicked, this, &gui::SimManager::submitSimSetup);
  // TODO connect export and import buttons
  connect(button_cancel, &QAbstractButton::clicked, sim_setup_dialog, &QWidget::hide);

  button_cancel->setShortcut(tr("Esc"));

  createButtonLayout();

  // Combine into one dialog
  new_setup_dialog_l = new QVBoxLayout;
  new_setup_dialog_l->addWidget(engine_sel_group);
  new_setup_dialog_l->addWidget(sim_params_group);
  new_setup_dialog_l->addLayout(bottom_buttons_hl);

  sim_setup_dialog->setLayout(new_setup_dialog_l);
}

void SimManager::createButtonLayout()
{
  bottom_buttons_hl = new QHBoxLayout;
  bottom_buttons_hl->addStretch(1);
  bottom_buttons_hl->addWidget(button_run);
  //bottom_buttons_hl->addWidget(button_export);
  //bottom_buttons_hl->addWidget(button_import);
  bottom_buttons_hl->addWidget(button_cancel);
}

void SimManager::createParamGroup()
{
  sim_params_group = new QGroupBox(tr("Simulation Parameters"));
  sim_params_vl = new QVBoxLayout;

  if(combo_eng_sel->currentText() == QString("SimAnneal")){
    label_result_queue_size = new QLabel("Result Queue Size:");
    label_preanneal_cycles = new QLabel("Preanneal Cycles:");
    label_anneal_cycles = new QLabel("Anneal Cycles:");
    label_global_v0 = new QLabel("Global Bias v_0:");
    label_debye_length = new QLabel("Debye Length (m):");

    le_result_queue_size = new QLineEdit("1000"); // TODO these default values should be read from the engine description file
    le_preanneal_cycles = new QLineEdit("1000");
    le_anneal_cycles = new QLineEdit("10000");
    le_global_v0 = new QLineEdit("1.5");
    le_debye_length = new QLineEdit("5E-9");

    label_result_queue_size->setBuddy(le_result_queue_size);
    label_preanneal_cycles->setBuddy(le_preanneal_cycles);
    label_anneal_cycles->setBuddy(le_anneal_cycles);
    label_global_v0->setBuddy(le_global_v0);
    label_debye_length->setBuddy(le_debye_length);

    QHBoxLayout *result_queue_size_hl = new QHBoxLayout;
    QHBoxLayout *preanneal_cycles_hl = new QHBoxLayout;
    QHBoxLayout *anneal_cycles_hl = new QHBoxLayout;
    QHBoxLayout *global_v0_hl = new QHBoxLayout;
    QHBoxLayout *debye_length_hl = new QHBoxLayout;
    result_queue_size_hl->addWidget(label_result_queue_size);
    result_queue_size_hl->addWidget(le_result_queue_size);
    preanneal_cycles_hl->addWidget(label_preanneal_cycles);
    preanneal_cycles_hl->addWidget(le_preanneal_cycles);
    anneal_cycles_hl->addWidget(label_anneal_cycles);
    anneal_cycles_hl->addWidget(le_anneal_cycles);
    global_v0_hl->addWidget(label_global_v0);
    global_v0_hl->addWidget(le_global_v0);
    debye_length_hl->addWidget(label_debye_length);
    debye_length_hl->addWidget(le_debye_length);

    sim_params_vl->addLayout(result_queue_size_hl);
    sim_params_vl->addLayout(preanneal_cycles_hl);
    sim_params_vl->addLayout(anneal_cycles_hl);
    sim_params_vl->addLayout(global_v0_hl);
    sim_params_vl->addLayout(debye_length_hl);

  }else if(combo_eng_sel->currentText() == QString("PoisSolver")){
    button_xml_find = new QPushButton(tr("Find"));
    connect(button_xml_find, &QAbstractButton::clicked, this, &gui::SimManager::xmlFind);
    label_xml_path = new QLabel("XML Path:");
    le_xml_path = new QLineEdit("");
    label_xml_path->setBuddy(le_xml_path);
    QHBoxLayout *xml_path_hl = new QHBoxLayout;
    xml_path_hl->addWidget(label_xml_path);
    xml_path_hl->addWidget(le_xml_path);
    xml_path_hl->addWidget(button_xml_find);
    sim_params_vl->addLayout(xml_path_hl);
  }

  sim_params_group->setLayout(sim_params_vl);
}

//only called when combo_eng_sel selection is changed.
void SimManager::updateSimParams()
{
  //delete the old params
  if(new_setup_dialog_l->takeAt(new_setup_dialog_l->indexOf(sim_params_group)) != 0){
    delete sim_params_group;
  }
  //create and readd the params to the end of the list
  createParamGroup();
  new_setup_dialog_l->addWidget(sim_params_group);
  //TODO: Find a way to get the button layout's index so we can SAFELY delete it.
  //delete the old buttons, then readd them to the bottom
  delete bottom_buttons_hl;
  createButtonLayout();
  new_setup_dialog_l->addLayout(bottom_buttons_hl);
  //fit to content
  sim_setup_dialog->adjustSize();
}


void SimManager::xmlFind()
{
  QString fileName = QFileDialog::getOpenFileName(this,
    tr("Open XML"), ".", tr("XML Files (*.xml)"));
  le_xml_path->setText(fileName);
}


void SimManager::updateEngSelCombo()
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


void SimManager::submitSimSetup()
{
  int eng_ind = combo_eng_sel->currentIndex();
  if(eng_ind < 0 || eng_ind >= sim_engines.size()){
    qDebug() << tr("Invalid engine selection");
    return;
  }

  // hide setup dialog
  sim_setup_dialog->hide();

  // create job object
  prim::SimJob *new_job = new prim::SimJob(le_job_nm->text(), sim_engines[combo_eng_sel->currentIndex()]);

  // fill in job properties according to input fields
  if(combo_eng_sel->currentText() == QString("SimAnneal")){
    new_job->addSimulationParameter("result_queue_size", le_result_queue_size->text());
    new_job->addSimulationParameter("preanneal_cycles", le_preanneal_cycles->text());
    new_job->addSimulationParameter("anneal_cycles", le_anneal_cycles->text());
    new_job->addSimulationParameter("global_v0", le_global_v0->text());
    new_job->addSimulationParameter("debye_length", le_debye_length->text());
  } else if(combo_eng_sel->currentText() == QString("PoisSolver")){
    //it is up to the user to make sure the design xml is provided.
    new_job->addSimulationParameter("xml_path", le_xml_path->text());
  }
  // engine
    // auto filled in: job export path, job result path
    // TODO option to change job export/result paths and option to keep the files after
    // TODO somewhere else, delete files that aren't specified to be kept. Delete only files generated by the program.


  // sim params TODO change code to be general, hard coded for SimAnneal right now
  //new_job->

  // arguments



  // TODO add parameters to QStringList according according to input text
  /*QStringList arguments;
  arguments << "-p"; // preanneal cycles

  arguments << "-a"; // anneal cycles

  arguments << "-v"; // v_0*/

  addJob(new_job);
  emit emitSimJob(new_job);
}


void SimManager::initEngines()
{
  QString engine_lib_dir_path = settings::AppSettings::instance()->getPath("phys/eng_lib_dir");

  QDir engine_lib_dir(engine_lib_dir_path);
  QStringList engine_dir_paths = engine_lib_dir.entryList(QStringList({"*"}), QDir::AllDirs | QDir::NoDotAndDotDot);

  // find all existing engines in the engine library
  QList<QDir> engine_dirs;
  for(QString engine_dir_path : engine_dir_paths){
    qDebug() << tr("SimManager: Checking %1 for engine description file").arg(engine_dir_path);
    QDir eng_dir(engine_lib_dir.filePath(engine_dir_path));
    if(eng_dir.exists("engine_description.xml"))
      engine_dirs.append(eng_dir);
  }

  // read each engine description file
  for(QDir engine_dir : engine_dirs) {
    QFile eng_f(engine_dir.absoluteFilePath("engine_description.xml"));

    if(!eng_f.open(QFile::ReadOnly | QFile::Text)){
      qCritical() << tr("SimManager: cannot open engine description file %1").arg(eng_f.fileName());
      return;
    }

    QXmlStreamReader eng_s(&eng_f);
    qDebug() << tr("Reading engine library from %1").arg(eng_f.fileName());

    QString read_eng_nm, read_eng_ver, read_bin_path;
    while(!eng_s.atEnd()){
      if(eng_s.isStartElement()){
        if(eng_s.name() == "physeng"){
          // TODO move the following to SimEngine
          while(!(eng_s.isEndElement() && eng_s.name() == "physeng")){
            if(!eng_s.readNextStartElement())
              continue; // skip until a start element is encountered
            if(eng_s.name() == "name"){
              read_eng_nm = eng_s.readElementText();
            }
            else if(eng_s.name() == "version"){
              read_eng_ver = eng_s.readElementText();
            }
            else if(eng_s.name() == "bin_path"){
              read_bin_path = eng_s.readElementText();
            }
          }
          prim::SimEngine *eng = new prim::SimEngine(read_eng_nm);
          eng->setVersion(read_eng_ver);
          eng->setBinaryPath(engine_dir.filePath(read_bin_path));

          sim_engines.append(eng);

          read_eng_nm = read_eng_ver = read_bin_path = "";
          eng_s.readNext();
        }
        else{
          qDebug() << tr("SimManager: invalid element encountered on line %1 - %2").arg(eng_s.lineNumber()).arg(eng_s.name().toString());
        }
      }
      else
        eng_s.readNext();
    }

    eng_f.close();
  }


  qDebug() << tr("Successfully read physics engine library");
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
