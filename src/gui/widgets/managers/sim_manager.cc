// @file:     sim_manager.cc
// @author:   Samuel
// @created:  2017.09.27
// @editted:  2017.09.27 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     window that allows users to setup and dispatch new simulations,
//            as well as manage ongoing or completed simulations.

#include "sim_manager.h"
#include "global.h"


namespace gui{

// Qt::Dialog makes the main window unclickable. Use Qt::Window if this behavior should be changed.
SimManager::SimManager(QWidget *parent)
  : QWidget(parent, Qt::Dialog)
{
  initSimManager();
}

SimManager::~SimManager()
{
  // clean up jobs TODO change simjob to pluginjob
  for(comp::SimJob *job : sim_jobs)
    delete job;
  sim_jobs.clear();

  // engine pointer delete is dealt with by plugin manager
  plugin_engines.clear();
}


bool SimManager::addJob(comp::SimJob *job)
{
  if(!job)
    return false;
  sim_jobs.prepend(job);
  return true;
}

// PRIVATE

void SimManager::initSimManager()
{
  /*
  setWindowTitle(tr("Simulation Manager"));



  QListWidget *lw_job_plurality = new QListWidget();    // single or chained simulations
  QStackedWidget *sw_eng_list = new QStackedWidget();   // engine list, layout depends on lw_job_plurality
  QVBoxLayout *vl_sim_settings = new QVBoxLayout();     // simulation settings containing job and eng params
  QFormLayout *fl_job_params = new QFormLayout();       // job parameters
  QFormLayout *fl_eng_cmd_params = new QFormLayout();   // engine command parameters
  QVBoxLayout *vl_eng_sim_params = new QVBoxLayout();   // engine simulation parameters

  // job plurality
  QListWidgetItem *lwi_single = new QListWidgetItem("Single");    // TODO add icon
  QListWidgetItem *lwi_chained = new QListWidgetItem("Chained");  // TODO add icon
  lw_job_plurality->addItem(lwi_single);
  lw_job_plurality->addItem(lwi_chained);
  lw_job_plurality->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

  // engine list corresponding to job plurality list
  // TODO this does not account for duplicated names, account for that in the future
  // TODO there's no saving or loading chained simulation presets for now, add that in the future
  QListWidget *lw_single_eng = new QListWidget();
  QListWidget *lw_chained_eng = new QListWidget();
  QToolButton *tb_add_chained_eng = new QToolButton();
  QMenu *menu_add_chained_eng = new QMenu();
  tb_add_chained_eng->setIcon(QIcon::fromTheme("list-add"));
  tb_add_chained_eng->setText("Add Engine to Chain");
  tb_add_chained_eng->setPopupMode(QToolButton::InstantPopup);
  tb_add_chained_eng->setMenu(menu_add_chained_eng);
  tb_add_chained_eng->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  for (comp::SimEngine *engine : sim_engines) {
    QListWidgetItem *lwi_single_eng = new QListWidgetItem(engine->name());
    lw_single_eng->addItem(lwi_single_eng);
    eng_datasets.insert(lwi_single_eng, new EngineDataset(engine));

    // add the engine to the list of engines that can be added to a chained simulation
    menu_add_chained_eng->addAction(engine->name());
  }
  QListWidgetItem *lwi_add_chain_eng = new QListWidgetItem("Add Engine to Chain");
  lw_chained_eng->addItem(lwi_add_chain_eng);
  lw_chained_eng->setItemWidget(lwi_add_chain_eng, tb_add_chained_eng);
  eng_datasets.insert(lwi_add_chain_eng, new EngineDataset());  // insert blank dataset for the add engine action

  sw_eng_list->addWidget(lw_single_eng);
  sw_eng_list->addWidget(lw_chained_eng);
  sw_eng_list->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

  // initialize job params form
  QGroupBox *group_job = new QGroupBox("Job");
  le_job_name = new QLineEdit();
  fl_job_params->addRow(new QLabel("Job Name:"), le_job_name);
  group_job->setLayout(fl_job_params);
  group_job->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  // initialize engine command line parameters form
  QGroupBox *group_eng_cml = new QGroupBox("Engine Command");
  QTextEdit *te_eng_command = new QTextEdit();
  QToolButton *tb_eng_command_preset = new QToolButton();
  QMenu *menu_eng_command_preset = new QMenu();
  tb_eng_command_preset->setIcon(QIcon::fromTheme("preferences-system"));
  tb_eng_command_preset->setText("Presets");
  tb_eng_command_preset->setPopupMode(QToolButton::InstantPopup);
  tb_eng_command_preset->setMenu(menu_eng_command_preset);
  tb_eng_command_preset->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

  QHBoxLayout *hl_eng_command = new QHBoxLayout();
  hl_eng_command->addWidget(new QLabel("Invocation command format"));
  hl_eng_command->addStretch();
  hl_eng_command->addWidget(tb_eng_command_preset);

  fl_eng_cmd_params->addRow(hl_eng_command);
  fl_eng_cmd_params->addRow(te_eng_command);
  group_eng_cml->setLayout(fl_eng_cmd_params);
  group_eng_cml->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  // initialize engine simulation parameters form
  QGroupBox *group_eng_sim = new QGroupBox("Engine Simulation Parameters");
  group_eng_sim->setLayout(vl_eng_sim_params);
  group_eng_sim->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  // add engine name to chained engine list when engine is selected from add
  // engine menu
  connect(menu_add_chained_eng, &QMenu::triggered,
          [this, lw_chained_eng](QAction *action)
          {
            // insert into the second-last row
            int row = (lw_chained_eng->count() > 1) ? lw_chained_eng->count()-1 : 0;
            QWidget *w_eng = new QWidget();
            QHBoxLayout *hl_eng = new QHBoxLayout();
            QPushButton *pb_remove = new QPushButton();
            if (QIcon::hasThemeIcon("list-remove")) {
              pb_remove->setIcon(QIcon::fromTheme("list-remove"));
            } else {
              pb_remove->setText("Remove");
            }
            hl_eng->addWidget(new QLabel(action->text()));
            hl_eng->addStretch();
            hl_eng->addWidget(pb_remove);
            w_eng->setLayout(hl_eng);

            QListWidgetItem *lwi_eng = new QListWidgetItem();
            lwi_eng->setSizeHint(w_eng->sizeHint());
            lw_chained_eng->insertItem(row, lwi_eng);
            lw_chained_eng->setItemWidget(lwi_eng, w_eng);

            comp::SimEngine *engine = getEngine(action->text());
            eng_datasets.insert(lwi_eng, new EngineDataset(engine));
            lw_chained_eng->setCurrentItem(lwi_eng);

            connect(pb_remove, &QAbstractButton::clicked,
                    [this, lw_chained_eng, lwi_eng]()
                    {
                      int rm_row = lw_chained_eng->row(lwi_eng);
                      QListWidgetItem *lwi_rm_eng = lw_chained_eng->takeItem(rm_row);
                      if (lwi_rm_eng != nullptr) {
                        EngineDataset *rm_dataset = eng_datasets.take(lwi_rm_eng);
                        // TODO deal with the removal of all form fields
                        delete rm_dataset->prop_form;
                        delete rm_dataset;
                        delete lwi_rm_eng;
                      }
                    });
          });

  // get current engine name, property form pair
  auto currentEngineDataset = [this, sw_eng_list]() -> EngineDataset*
  {
    QListWidget *curr_list = static_cast<QListWidget*>(sw_eng_list->currentWidget());
    return eng_datasets.value(curr_list->currentItem());
  };

  // update job parameters form according to the current plurality and engine selection
  auto updateJobParamsForm = [this]()
  {
    le_job_name->setText(defaultJobName());
  };

  auto updateEngCommandForm = [te_eng_command, menu_eng_command_preset](EngineDataset *eng_data)
  {
    te_eng_command->setText(eng_data->command_format);

    // update engine command presets
    menu_eng_command_preset->clear();
    for (auto cmd_format : eng_data->engine->commandFormats()) {
      menu_eng_command_preset->addAction(cmd_format.first);
    }
  };

  // update simulation parameters form according to the current engine selection
  auto updateSimParamsForm = [vl_eng_sim_params](EngineDataset *eng_data) mutable
  {
    // remove previous form
    QLayoutItem *child;
    while ((child = vl_eng_sim_params->takeAt(0)) != 0) {
      QWidget *old_form = child->widget();
      old_form->setParent(nullptr);
      delete child;
    }

    // show new form
    vl_eng_sim_params->addWidget(eng_data->prop_form);
  };

  // actions performed whenever engine selection changes
  auto engineSelectionChangeActions = [currentEngineDataset, 
                                       updateEngCommandForm,
                                       updateSimParamsForm]() mutable
  {
    EngineDataset *curr_eng_data = currentEngineDataset();
    if (curr_eng_data != nullptr && !curr_eng_data->isEmpty()) {
      updateEngCommandForm(curr_eng_data);
      updateSimParamsForm(curr_eng_data);
    }
  };

  // update the sim param property form every time one of the lists have changed
  connect(lw_job_plurality, &QListWidget::currentItemChanged,
          [sw_eng_list, updateJobParamsForm, engineSelectionChangeActions,
            lw_job_plurality]
            (QListWidgetItem *lwi_new, QListWidgetItem *) mutable 
          {
            // write changes before updating list
            //writeChangesToEngineData();
            sw_eng_list->setCurrentIndex(lw_job_plurality->row(lwi_new));
            updateJobParamsForm();
            engineSelectionChangeActions();
          });

  // react to single engine list changes
  connect(lw_single_eng, &QListWidget::currentItemChanged, 
          engineSelectionChangeActions);
  
  // react to chained engine list changes
  connect(lw_chained_eng, &QListWidget::currentItemChanged, 
          engineSelectionChangeActions);

  // immediately update the current engine dataset when the command field is 
  // updated by the user (but not programmatically)
  connect(te_eng_command, &QTextEdit::textChanged,
          [currentEngineDataset, te_eng_command]()
          {
            EngineDataset *eng_dataset = currentEngineDataset();
            if (eng_dataset != nullptr) {
              eng_dataset->command_format = te_eng_command->toPlainText();
            }
          });

  // update the command format if a preset format has been chosen by the user
  connect(menu_eng_command_preset, &QMenu::triggered,
          [currentEngineDataset, menu_eng_command_preset, te_eng_command](QAction *action)
          {
            EngineDataset *eng_dataset = currentEngineDataset();
            int action_i = menu_eng_command_preset->actions().indexOf(action);
            QString chosen_cmd = eng_dataset->engine->jointCommandFormat(action_i).second;
            te_eng_command->setText(chosen_cmd);
          });


  // select the top item by default
  lw_job_plurality->setCurrentItem(lw_job_plurality->item(0));
  lw_single_eng->setCurrentItem(lw_single_eng->item(0));

  // job params form
  vl_sim_settings->addWidget(group_job);
  vl_sim_settings->addWidget(group_eng_cml);
  vl_sim_settings->addWidget(group_eng_sim);

  // buttons
  QPushButton *pb_run = new QPushButton(tr("&Run"));
  pb_run->setShortcut(Qt::Key_Return);
  QPushButton *pb_close = new QPushButton(tr("&Close"));
  pb_close->setShortcut(Qt::Key_Escape);

  // button actions
  connect(pb_close, &QAbstractButton::clicked, this, &QWidget::hide);
  connect(pb_run, &QAbstractButton::clicked,
          [this, lwi_single, lwi_chained, lw_chained_eng, currentEngineDataset]()
          {
            hide();
            // create sim job and submit to application
            // TODO improve Job constructor to take job step directly
            // TODO simplify this lambda function
            if (lwi_single->isSelected()) {
              EngineDataset *eng_data = currentEngineDataset();
              comp::SimJob *new_job = new comp::SimJob(le_job_name->text());
              comp::SimJob::JobStep js(eng_data->engine, 
                                       eng_data->command_format.split("\n"),
                                       eng_data->prop_form->finalProperties()
                                       );
              new_job->addJobStep(js);
              addJob(new_job);
              emit sig_simJob(new_job);
            } else if (lwi_chained->isSelected()) {
              comp::SimJob *new_job = new comp::SimJob(le_job_name->text());
              for (int i=0; i<lw_chained_eng->count(); i++) {
                QListWidgetItem *lwi_eng_form = lw_chained_eng->item(i);
                EngineDataset *eng_data = eng_datasets.value(lwi_eng_form);
                if (eng_data == nullptr || eng_data->isEmpty())
                  continue;

                // create sim job step and add it to the sim job
                comp::SimJob::JobStep js(eng_data->engine, 
                                         eng_data->command_format.split("\n"),
                                         eng_data->prop_form->finalProperties());
                new_job->addJobStep(js);
              }
              addJob(new_job);
              emit sig_simJob(new_job);
            } else {
              qWarning() << "Neither expected engine execution mode is selected, nothing to run.";
            }
          });

  // main layout
  QHBoxLayout *hl_sim_panes = new QHBoxLayout();
  hl_sim_panes->addWidget(lw_job_plurality);
  hl_sim_panes->addWidget(sw_eng_list);
  hl_sim_panes->addLayout(vl_sim_settings);

  QHBoxLayout *hl_buttons = new QHBoxLayout();
  hl_buttons->addStretch();
  hl_buttons->addWidget(pb_run);
  hl_buttons->addWidget(pb_close);

  QVBoxLayout *vl_main = new QVBoxLayout();
  vl_main->addLayout(hl_sim_panes);
  vl_main->addLayout(hl_buttons);

  setLayout(vl_main);
  */
}


void SimManager::showEvent(QShowEvent *e)
{
  if (!e->spontaneous()) {
    le_job_name->setText(defaultJobName());
  }
}


void SimManager::quickRun()
{
  /*
  le_job_nm->setText(defaultJobName());
  submitSimSetup();
  */
}


QString SimManager::defaultJobName()
{
  return "SIM_" + QDateTime::currentDateTime().toString("yyMMdd_HHmmss");
}


void SimManager::saveSimulationPreset()
{
  /*
  comp::SimEngine *curr_engine = getEngine(cb_eng_sel->currentIndex());

  if (!curr_engine || !curr_sim_params_form) {
    qCritical() << tr("Invalid engine selection or engine doesn't have parameters");
    return;
  }

  // set up the directory
  QDir config_dir(curr_engine->userPresetDirectoryPath());
  if (!config_dir.mkpath("."))
    qWarning() << tr("Unable to create user preset directory at %1").arg(config_dir.path());

  // prompt for preset name
  bool ok;
  QString preset_name = QInputDialog::getText(sim_manager_dialog, tr("Preset name"),
                                              tr("Preset name:"), QLineEdit::Normal,
                                              tr("Custom Preset"), &ok);
  if (!ok) {
    return;
  } else if (preset_name.isEmpty()) {
    QMessageBox msg;
    msg.setText("The preset name cannot be empty, aborting.");
    msg.exec();
    return;
  }
  // TODO check for preset name conflicts with existing presets

  QString preset_path = config_dir.filePath(preset_name);
  QFile f(preset_path);
  if (!f.open(QIODevice::WriteOnly)) {
    qCritical() << tr("Error when opening file to save preset.");
    return;
  }

  QXmlStreamWriter ws(&f);
  qDebug() << tr("Beginning preset writing to %1").arg(preset_path);
  ws.setAutoFormatting(true);
  ws.writeStartDocument();

  ws.writeStartElement("properties");
  PropertyMap::writeValuesToXMLStream(curr_sim_params_form->finalProperties(), &ws);
  ws.writeEndElement();

  f.close();
  qDebug() << tr("Preset writing complete");
  */
}


void SimManager::submitSimSetup()
{
  /* TODO remove after finishing new simulation submission implementation
  comp::SimEngine *curr_engine = getEngine(cb_eng_sel->currentIndex());

  if (!curr_engine) {
    qCritical() << tr("Invalid engine selection");
    return;
  }

  // hide setup dialog
  sim_setup_dialog->hide();

  // create job object and extract simulation parameters from the engine's parameter widget
  comp::SimJob *new_job = new comp::SimJob(le_job_nm->text(), curr_engine);
  new_job->addSimParams(curr_sim_params_form->finalProperties());

  addJob(new_job);
  emit sig_simJob(new_job);
  */
}

/*
void SimManager::saveSettingsAsDefault()
{
  comp::SimEngine *curr_engine = getEngine(cb_eng_sel->currentIndex());

  if (!curr_engine || !curr_sim_params_form) {
    qCritical() << tr("Invalid engine selection or engine doesn't have parameters");
    return;
  }

  QString usr_cfg_file_path = curr_engine->userConfigurationFilePath();
  QFileInfo usr_cfg_file_inf(usr_cfg_file_path);
  usr_cfg_file_inf.dir().mkpath(".");
  QFile write_file(usr_cfg_file_path);

  if (!write_file.open(QIODevice::WriteOnly)) {
    qCritical() << tr("Export Simulation Settings: error when opening file to save");
    return;
  }
  QXmlStreamWriter ws(&write_file);
  qDebug() << tr("Beginning export to %1").arg(usr_cfg_file_path);
  ws.setAutoFormatting(true);
  ws.writeStartDocument();

  ws.writeStartElement("properties");
  PropertyMap::writeValuesToXMLStream(curr_sim_params_form->finalProperties(), &ws);
  ws.writeEndElement();

  write_file.close();
  qDebug() << tr("Export complete");
}


void SimManager::resetToUserDefault()
{
  updateSimParams();
}


void SimManager::resetToEngineDefault()
{
  comp::SimEngine *curr_engine = getEngine(cb_eng_sel->currentIndex());
  QFile usr_cfg_file(curr_engine->userConfigurationFilePath());
  if (usr_cfg_file.remove()) {
    qDebug() << tr("Removed user config file");
  } else {
    qDebug() << tr("No user config file to remove");
  }
  updateSimParams();
}
*/

} // end gui namespace
