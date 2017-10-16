// @file:     sim_visualize_panel.h
// @author:   Samuel
// @created:  2017.10.03
// @editted:  2017.10.03 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     SimVisualize classes

#include "sim_visualize_panel.h"


namespace gui{

// Qt::Dialog makes the main window unclickable. Use Qt::Window if this behavior should be changed.
SimVisualize::SimVisualize(SimManager *sim_man, QWidget *parent)
  : QWidget(parent, Qt::Widget), sim_manager(sim_man), show_job(0)
{
  initSimVisualize();
}



bool SimVisualize::setManager(SimManager *sim_man)
{
  if(!sim_man)
    return false;
  sim_manager = sim_man;
  return true;
}


bool SimVisualize::showJob(int job_ind)
{
  if(job_ind < 0 || job_ind >= sim_manager->sim_jobs.size()){
    // TODO throw error
    return false;
  }
  return showJob(sim_manager->sim_jobs[job_ind]);
}


bool SimVisualize::showJob(prim::SimJob *job)
{
  if(!job)
    return false;
  show_job = job;
  updateOptions();
  return true;
}


void SimVisualize::updateJobSelCombo()
{
  if(!combo_job_sel)
    return;
  combo_job_sel->clear();

  if(sim_manager->sim_jobs.isEmpty())
    combo_job_sel->addItem("No Jobs");
  else
    for(auto job : sim_manager->sim_jobs)
      combo_job_sel->addItem(job->name());
}


bool SimVisualize::showElecDist(int dist_ind)
{
  if(!show_job || dist_ind < 0 || dist_ind >= show_job->elec_dists.size())
    return false;
  // TODO emit signal telling design_panel to show dist_ind of show_job->elec_dists
  emit showElecDistOnScene(show_job, dist_ind);
  return true;
}


void SimVisualize::updateElecDistCombo()
{
  if(!combo_dist_sel || !show_job)
    return;
  combo_dist_sel->clear();

  if(!show_job->isComplete() || show_job->elec_dists.isEmpty())
    combo_dist_sel->addItem("No Distribution");
  else{
    // primitive solution, need to figure out something better
    // like better ways to visualize degenerate states, etc
    int i=0;
    for(auto dist : show_job->elec_dists)
      combo_dist_sel->addItem(QString::number(i++));
  }
}


void SimVisualize::updateOptions()
{
  if(!show_job){
    // TODO don't show any options
    qDebug() << tr("Job is null, exiting.");
  }
  else if(!show_job->isComplete()){
    // TODO show that the job is not complete
    // maybe also set this up to catch job completion signals, so when a job is complete this panel also updates?
    qDebug() << tr("Job isn't complete, exiting. Job Name: %1").arg(show_job->name());
  }
  else{
    // group box showing details of the simulation
    text_job_engine->setText(show_job->engineName());
    text_job_start_time->setText(show_job->startTime().toString("yyyy-MM-dd HH:mm:ss"));
    text_job_end_time->setText(show_job->endTime().toString("yyyy-MM-dd HH:mm:ss"));
      // name - button for rename

    // TODO result type selector (not needed for now)

    // elec dist selection
    updateElecDistCombo();
    
    // group box result filter
      // energies
      // # elecs
      // etc
  }
}




// PRIVATE

void SimVisualize::initSimVisualize()
{
  // Job Info Group
  QGroupBox *job_info_group = new QGroupBox(tr("Job Information"));
  QLabel *label_job_sel = new QLabel(tr("Job:"));
  QLabel *label_job_engine = new QLabel(tr("Engine:"));
  QLabel *label_job_start_time = new QLabel(tr("Start time:"));
  QLabel *label_job_end_time = new QLabel(tr("End time:"));

  combo_job_sel = new QComboBox();
  combo_job_sel->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  updateJobSelCombo();

  text_job_engine = new QLabel;
  text_job_start_time = new QLabel;
  text_job_end_time = new QLabel;

  text_job_engine->setAlignment(Qt::AlignRight);
  text_job_start_time->setAlignment(Qt::AlignRight);
  text_job_end_time->setAlignment(Qt::AlignRight);

  label_job_sel->setBuddy(combo_job_sel);
  label_job_engine->setBuddy(text_job_engine);
  label_job_start_time->setBuddy(text_job_start_time);
  label_job_end_time->setBuddy(text_job_end_time);

  QHBoxLayout *job_sel_hl = new QHBoxLayout;
  QHBoxLayout *job_engine_hl = new QHBoxLayout;
  QHBoxLayout *job_start_time_hl = new QHBoxLayout;
  QHBoxLayout *job_end_time_hl = new QHBoxLayout;
  job_sel_hl->addWidget(label_job_sel);
  job_sel_hl->addWidget(combo_job_sel);
  job_engine_hl->addWidget(label_job_engine);
  job_engine_hl->addWidget(text_job_engine);
  job_start_time_hl->addWidget(label_job_start_time);
  job_start_time_hl->addWidget(text_job_start_time);
  job_end_time_hl->addWidget(label_job_end_time);
  job_end_time_hl->addWidget(text_job_end_time);

  QVBoxLayout *job_info_layout = new QVBoxLayout();
  job_info_layout->addLayout(job_sel_hl);
  job_info_layout->addLayout(job_engine_hl);
  job_info_layout->addLayout(job_start_time_hl);
  job_info_layout->addLayout(job_end_time_hl);
  job_info_group->setLayout(job_info_layout);

  // Elec Distribution Group
  QGroupBox *dist_group = new QGroupBox(tr("Electron Distribution"));
  QLabel *label_dist_sel = new QLabel(tr("Dist:"));
  combo_dist_sel = new QComboBox;
  label_dist_sel->setBuddy(combo_job_sel);
  updateElecDistCombo();

  QHBoxLayout *dist_sel = new QHBoxLayout;
  dist_sel->addWidget(label_dist_sel);
  dist_sel->addWidget(combo_dist_sel);

  QVBoxLayout *dist_layout = new QVBoxLayout;
  dist_layout->addLayout(dist_sel);
  dist_group->setLayout(dist_layout);

  // signal connection
  connect(combo_job_sel, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &gui::SimVisualize::jobSelUpdate);
  connect(combo_dist_sel, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &gui::SimVisualize::distSelUpdate);

  // TODO show energy level, and maybe sorting feature

  QVBoxLayout *visualize_layout = new QVBoxLayout;
  visualize_layout->addWidget(job_info_group);
  visualize_layout->addWidget(dist_group);
  visualize_layout->addStretch(1);
  setLayout(visualize_layout);
}


void SimVisualize::jobSelUpdate()
{
  showJob(combo_job_sel->currentIndex());
}


void SimVisualize::distSelUpdate()
{
  showElecDist(combo_dist_sel->currentIndex());
}


} // end gui namespace
