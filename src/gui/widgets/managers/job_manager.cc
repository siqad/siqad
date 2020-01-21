// @file:     job_manager.cc
// @author:   Samuel
// @created:  2019.03.14
// @license:  GNU LGPL v3
//
// @desc:     Manage all plugin jobs both setting them up, keeping track of 
//            them, as well as sending results to the appropriate target widget.

#include "job_manager.h"
#include "global.h"
#include <initializer_list>

using namespace gui;

typedef comp::PluginEngine PE;

JobManager::JobManager(PluginManager *plugin_manager, SimVisualizer *sim_visualizer,
                       QWidget *parent)
  : QWidget(parent, Qt::Dialog), plugin_manager(plugin_manager),
    sim_visualizer(sim_visualizer)
{
  initJobManagerGUI();
}

JobManager::~JobManager()
{
  // clean up jobs
  for (comp::SimJob *job : sim_jobs)
    delete job;
  sim_jobs.clear();
}

void JobManager::addJob(comp::SimJob *job)
{
  if (sim_jobs.contains(job))
    return;

  sim_jobs.append(job);
  connect(job, &comp::SimJob::sig_exportJobStepProblem,
          this, &gui::JobManager::sig_exportJobProblem);
  connect(job, &comp::SimJob::sig_jobFinishState, 
          this, &JobManager::processFinishedJob);
  connect(job, &comp::SimJob::sig_requestJobVisualization,
          [this, job]()
          {
            if (eligibleForSimVisualizer(job)) {
              sim_visualizer->showJob(job);
            } else {
              qWarning() << "Job not eligible for SimVisualizer.";
            }
          });

  // update job list
  // TODO better implementation in the future, current implementation is a quick hack
  QList<QStandardItem*> row_job_info;
  row_job_info.append(new QStandardItem(job->name()));
  job_view_model->insertRow(0, row_job_info); // prepend row

  QList<QWidget*> row_widgets({
        job->guiControlElems().pb_terminate,
        job->guiControlElems().pb_sim_visualize,
        job->guiControlElems().pb_job_terminal
      });

  tv_job_view->resizeColumnToContents(0);
  QModelIndex mi_back = job_view_model->indexFromItem(row_job_info.back());
  int col_start = mi_back.column() + 1;
  for (int col=col_start; col < col_start + row_widgets.size(); col++) {
    tv_job_view->setIndexWidget(job_view_model->index(mi_back.row(), 
          col), row_widgets[col-col_start]);
    tv_job_view->resizeColumnToContents(col);
  }
}

void JobManager::runJob(comp::SimJob *job)
{
  addJob(job);
  job->beginJob();
}

void JobManager::processFinishedJob(comp::SimJob *job, comp::SimJob::JobState)
{
  // TODO if successful, check that result files are all successfully read (add
  // a flag in job steps to facilitate this)

  // update GUI elements in job manager


  // execute SQCommands if any is available
  // TODO allow users to make execution manual and prompt user before execution
  for (comp::JobStep *js : job->jobSteps()) {
    if (js->jobResults().uniqueKeys().contains(comp::JobResult::SQCommandsResult)) {
      comp::JobResult *sq_commands_result = js->jobResults().value(comp::JobResult::SQCommandsResult);
      QStringList commands = static_cast<comp::SQCommands*>(sq_commands_result)->sqCommands();
      for (const QString &command : commands) {
        // TODO add convenient function to commander to take QStringList of commands
        emit sig_executeSQCommand(command);
      }
    }
  }

  // send job to sim visualize if the job contains any components that can be
  // visualized using it
  // TODO check if any of the result types in the job match with any of the result
  // types in sim_visualizer->supportedResultTypes()
  if (eligibleForSimVisualizer(job)) {
    sim_visualizer->showJob(job);
  }
}

bool JobManager::eligibleForSimVisualizer(comp::SimJob *job)
{
  for (comp::JobResult::ResultType type : job->resultTypeStepMap().keys())
    if (sim_visualizer->supportedResultTypes().contains(type))
      return true;
  return false;
}


// PRIVATE

void JobManager::initJobManagerGUI()
{
  // generic GUI settings
  setWindowTitle(tr("Job Manager"));

  // panes setup
  // TODO job manager should consist of the following panes:
  //    action type (set up new job, view previous jobs)
  //    if new job:
  //        job plurality (single, chained)
  //        engine selection (filter available)
  //        job details
  //    if previous jobs:
  //        big table with filters

  QListWidget *lw_job_action = new QListWidget();
  QStackedWidget *sw_job_action = new QStackedWidget();

  QListWidgetItem *lwi_new_job = new QListWidgetItem("New Job");
  QListWidgetItem *lwi_view_jobs = new QListWidgetItem("View Jobs");

  lwi_new_job->setIcon(QIcon::fromTheme("list-add"));
  lwi_view_jobs->setIcon(QIcon::fromTheme("document-properties"));

  lw_job_action->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

  // new job action
  lw_job_action->addItem(lwi_new_job);
  sw_job_action->addWidget(initJobSetupPanel());

  // view job action
  lw_job_action->addItem(lwi_view_jobs);
  sw_job_action->addWidget(initJobViewPanel());

  connect(lw_job_action, &QListWidget::currentRowChanged,
          sw_job_action, &QStackedWidget::setCurrentIndex);

  // put stack widget in scroll area
  /*
  QScrollArea *sa_sw_job_action = new QScrollArea();
  sa_sw_job_action->setWidget(sw_job_action);
  sa_sw_job_action->setWidgetResizable(true);
  */

  // set main job manager layout
  QHBoxLayout *hl_main = new QHBoxLayout();
  hl_main->addWidget(lw_job_action);
  hl_main->addWidget(sw_job_action);
  setLayout(hl_main);
}


QWidget *JobManager::initJobSetupPanel()
{
  // Job setup panel has the following panes:
  // _________________________________
  // |           |          |         |
  // |  ENGINE   | JOB STEP |   JOB   |
  // | SELECTION |   LIST   | DETAILS |
  // |___________|__________|_________|
  //

  // overall layout of this panel
  //QHBoxLayout *hl_job_setup = new QHBoxLayout();

  // ENGINE SELECTION
  // engine filter
  QGroupBox *gb_eng_selection = new QGroupBox("Engine List");
  QCheckBox *cb_eng_filter = new QCheckBox("Filter");
  cb_eng_filter->setChecked(false);
  QWidget *w_eng_filter = new QWidget();
  QMultiMap<QString, PE::Service> service_map;
  for (PE::Service service : PE::official_services) {
    service_map.insert(service.category, service);
  }
  cat_filter_model = new QStandardItemModel();
  enum JobManagerItemRole{Ignore=Qt::UserRole, ServiceNameRole, EngineDatasetRole};
  for (const QString &key : service_map.uniqueKeys()) {
    QStandardItem *si_service_category = new QStandardItem(key);
    si_service_category->setCheckable(true);
    QList<PE::Service> cat_services = service_map.values(key);
    for (PE::Service cat_service : cat_services) {
      QStandardItem *si_service = new QStandardItem(cat_service.label);
      si_service->setData(cat_service.name, ServiceNameRole);
      si_service->setCheckable(true);
      si_service_category->appendRow(si_service);
    }
    cat_filter_model->appendRow(si_service_category);
  }
  QTreeView *tv_cat_filter = new QTreeView();
  tv_cat_filter->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tv_cat_filter->setWordWrap(true);
  tv_cat_filter->setUniformRowHeights(false);
  tv_cat_filter->header()->hide();
  tv_cat_filter->setModel(cat_filter_model);

  // filter buttons
  QPushButton *pb_apply_filter = new QPushButton("Apply");
  QPushButton *pb_clear_filter = new QPushButton("Clear");
  QHBoxLayout *hl_filter_buttons = new QHBoxLayout();
  hl_filter_buttons->addStretch();
  hl_filter_buttons->addWidget(pb_apply_filter);
  hl_filter_buttons->addWidget(pb_clear_filter);

  QVBoxLayout *vl_eng_filter = new QVBoxLayout();
  vl_eng_filter->addWidget(tv_cat_filter);
  vl_eng_filter->addLayout(hl_filter_buttons);
  w_eng_filter->setLayout(vl_eng_filter);

  // connect group box checkbox to show/hide category filter tree view
  connect(cb_eng_filter, &QCheckBox::toggled,
          w_eng_filter, &QWidget::setVisible);
  w_eng_filter->setVisible(cb_eng_filter->isChecked());

  // clear filter checks, this function alone doesn't apply the filter
  auto clearFilterChecks = [this]()
  {
    for (int i=0; i<cat_filter_model->rowCount(); i++) {
      // go through categories
      QStandardItem *cat_item = cat_filter_model->item(i);
      for (int j=0; j<cat_item->rowCount(); j++) {
        // go through services
        cat_item->child(j)->setCheckState(Qt::Unchecked);
      }
    }
  };

  // update filter check states
  auto updateFilterCheckStates = [](QStandardItem *changed_item)
  {
    if (changed_item->hasChildren()) {
      // changed item is a category, change all children to the same check state
      Qt::CheckState set_check_state = changed_item->checkState();
      if (changed_item->checkState() == Qt::PartiallyChecked)
        return;
      for (int i=0; i<changed_item->rowCount(); i++) {
        QStandardItem *si_service = changed_item->child(i);
        si_service->setCheckState(set_check_state);
      }
      return;
    } else {
      // changed item is a service, update parent check state
      QStandardItem *si_cat = changed_item->parent();
      int checked_count = 0;
      for (int i=0; i<si_cat->rowCount(); i++) {
        QStandardItem *si_service = si_cat->child(i);
        if (si_service->checkState() == Qt::Checked) {
          checked_count++;
        }
      }
      if (checked_count == 0) {
        si_cat->setCheckState(Qt::Unchecked);
      } else if (checked_count == si_cat->rowCount()) {
        si_cat->setCheckState(Qt::Checked);
      } else {
        si_cat->setCheckState(Qt::PartiallyChecked);
      }
    }
  };

  // ItemIsAutoTristate does not work for QTreeView as of Qt 5.12.2, a manual 
  // implementation is needed to set parent checkboxes correctly
  connect(cat_filter_model, &QStandardItemModel::itemChanged, updateFilterCheckStates);

  // engine list
  eng_model = new QStandardItemModel();
  eng_list_fields = QList<PE::StandardItemField>(
      {
        PE::NameField,
        PE::UniqueIdentifierField,
        PE::ServicesField
      });
  for (const PE *engine : plugin_manager->pluginEngines()) {
    QList<QStandardItem*> row_si = engine->standardItemRow(eng_list_fields);
    eng_model->appendRow(row_si);
  }
  // set the engine list model as the filtered proxy model
  eng_filter_proxy_model = new QSortFilterProxyModel();
  eng_filter_proxy_model->setSourceModel(eng_model);
  eng_filter_proxy_model->setFilterKeyColumn(eng_list_fields.indexOf(PE::ServicesField));
  lv_engines = new QListView();
  lv_engines->setModel(eng_filter_proxy_model);
  lv_engines->setEditTriggers(QAbstractItemView::NoEditTriggers);

  auto applyEngineFilter = [this]()
  {
    // update filter expression
    QString filter_exp;
    for (int i=0; i<cat_filter_model->rowCount(); i++) {
      // go through categories
      QStandardItem *cat_item = cat_filter_model->item(i);
      for (int j=0; j<cat_item->rowCount(); j++) {
        // go through services
        QStandardItem *service_item = cat_item->child(j);
        if (service_item->checkState() == Qt::Checked ) {
          QString pipe = (filter_exp.isEmpty()) ? "" : "|";
          filter_exp += pipe + service_item->data(ServiceNameRole).toString();
        }
      }
    }
    qDebug() << tr("filter: %1").arg(filter_exp);
    eng_filter_proxy_model->setFilterRegExp(filter_exp);
  };

  // apply engine filter
  connect(pb_apply_filter, &QPushButton::clicked, applyEngineFilter);

  // clear engine filter
  connect(pb_clear_filter, &QPushButton::clicked,
          [clearFilterChecks, applyEngineFilter]()
          {
            clearFilterChecks();
            applyEngineFilter();
          });

  QVBoxLayout *vl_eng_selection = new QVBoxLayout();
  vl_eng_selection->addWidget(cb_eng_filter);
  vl_eng_selection->addWidget(w_eng_filter);
  vl_eng_selection->addWidget(lv_engines);
  gb_eng_selection->setLayout(vl_eng_selection);


  // JOB STEP LIST
  QGroupBox *gb_job_steps = new QGroupBox("Job Steps");
  QToolBar *tb_job_steps = new QToolBar();
  QAction *a_add_job_step = new QAction(QIcon::fromTheme("list-add"),
                                        "Add job step");
  QAction *a_remove_job_step = new QAction(QIcon::fromTheme("list-remove"),
                                           "Remove job step");
  QAction *a_move_up_job_step = new QAction(QIcon::fromTheme("arrow-up"),
                                            "Move up");
  QAction *a_move_down_job_step = new QAction(QIcon::fromTheme("arrow-down"),
                                              "Move down");
  tb_job_steps->addAction(a_add_job_step);
  tb_job_steps->addAction(a_remove_job_step);
  tb_job_steps->addAction(a_move_up_job_step);
  tb_job_steps->addAction(a_move_down_job_step);
  job_steps_model = new QStandardItemModel();
  lv_job_steps = new QListView();
  lv_job_steps->setModel(job_steps_model);
  lv_job_steps->setEditTriggers(QAbstractItemView::NoEditTriggers);
  // NOTE InternalMove deletes the original item and recreates it at the new
  // location, which doesn't work well with the current EngineDataset pointer
  // set up. Disabling drag and drop support for now.
  //lv_job_steps->setDragDropMode(QAbstractItemView::InternalMove);

  auto addSelectedEngineToJobStep = [this]()
  {
    PE *engine = selectedEngine();
    if (engine == nullptr)
      return;
    JobStepViewListItem *si_job_step = new JobStepViewListItem(engine->name());
    si_job_step->eng_dataset = new EngineDataset(engine);
    // don't allow drop events on individual job steps
    // TODO in the future, allow multi-level job steps (so tree view) to support
    // wrapper scripts. When that time comes, remove this line.
    si_job_step->setFlags(si_job_step->flags() & ~Qt::ItemIsDropEnabled);
    job_steps_model->appendRow(si_job_step);
    lv_job_steps->setCurrentIndex(job_steps_model->indexFromItem(si_job_step));
    emit lv_job_steps->clicked(job_steps_model->indexFromItem(si_job_step));
  };

  // move the selected job step up if up_dir, or down if !up_dir.
  auto moveSelectedJobStep = [this](bool up_dir)
  {
    QModelIndex model_index = lv_job_steps->currentIndex();
    // if no index, index already at the bottom and can't move down, or index 
    // already at the top and can't move up, return
    if (!model_index.isValid()
        || (up_dir && model_index.row() == 0)
        || (!up_dir && model_index.row() == job_steps_model->rowCount()-1)) {
      return;
    }

    int target_row = up_dir ? model_index.row()-1 : model_index.row()+1;
    job_steps_model->insertRow(target_row, job_steps_model->takeRow(model_index.row()));
    lv_job_steps->setCurrentIndex(job_steps_model->indexFromItem(job_steps_model->item(target_row)));
  };

  // add job push button adds the currently selected engine to the list
  connect(a_add_job_step, &QAction::triggered, addSelectedEngineToJobStep);

  // move job step up or down
  connect(a_move_up_job_step, &QAction::triggered, 
          [moveSelectedJobStep](){moveSelectedJobStep(true);});
  connect(a_move_down_job_step, &QAction::triggered, 
          [moveSelectedJobStep](){moveSelectedJobStep(false);});

  // map engine list double click action also too adding engine
  connect(lv_engines, &QAbstractItemView::doubleClicked, addSelectedEngineToJobStep);

  QVBoxLayout *vl_job_steps = new QVBoxLayout();
  vl_job_steps->addWidget(tb_job_steps);
  vl_job_steps->addWidget(lv_job_steps);
  gb_job_steps->setLayout(vl_job_steps);


  // JOB DETAILS
  JobSetupDetailsPane *job_details_pane = new JobSetupDetailsPane();
  QScrollArea *sa_job_details_pane = new QScrollArea();
  sa_job_details_pane->setWidget(job_details_pane);
  sa_job_details_pane->setWidgetResizable(true);

  QPushButton *pb_job_run = new QPushButton("Run");
  QPushButton *pb_job_close = new QPushButton("Close");
  pb_job_run->setShortcut(Qt::Key_Return);
  pb_job_close->setShortcut(Qt::Key_Escape);
  QDialogButtonBox *dbb_job_buttons = new QDialogButtonBox();
  dbb_job_buttons->addButton(pb_job_run, QDialogButtonBox::AcceptRole);
  dbb_job_buttons->addButton(pb_job_close, QDialogButtonBox::RejectRole);

  QVBoxLayout *vl_job_details_pane = new QVBoxLayout();
  vl_job_details_pane->addWidget(sa_job_details_pane);
  vl_job_details_pane->addWidget(dbb_job_buttons);

  QWidget *w_job_details_pane = new QWidget();
  w_job_details_pane->setLayout(vl_job_details_pane);

  auto showSelectedJobStepDetails = [this, job_details_pane](QModelIndex model_index)
  {
    if (!model_index.isValid()) {
      qDebug() << "no item available, clear job details pane.";
      // no item available, clear job details pane
      job_details_pane->setEngineDataset(nullptr);
      return;
    }
    QStandardItem *si_job_step = job_steps_model->item(model_index.row());
    EngineDataset *eng_dataset = static_cast<JobStepViewListItem*>(si_job_step)->eng_dataset;
    job_details_pane->setEngineDataset(eng_dataset);
  };

  // when a job step is clicked, show the details in the job details pane
  connect(lv_job_steps, &QAbstractItemView::clicked, showSelectedJobStepDetails);

  // remove the selected job step and clear pointers
  connect(a_remove_job_step, &QAction::triggered,
          [this, showSelectedJobStepDetails]()
          {
            QModelIndex model_index = lv_job_steps->currentIndex();
            if (!model_index.isValid())
              return;
            auto del_si_job_step = job_steps_model->takeRow(model_index.row());
            delete del_si_job_step.at(0); // destructor of JobStepViewListItem 
                                          // takes care of cleaning up eng_dataset
            // update job pane
            showSelectedJobStepDetails(lv_job_steps->currentIndex());
          });

  // run simulation
  connect(pb_job_run, &QPushButton::clicked,
          [this, job_details_pane]()
          {
            if (job_steps_model->rowCount() == 0) {
              QMessageBox mb_no_js;
              mb_no_js.setText("Job step list is empty.");
              mb_no_js.exec();
              return;
            }
            hide();
            auto job_details = job_details_pane->finalJobDetails();
            if (job_details.name.isEmpty()) {
              job_details.name = comp::SimJob::defaultJobName();
            }
            // create sim job and submit to application
            comp::SimJob *new_job = new comp::SimJob(job_details.name);
            new_job->setInclusionArea(job_details.inclusion_area);
            for (int i=0; i<job_steps_model->rowCount(); i++) {
              QStandardItem *si_job_step = job_steps_model->item(i);
              EngineDataset *eng_dataset = static_cast<JobStepViewListItem*>(si_job_step)->eng_dataset;
              if (eng_dataset == nullptr || eng_dataset->isEmpty())
                continue;
              // create a sim job step and add it to the job
              new_job->addJobStep(new comp::JobStep(eng_dataset->engine,
                                                    eng_dataset->command_format.split("\n"),
                                                    eng_dataset->prop_form->finalProperties()));
            }
            runJob(new_job);
          });

  // close dialog
  connect(pb_job_close, &QPushButton::clicked, this, &QWidget::hide);

  gb_eng_selection->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
  gb_job_steps->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
  job_details_pane->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
  sa_job_details_pane->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

  // set layout and return
  QSplitter *sp_job_setup = new QSplitter();
  sp_job_setup->setOrientation(Qt::Horizontal);
  sp_job_setup->addWidget(gb_eng_selection);
  sp_job_setup->addWidget(gb_job_steps);
  sp_job_setup->addWidget(w_job_details_pane);

  /*
  QWidget *job_setup_panel = new QWidget();
  job_setup_panel->setLayout(hl_job_setup);
  */
  return sp_job_setup;
}

QWidget *JobManager::initJobViewPanel()
{
  job_view_model = new QStandardItemModel();
  job_view_model->setColumnCount(4);  // TODO make dynamic
  tv_job_view = new QTreeView();
  tv_job_view->header()->setStretchLastSection(false);
  tv_job_view->setModel(job_view_model);
  // TODO QTreeView with multiple columns
  // TODO job details (start and end times, job step count, list of invocation commands for job steps)
  // TODO view terminal output
  // TODO view sim results where appropriate
  // TODO allow sorting, sort by newest by default

  return tv_job_view;
}

comp::PluginEngine *JobManager::selectedEngine()
{
  QModelIndex model_index = lv_engines->currentIndex();
  if (!model_index.isValid())
    return nullptr;
  // row: use proxy model to map filtered index to original index
  // col: access unique identifier col
  QStandardItem *si_eng_id = eng_model->item(
      eng_filter_proxy_model->mapToSource(model_index).row(),
      eng_list_fields.indexOf(PE::UniqueIdentifierField));
  return plugin_manager->getEngine(si_eng_id->text().toUInt());
}



JobSetupDetailsPane::JobSetupDetailsPane(QWidget *parent)
  : QWidget(parent)
{
  // main groups
  QGroupBox *gb_job_props = new QGroupBox("Job");
  QGroupBox *gb_plugin_props = new QGroupBox("Plugin Invocation");
  QGroupBox *gb_plugin_params = new QGroupBox("Plugin Runtime Parameters");

  // Job
  le_job_name = new QLineEdit();
  QFormLayout *fl_job_props = new QFormLayout();
  QCheckBox *cb_auto_job_name = new QCheckBox("Auto job name");
  cb_auto_job_name->setChecked(true);
  cbb_inclusion_area = new QComboBox();

  // response to auto job name checkbox
  auto autoJobNameResponse = [this](int check_state)
  {
    le_job_name->setDisabled(check_state == Qt::Checked);
    if (check_state == Qt::Checked) {
      le_job_name->setText("");
    }
  };
  connect(cb_auto_job_name, &QCheckBox::stateChanged, autoJobNameResponse);
  autoJobNameResponse(cb_auto_job_name->checkState());

  // fill in inclusion area combo box
  QMetaEnum inclusion_area_enum = QMetaEnum::fromType<gui::DesignInclusionArea>();
  for (int i=0; i<inclusion_area_enum.keyCount(); i++) {
    cbb_inclusion_area->addItem(inclusion_area_enum.key(i));
  }

  QHBoxLayout *hl_auto_job_name = new QHBoxLayout();
  hl_auto_job_name->addStretch();
  hl_auto_job_name->addWidget(cb_auto_job_name);

  fl_job_props->addRow(new QLabel("Job name"), le_job_name);
  fl_job_props->addRow(hl_auto_job_name);
  fl_job_props->addRow(new QLabel("Inclusion area"), cbb_inclusion_area);
  fl_job_props->setSizeConstraint(QLayout::SetMinimumSize);
  gb_job_props->setLayout(fl_job_props);


  // Plugin Invocation
  te_command = new QTextEdit();
  QToolButton *tb_command_preset = new QToolButton();
  menu_command_preset = new QMenu();
  tb_command_preset->setIcon(QIcon::fromTheme("preferences-system"));
  tb_command_preset->setText("Presets");
  tb_command_preset->setPopupMode(QToolButton::InstantPopup);
  tb_command_preset->setMenu(menu_command_preset);
  tb_command_preset->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

  QHBoxLayout *hl_command = new QHBoxLayout();
  hl_command->addWidget(new QLabel("Invocation command format"));
  hl_command->addStretch();
  hl_command->addWidget(tb_command_preset);

  QFormLayout *fl_plugin_props = new QFormLayout();
  fl_plugin_props->addRow(hl_command);
  fl_plugin_props->addRow(te_command);
  gb_plugin_props->setLayout(fl_plugin_props);

  // update command format in engine dataset to the newest textedit content
  connect(te_command, &QTextEdit::textChanged,
          [this]()
          {
            if (eng_dataset == nullptr)
              return;
            eng_dataset->command_format = te_command->toPlainText();
          });

  // update the command format if a preset format has been chosen by the user
  connect(menu_command_preset, &QMenu::triggered,
          [this](QAction *action)
          {
            int action_i = menu_command_preset->actions().indexOf(action);
            QString chosen_cmd = eng_dataset->engine->jointCommandFormat(action_i).second;
            te_command->setText(chosen_cmd);
          });


  // Plugin Params
  vl_plugin_params = new QVBoxLayout();
  gb_plugin_params->setLayout(vl_plugin_params);

  QVBoxLayout *vl_pane = new QVBoxLayout();
  vl_pane->addWidget(gb_job_props);
  vl_pane->addWidget(gb_plugin_props);
  vl_pane->addWidget(gb_plugin_params);
  vl_pane->addStretch();
  setLayout(vl_pane);
}

void JobSetupDetailsPane::setEngineDataset(JobManager::EngineDataset *t_eng_dataset)
{
  eng_dataset = t_eng_dataset;

  if (eng_dataset == nullptr) {
    // no dataset selected, clear GUI elements
    te_command->setText("");
    menu_command_preset->clear();
    return;
  }

  te_command->setText(eng_dataset->command_format);

  // update engine command preset menu
  menu_command_preset->clear();
  for (auto cmd_format : eng_dataset->engine->commandFormats()) {
    menu_command_preset->addAction(cmd_format.first);
  }

  // remove old property form
  QLayoutItem *child;
  while ((child = vl_plugin_params->takeAt(0)) != 0) {
    QWidget *old_form = child->widget();
    old_form->setParent(nullptr);
    delete child;
  }

  // show new property form
  vl_plugin_params->addWidget(eng_dataset->prop_form);
}
