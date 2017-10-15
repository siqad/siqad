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
  : QWidget(parent, Qt::Widget), sim_manager(sim_man)
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


bool SimVisualize::setJob(int job_ind)
{
  auto jobs = sim_manager->jobs();
  if(job_ind < 0 || job_ind >= jobs.size()){
    // TODO throw error
    return false;
  }
  //return setJob(jobs[job_ind]);
  return setJob(sim_manager->jobs()[job_ind]);
}


bool SimVisualize::setJob(prim::SimJob *job)
{
  if(!job)
    return false;
  curr_job = job;
  updateOptions();
  return true;
}


void SimVisualize::updateJobList()
{
  if(!combo_job_sel)
    return;
  combo_job_sel->clear();

  if(sim_manager->jobs().isEmpty())
    combo_job_sel->addItem("No Jobs");
  else
    for(auto job : sim_manager->jobs())
      combo_job_sel->addItem(job->name());
}


void SimVisualize::updateOptions()
{
  int job_ind = combo_job_sel->currentIndex();
  qDebug() << tr("Index: %1").arg(job_ind);

  if(job_ind < 0) return;

  if(!curr_job){
    // TODO don't show any options
    qDebug() << tr("Job is null, exiting.");
  }
  else if(!curr_job->isComplete()){
    // TODO show that the job is not complete
    // maybe also set this up to catch job completion signals, so when a job is complete this panel also updates?
    //qDebug() << tr("Job isn't complete, exiting. Job Name: %1").arg(sim_manager->jobs()[combo_job_sel->currentIndex()]->name());
    qDebug() << tr("Job isn't complete, exiting. Job Name: %1").arg(curr_job->name());
  }
  else{
    // TODO actually update the options
    qDebug() << tr("This job exists, good job Samuel.");
  }
}




// PRIVATE

void SimVisualize::initSimVisualize()
{
  // TODO this is getting way too messy, just nuke this and put everything in sim_manager.

  // always show job selection
  QLabel *label_job_sel = new QLabel(tr("Job:"));
  combo_job_sel = new QComboBox();
  combo_job_sel->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  label_job_sel->setBuddy(combo_job_sel);
  if(sim_manager->jobs().isEmpty())
    combo_job_sel->addItem("No Jobs");
  else
    for(auto job : sim_manager->jobs())
      combo_job_sel->addItem(job->name());

  // TODO connect combo_job_sel currentIndexChanged to updateOptions
  connect(combo_job_sel, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &gui::SimVisualize::updateOptions);

  QHBoxLayout *job_sel = new QHBoxLayout;
  job_sel->addWidget(label_job_sel);
  job_sel->addWidget(combo_job_sel);

  QVBoxLayout *visualize_layout = new QVBoxLayout;
  visualize_layout->addLayout(job_sel);

  // if no job is selected, show nothing else - easiest for now. In the future maybe still show options but grayed out
  // if an incomplete job is selected, say it's incomplete
  // else show actual stuff

  // TODO for now, only show options to choose which result to show, maybe get rid of duplicated ones but keep the count of occurance for duplicated results
  // TODO next, show energy level, and maybe sorting feature
  // TODO that's about time to make slides

  visualize_layout->addStretch(1);
  setLayout(visualize_layout);
}


} // end gui namespace
