// @file:     sim_visualizer.cc
// @author:   Samuel
// @created:  2017.10.03
// @license:  GNU LGPL v3
//
// @desc:     SimVisualizer classes

#include <QImage>
#include <QtCharts/QChartView>
#include <QtCharts/QScatterSeries>

#include "sim_visualizer.h"
#include "settings/settings.h"

using namespace gui;

typedef comp::JobResult JR;
typedef comp::ChargeConfigSet ECS;
typedef comp::PotentialLandscape PL;

// Qt::Dialog makes the main window unclickable. Use Qt::Window if this behavior should be changed.
SimVisualizer::SimVisualizer(DesignPanel *design_pan, QWidget *parent)
  : QWidget(parent, Qt::Widget), design_pan(design_pan)
{
  // basic job information
  gb_job_info = new QGroupBox("Job Information");
  QVBoxLayout *vl_job_info = new QVBoxLayout(gb_job_info);
  QHBoxLayout *hl_job_actions = new QHBoxLayout(gb_job_info);
  job_info_model = new QStandardItemModel();
  tv_job_info = new QTableView();
  tv_job_info->setModel(job_info_model);
  tv_job_info->horizontalHeader()->hide();
  tv_job_info->verticalHeader()->hide();
  QPushButton *pb_job_terminal = new QPushButton("Log");
  QPushButton *pb_open_result_path = new QPushButton("Result Directory");
  vl_job_info->addWidget(tv_job_info);
  hl_job_actions->addWidget(pb_job_terminal);
  hl_job_actions->addWidget(pb_open_result_path);
  vl_job_info->addLayout(hl_job_actions);

  // show job terminal
  connect(pb_job_terminal, &QPushButton::clicked,
          [this]()
          {
            if (sim_job != nullptr) {
              sim_job->terminalOutputDialog(this)->show();
            }
          });

  // open result directory
  connect(pb_open_result_path, &QPushButton::clicked,
          [this]()
          {
            if (sim_job != nullptr) {
              QDesktopServices::openUrl(QUrl::fromLocalFile(sim_job->runtimeTempPath()));
            }
          });

  // TODO the following can probably be templated

  // charge configuration results
  charge_config_set_visualizer = new ChargeConfigSetVisualizer(design_pan);
  gb_charge_configs = new QGroupBox("Charge Configurations");
  cb_job_steps_charge_configs = new QComboBox();
  QToolButton *tb_refresh_job_steps_charge_configs = new QToolButton();
  tb_refresh_job_steps_charge_configs->setIcon(QIcon::fromTheme("view-refresh"));
  tb_refresh_job_steps_charge_configs->setText("Refresh");
  tb_refresh_job_steps_charge_configs->setToolTip("Refresh result view");
  QHBoxLayout *hl_job_steps_charge_configs = new QHBoxLayout();
  hl_job_steps_charge_configs->addWidget(new QLabel("Relevant job steps"));
  hl_job_steps_charge_configs->addWidget(cb_job_steps_charge_configs);
  hl_job_steps_charge_configs->addWidget(tb_refresh_job_steps_charge_configs);
  QVBoxLayout *vl_charge_configs = new QVBoxLayout();
  vl_charge_configs->addLayout(hl_job_steps_charge_configs);
  vl_charge_configs->addWidget(charge_config_set_visualizer);
  gb_charge_configs->setLayout(vl_charge_configs);

  auto setChargeConfigSetJobStep = [this](const int &job_step_ind)
  {
    comp::JobStep *js = sim_job->getJobStep(job_step_ind);
    ECS *charge_config_set = static_cast<ECS*>(
        js->jobResults().value(comp::JobResult::ChargeConfigsResult));
    charge_config_set_visualizer->setChargeConfigSet(charge_config_set);
  };

  // update electron config set selection GUI elements when a job step is selected
  connect(cb_job_steps_charge_configs, &QComboBox::currentTextChanged,
          [setChargeConfigSetJobStep](const QString &str_job_step_ind)
          {
            if (str_job_step_ind.isEmpty())
              return;
            setChargeConfigSetJobStep(str_job_step_ind.toInt());
          });

  // same as above but explicitly for user manual activation
  connect(cb_job_steps_charge_configs, QOverload<const QString &>::of(&QComboBox::activated),
          [setChargeConfigSetJobStep](const QString &str_job_step_ind)
          {
            if (str_job_step_ind.isEmpty())
              return;
            setChargeConfigSetJobStep(str_job_step_ind.toInt());
          });

  // map refresh button to reactivate the currently selected electron config set step
  connect(tb_refresh_job_steps_charge_configs, &QToolButton::pressed,
          [this, setChargeConfigSetJobStep]()
          {
            QString str_job_step_ind = cb_job_steps_charge_configs->currentText();
            if (str_job_step_ind.isEmpty())
              return;
            setChargeConfigSetJobStep(str_job_step_ind.toInt());
          });

  // potential landscape results
  pot_landscape_visualizer = new PotentialLandscapeVisualizer(design_pan);
  gb_pot_landscape = new QGroupBox("Potential Landscape");
  cb_job_steps_pot_landscape = new QComboBox();
  QToolButton *tb_refresh_job_steps_pot_landscape = new QToolButton();
  tb_refresh_job_steps_pot_landscape->setIcon(QIcon::fromTheme("view-refresh"));
  tb_refresh_job_steps_pot_landscape->setText("Refresh");
  tb_refresh_job_steps_pot_landscape->setToolTip("Refresh result view");
  QHBoxLayout *hl_job_steps_pot_landscape = new QHBoxLayout();
  hl_job_steps_pot_landscape->addWidget(new QLabel("Relevant job steps"));
  hl_job_steps_pot_landscape->addWidget(cb_job_steps_pot_landscape);
  hl_job_steps_pot_landscape->addWidget(tb_refresh_job_steps_pot_landscape);
  QVBoxLayout *vl_pot_landscape = new QVBoxLayout();
  vl_pot_landscape->addLayout(hl_job_steps_pot_landscape);
  vl_pot_landscape->addWidget(pot_landscape_visualizer);
  gb_pot_landscape->setLayout(vl_pot_landscape);

  auto setPotentialLandscapeJobStep = [this](const int &job_step_ind)
  {
    comp::JobStep *js = sim_job->getJobStep(job_step_ind);
    PL *pot_landscape = static_cast<PL*>(
        js->jobResults().value(comp::JobResult::PotentialLandscapeResult));
    pot_landscape_visualizer->setPotentialLandscape(pot_landscape);
  };

  // update potential landscape results selection GUI elements when a job step
  // is selected
  connect(cb_job_steps_pot_landscape, &QComboBox::currentTextChanged,
          [setPotentialLandscapeJobStep](const QString &str_job_step_ind)
          {
            if (str_job_step_ind.isEmpty())
              return;
            setPotentialLandscapeJobStep(str_job_step_ind.toInt());
          });

  // same as above but explicitly for user manual activation
  connect(cb_job_steps_pot_landscape, QOverload<const QString &>::of(&QComboBox::activated),
          [setPotentialLandscapeJobStep](const QString &str_job_step_ind)
          {
            if (str_job_step_ind.isEmpty())
              return;
            setPotentialLandscapeJobStep(str_job_step_ind.toInt());}
          );

  // map refresh button to reactivate the currently selected potential landscape step
  connect(tb_refresh_job_steps_pot_landscape, &QToolButton::pressed,
          [this, setPotentialLandscapeJobStep]()
          {
            QString str_job_step_ind = cb_job_steps_charge_configs->currentText();
            if (str_job_step_ind.isEmpty())
              return;
            setPotentialLandscapeJobStep(cb_job_steps_pot_landscape->currentText().toInt());
          });

  // set widget layout
  QVBoxLayout *vl_main = new QVBoxLayout();
  vl_main->addWidget(gb_job_info);
  vl_main->addWidget(gb_charge_configs);
  vl_main->addWidget(gb_pot_landscape);
  vl_main->addStretch();

  setLayout(vl_main);
}

void SimVisualizer::showJob(comp::SimJob *job)
{
  sim_job = job;
  qDebug() << tr("Showing job %1").arg(job->name());
  setEnabled(true);

  emit sig_showJobInvoked(job);

  // generic job information
  job_info_model->clear();
  typedef comp::SimJob::JobInfoStandardItemField SIF;
  QList<SIF> info_list = QList<SIF>({
        SIF::JobNameField,
        SIF::JobStartTimeField,
        SIF::JobEndTimeField,
        SIF::JobStepCountField,
        SIF::JobTempPathField
      });
  QList<QStandardItem*> labels = QList<QStandardItem*>({
        new QStandardItem("Name"),
        new QStandardItem("Start time"),
        new QStandardItem("End time"),
        new QStandardItem("Step count"),
        new QStandardItem("Temp path")
      });
  QList<QStandardItem*> job_si_row = job->jobInfoStandardItemRow(info_list);
  job_info_model->appendColumn(labels);
  job_info_model->appendColumn(job_si_row);
  tv_job_info->resizeColumnsToContents();

  // TODO update siqadconn to put physloc and elecconfigresult inside the same parent node
  QList<JR::ResultType> result_types = job->resultTypeStepMap().uniqueKeys();

  // deal with ChargeConfigsResult type
  cb_job_steps_charge_configs->clear();
  if (result_types.contains(JR::ChargeConfigsResult)) {
    gb_charge_configs->setEnabled(true);
    for (comp::JobStep *step : job->resultTypeStepMap().values(JR::ChargeConfigsResult)) {
      cb_job_steps_charge_configs->addItem(QString::number(step->jobStepPlacement()));
    }
  } else {
    gb_charge_configs->setEnabled(false);
  }

  // deal with PotentialLandscapeResult type
  cb_job_steps_pot_landscape->clear();
  if (result_types.contains(JR::PotentialLandscapeResult)) {
    gb_pot_landscape->setEnabled(true);
    for (comp::JobStep *step : job->resultTypeStepMap().values(JR::PotentialLandscapeResult)) {
      cb_job_steps_pot_landscape->addItem(QString::number(step->jobStepPlacement()));
    }
  } else {
    gb_pot_landscape->setEnabled(false);
  }
}

void SimVisualizer::clearJob()
{
  // clear job information from data model in this widget and from children
  // widgets
  job_info_model->clear();

  charge_config_set_visualizer->clearVisualizer();
  pot_landscape_visualizer->clearVisualizer();

  sim_job = nullptr;

  // disable user interaction to the entire plugin
  setEnabled(false);

  // TODO doesn't belong here but move it to the appropriate location: terminal
  // output dialog button somewhere
}
