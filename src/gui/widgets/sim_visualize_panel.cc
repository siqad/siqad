// @file:     sim_visualize_panel.h
// @author:   Samuel
// @created:  2017.10.03
// @editted:  2017.10.03 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     SimVisualize classes

#include "sim_visualize_panel.h"
#include <QImage>


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

void SimVisualize::showJobTerminalOutput()
{
  if(!show_job){
    qWarning() << tr("No job is being shown, cannot show terminal output.");
    return;
  }

  // main widget for the terminal output window
  QWidget *w_job_term = new QWidget(this, Qt::Dialog);

  QPlainTextEdit *te_term_output = new QPlainTextEdit;
  te_term_output->appendPlainText(show_job->terminalOutput());
  QPushButton *button_save_term_out = new QPushButton(tr("Save"));
  QPushButton *button_close_term_out = new QPushButton(tr("Close"));

  button_save_term_out->setShortcut(tr("CTRL+S"));
  button_close_term_out->setShortcut(tr("Esc"));

  connect(button_save_term_out, &QAbstractButton::clicked, show_job, &prim::SimJob::saveTerminalOutput);
  connect(button_close_term_out, &QAbstractButton::clicked, w_job_term, &QWidget::close);

  // layout
  QHBoxLayout *job_term_buttons_hl = new QHBoxLayout;
  job_term_buttons_hl->addStretch(1);
  job_term_buttons_hl->addWidget(button_save_term_out);
  job_term_buttons_hl->addWidget(button_close_term_out);
  QVBoxLayout *job_term_vl = new QVBoxLayout;
  job_term_vl->addWidget(te_term_output);
  job_term_vl->addLayout(job_term_buttons_hl);

  // pop-up widget
  w_job_term->setWindowTitle(tr("%1 Terminal Output").arg(show_job->name()));
  w_job_term->setLayout(job_term_vl);

  w_job_term->show();
}

void SimVisualize::showPotPlot()
{
  QVector<qreal> x_vec;
  QVector<qreal> y_vec;
  qreal pixels_per_angstrom = settings::GUISettings::instance()->get<qreal>("view/scale_fact");
  QList<QVector<float>>::iterator iter = show_job->potentials.begin();
  // first entry will be topLeft
  x_vec.append((*iter)[0]*pixels_per_angstrom);
  y_vec.append((*iter)[1]*pixels_per_angstrom);
  // last entry will be bottomRight
  iter = show_job->potentials.end();
  iter--;
  x_vec.append((*iter)[0]*pixels_per_angstrom);
  y_vec.append((*iter)[1]*pixels_per_angstrom);

  if( x_vec.size() > 0){
    qDebug() << tr("QVectors filled. Size of vectors: %1").arg(x_vec.size());
    QRectF graph_container(QPointF(x_vec.first(),y_vec.first()), QPointF(x_vec.last(),y_vec.last()));
    QDir image_dir = QDir(show_job->resultFilePath(show_job->jobSteps()->length()-1)); // TODO only reading the last step in the job for now, in the future check for which job step is relevant before attempting read
    // find the image generated by PoisSolver
    image_dir.cdUp();
    QString pot_plot_path = image_dir.filePath("SiAirBoundary000.png");
    qDebug() << pot_plot_path;
    QImage potential_plot = QImage(image_dir.filePath("SiAirBoundary000.png"));
    QString pot_anim_path = image_dir.filePath("SiAirBoundary.gif");
    emit clearPotPlots();
    emit showPotPlotOnScene(pot_plot_path, graph_container, pot_anim_path);
  } else {
    qDebug() << tr("QVector size 0. Please see logs for more info.");
  }
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
  if(!show_job || dist_ind < 0 || dist_ind >= show_job->filteredElecDists().size())
    return false;
  text_elec_count->setText(QString::number(show_job->filteredElecDists().at(dist_ind).elec_count));
  emit showElecDistOnScene(show_job, dist_ind);
  return true;
}


void SimVisualize::showElecCountFilter(int check_state)
{
  bool show = check_state == Qt::Checked;
  elec_count_filter_group->setVisible(show);

  // change default filter selection to the electron count of the currently
  // selected configuration
  if (show && slider_dist_sel->sliderPosition() > 0)
    slider_elec_count_sel->setValue(show_job->elec_counts.indexOf(show_job->
          filteredElecDists().at(slider_dist_sel->sliderPosition()-1).elec_count)+1);

  // apply the filter update
  elecCountFilterUpdate(show);
}


void SimVisualize::showPotential()
{
  QDir image_dir = QDir(show_job->resultFilePath(show_job->jobSteps()->length()-1)); // TODO only reading the last step in the job for now, in the future check for which job step is relevant before attempting read
  image_dir.cdUp();
  QImage potential_plot = QImage(image_dir.filePath("SiAirPlot.png"));
  potential_window->clear();
  potential_window->setPixmap(QPixmap::fromImage(potential_plot, Qt::AutoColor));
  potential_window->setScaledContents(true);
  potential_window->show();
}


void SimVisualize::showAverageElecDist()
{
  qDebug() << tr("Showing average electron distribution.");
  emit showElecDistOnScene(show_job, -1);
}


void SimVisualize::showAverageElecDistDegen()
{
  qDebug() << tr("Show degenerate states for distribution %1").arg(slider_dist_sel->sliderPosition());
  emit showElecDistOnScene(show_job, slider_dist_sel->sliderPosition()-1, true);
}


void SimVisualize::updateElecDistOptions()
{
  // slider
  if(!text_dist_selected || !slider_dist_sel || !show_job)
    return;

  if (!show_job->isComplete() || show_job->elec_dists.isEmpty()) {
    // reset electron count filter options
    slider_elec_count_sel->setMinimum(0);
    slider_elec_count_sel->setMaximum(0);
    slider_elec_count_sel->setValue(0);
    text_elec_count->setText("0");

    // reset electron distribution options
    slider_dist_sel->setMinimum(0);
    slider_dist_sel->setMaximum(0);
    slider_dist_sel->setValue(0);
    text_dist_selected->setText("0/0");

    // hide this group
    dist_group->setVisible(false);
  } else {
    // control visibility of relevant groups
    dist_group->setVisible(true);
    elec_count_filter_group->setVisible(false);

    // update electron count filter options
    int elec_counts_size = show_job->elec_counts.size();
    int min_elec_count_sel = elec_counts_size > 0;
    slider_elec_count_sel->setMinimum(min_elec_count_sel);
    slider_elec_count_sel->setMaximum(elec_counts_size);
    slider_elec_count_sel->setValue(min_elec_count_sel);
    text_elec_count->setText(tr("%1").arg(elec_counts_size > 0 ?
        show_job->elec_counts[min_elec_count_sel-1] : min_elec_count_sel));
    cb_elec_count_filter->setCheckState(Qt::Unchecked); // disable the filter by default
    showElecCountFilter(Qt::Unchecked);                 // force no filter workaround

    // update electron distribution options
    slider_dist_sel->setValue(show_job->default_elec_dist_ind+1);
    distSelUpdate();
  }
}

void SimVisualize::updateViewPotentialOptions()
{
  if(!show_job)
    return;
  if (!show_job->isComplete() || show_job->potentials.isEmpty()) {
    view_potential_group->setVisible(false);
  } else {
    view_potential_group->setVisible(true);
    showPotPlot();
  }
}

void SimVisualize::updateOptions()
{
  if (!show_job) {
    // don't show any options
    qDebug() << tr("Job is null, exiting.");
  } else if (!show_job->isComplete()) {
    // maybe also set this up to catch job completion signals, so when a job is complete this panel also updates
    qDebug() << tr("Job isn't complete, exiting. Job Name: %1").arg(show_job->name());
  } else {
    // TODO only showing last step for now, in the future allow user to pick which step
    // group box showing details of the simulation
    QString eng_name = show_job->getJobStep(show_job->jobSteps()->length()-1).engine->name();
    text_job_engine->setText(eng_name);
    text_job_start_time->setText(show_job->startTime().toString("yyyy-MM-dd HH:mm:ss"));
    text_job_end_time->setText(show_job->endTime().toString("yyyy-MM-dd HH:mm:ss"));

    updateElecDistOptions();
    updateViewPotentialOptions();
    qDebug() << tr("Engine Name: %1").arg(eng_name);

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

  button_show_term_out = new QPushButton("Show Terminal Output");

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
  job_info_layout->addWidget(button_show_term_out);
  job_info_group->setLayout(job_info_layout);

  // View Potential Group
  view_potential_group = new QGroupBox(tr("View Potential"));
  QPushButton *button_view_potential = new QPushButton(tr(("Open viewer window")));
  QHBoxLayout *view_potential_hl = new QHBoxLayout;
  view_potential_hl->addWidget(button_view_potential);
  QVBoxLayout *view_potential_vl = new QVBoxLayout;
  view_potential_vl->addLayout(view_potential_hl);
  view_potential_group->setLayout(view_potential_vl);
  potential_window = new QLabel();


  // Elec Distribution Group
  dist_group = new QGroupBox(tr("Electron Distribution"));

  // choose the elec distribution ind
  QLabel *label_dist_sel = new QLabel(tr("Dist:"));
  QPushButton *button_dist_prev = new QPushButton(tr("<"));
  QPushButton *button_dist_next = new QPushButton(tr(">"));
  text_dist_selected = new QLabel("0/0");

  // show the energy of the configuration being viewed
  QLabel *label_dist_energy = new QLabel("Energy:");
  text_dist_energy = new QLabel("0");
  QLabel *label_dist_energy_unit = new QLabel("eV");

  // show the electron count and option to filter
  QLabel *label_elec_count_filter = new QLabel(tr("Electron Count:"));
  text_elec_count = new QLabel("0");
  cb_elec_count_filter = new QCheckBox("Filter");
  elec_count_filter_group = new QGroupBox(tr("Electron Count Filter"));
  QPushButton *button_elec_count_prev = new QPushButton(tr("<"));
  QPushButton *button_elec_count_next = new QPushButton(tr(">"));

  // show the average distribution for one of the few presets
  QLabel *label_average_elec_dist = new QLabel("Show average for:");
  QPushButton *button_average_elec_dist_all = new QPushButton(tr("All distributions"));
  QPushButton *button_average_elec_dist_degen = new QPushButton(tr("Degenerate states"));

  button_dist_prev->setShortcut(tr("CTRL+H"));
  button_dist_next->setShortcut(tr("CTRL+L"));

  slider_elec_count_sel = new QSlider(Qt::Horizontal);
  slider_dist_sel = new QSlider(Qt::Horizontal);
  updateElecDistOptions();
  updateViewPotentialOptions();

  // choose distribution slider
  QHBoxLayout *dist_sel_hl = new QHBoxLayout;
  dist_sel_hl->addWidget(label_dist_sel);
  dist_sel_hl->addWidget(slider_dist_sel);

  // buttons for distribution choosing
  QHBoxLayout *dist_sel_buttons_hl = new QHBoxLayout;
  dist_sel_buttons_hl->addWidget(button_dist_prev);
  dist_sel_buttons_hl->addWidget(text_dist_selected);
  dist_sel_buttons_hl->addWidget(button_dist_next);

  // energy
  QHBoxLayout *dist_energy_hl = new QHBoxLayout;
  dist_energy_hl->addWidget(label_dist_energy);
  dist_energy_hl->addWidget(text_dist_energy);
  dist_energy_hl->addWidget(label_dist_energy_unit);

  // show electron count and option to filter
  QHBoxLayout *elec_count_hl = new QHBoxLayout;
  elec_count_hl->addWidget(label_elec_count_filter);
  elec_count_hl->addWidget(text_elec_count);
  elec_count_hl->addWidget(cb_elec_count_filter);

  // buttons for navigating through electron count selections
  QHBoxLayout *elec_count_filter_options_hl = new QHBoxLayout;
  elec_count_filter_options_hl->addWidget(button_elec_count_prev);
  elec_count_filter_options_hl->addWidget(slider_elec_count_sel);
  elec_count_filter_options_hl->addWidget(button_elec_count_next);

  elec_count_filter_group->setLayout(elec_count_filter_options_hl);

  // average
  QHBoxLayout *dist_average_hl = new QHBoxLayout;
  QVBoxLayout *dist_average_buttons_hl = new QVBoxLayout;
  dist_average_hl->addWidget(label_average_elec_dist);
  dist_average_buttons_hl->addWidget(button_average_elec_dist_all);
  dist_average_buttons_hl->addWidget(button_average_elec_dist_degen);
  dist_average_hl->addLayout(dist_average_buttons_hl);

  // entire elec distribution group
  QVBoxLayout *dist_vl = new QVBoxLayout;
  dist_vl->addLayout(dist_sel_hl);
  dist_vl->addLayout(dist_sel_buttons_hl);
  dist_vl->addLayout(dist_energy_hl);
  dist_vl->addLayout(elec_count_hl);
  //dist_vl->addLayout(elec_count_filter_options_hl);
  dist_vl->addWidget(elec_count_filter_group);
  dist_vl->addLayout(dist_average_hl);

  dist_group->setLayout(dist_vl);

  // signal connection
  connect(button_show_term_out, &QAbstractButton::clicked,
          this, &gui::SimVisualize::showJobTerminalOutput);
  connect(combo_job_sel, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this, &gui::SimVisualize::jobSelUpdate);
  connect(slider_dist_sel, static_cast<void(QSlider::*)(int)>(&QSlider::valueChanged),
          this, &gui::SimVisualize::distSelUpdate);
  connect(button_dist_prev, &QAbstractButton::clicked,
          this, &gui::SimVisualize::distPrev);
  connect(button_dist_next, &QAbstractButton::clicked,
          this, &gui::SimVisualize::distNext);
  connect(slider_elec_count_sel, static_cast<void(QSlider::*)(int)>(&QSlider::valueChanged),
          this, &gui::SimVisualize::elecCountFilterUpdate);
  connect(button_elec_count_prev, &QAbstractButton::clicked,
          this, &gui::SimVisualize::elecCountPrev);
  connect(button_elec_count_next, &QAbstractButton::clicked,
          this, &gui::SimVisualize::elecCountNext);
  connect(cb_elec_count_filter, &QCheckBox::stateChanged,
          this, &gui::SimVisualize::showElecCountFilter);
  connect(button_average_elec_dist_all, &QAbstractButton::clicked,
          this, &gui::SimVisualize::showAverageElecDist);
  connect(button_average_elec_dist_degen, &QAbstractButton::clicked,
          this, &gui::SimVisualize::showAverageElecDistDegen);
  connect(button_view_potential, &QAbstractButton::clicked,
          this, &gui::SimVisualize::showPotential);
  // TODO show energy level, and maybe sorting feature

  QVBoxLayout *visualize_layout = new QVBoxLayout;
  visualize_layout->addWidget(job_info_group);
  visualize_layout->addWidget(dist_group);
  visualize_layout->addWidget(view_potential_group);
  //visualize_layout->addStretch(1);
  setLayout(visualize_layout);
}


void SimVisualize::jobSelUpdate()
{
  showJob(combo_job_sel->currentIndex());
}


void SimVisualize::elecCountFilterUpdate(bool apply_filter)
{
  if (show_job->elec_counts.isEmpty())
    return;

  int elec_counts_ind = slider_elec_count_sel->sliderPosition() - 1;
  text_elec_count->setText(tr("%1").arg(show_job->elec_counts[elec_counts_ind]));

  // save the current distribution being shown so it can be reselected later
  prim::SimJob::elecDist show_dist;
  int slider_pos = slider_dist_sel->sliderPosition(); // slider pos = dist_ind + 1
  if (slider_pos > 0 && slider_pos <= show_job->filteredElecDists().size())
    show_dist = show_job->filteredElecDists().at(slider_dist_sel->sliderPosition()-1);

  if (apply_filter)
    show_job->applyElecDistsFilter(show_job->elec_counts[elec_counts_ind]);
  else
    show_job->applyElecDistsFilter(-1);

  // find the new index of the current distribution being shown
  int dist_ind = 0;
  if (slider_dist_sel->sliderPosition() > 0)
    dist_ind = show_job->filteredElecDists().indexOf(show_dist) + 1;

  // TODO refactor this part with updateElecDistOptions
  int dist_count = show_job->filteredElecDists().size();
  int min_sel = dist_count > 0;
  slider_dist_sel->setMinimum(min_sel);
  slider_dist_sel->setMaximum(dist_count);
  slider_dist_sel->setValue(dist_ind);
  text_dist_selected->setText(tr("%1/%2").arg(show_job->default_elec_dist_ind+1).arg(dist_count));
  distSelUpdate();
}


void SimVisualize::elecCountPrev()
{
  if(!slider_elec_count_sel)
    return;

  if(slider_elec_count_sel->value() > 1)
    slider_elec_count_sel->setValue(slider_elec_count_sel->value() - 1);
}

void SimVisualize::elecCountNext()
{
  if (!slider_elec_count_sel)
    return;

  if (!(slider_elec_count_sel->value() + 1 > slider_elec_count_sel->maximum()))
    slider_elec_count_sel->setValue(slider_elec_count_sel->value() + 1);
}


void SimVisualize::distSelUpdate()
{
  if (show_job->elec_dists.isEmpty())
    return;
  int elec_ind = slider_dist_sel->sliderPosition() - 1;
  text_dist_selected->setText(tr("%1/%2").arg(
      slider_dist_sel->value()).arg(show_job->filteredElecDists().size()));
  showElecDist(elec_ind);

  QString energy_text;
  if (elec_ind >= 0 && elec_ind < show_job->filteredElecDists().size())
    energy_text = QString::number(show_job->filteredElecDists().at(elec_ind).energy);
  else
    energy_text = "--";
  text_dist_energy->setText(energy_text);
}

void SimVisualize::distPrev()
{
  if(!slider_dist_sel)
    return;

  if(slider_dist_sel->value() > 1)
    slider_dist_sel->setValue(slider_dist_sel->value() - 1);
}

void SimVisualize::distNext()
{
  if (!slider_dist_sel)
    return;

  if (!(slider_dist_sel->value() + 1 > slider_dist_sel->maximum()))
    slider_dist_sel->setValue(slider_dist_sel->value() + 1);
}


} // end gui namespace
