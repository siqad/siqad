// @file:     sim_visualize_panel.h
// @author:   Samuel
// @created:  2017.10.03
// @editted:  2017.10.03 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     SimVisualize classes

#include "sim_visualize_panel.h"
#include "../../qcustomplot.h"
#include <QPixmap>
#include <QtMath>


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
  QCustomPlot *customPlot = new QCustomPlot();
  QVector<qreal> x_vec;
  QVector<qreal> y_vec;
  QVector<qreal> val_vec;
  qreal pixels_per_angstrom = settings::GUISettings::instance()->get<qreal>("view/scale_fact");
  qreal angstrom_per_meter = 1E10;
  qDebug() << tr("fill QVectors");
  QList<QVector<float>>::iterator iter;
  for(iter=show_job->potentials.begin(); iter!=show_job->potentials.end(); ++iter){
    x_vec.append((*iter)[0]*angstrom_per_meter*pixels_per_angstrom);
    y_vec.append((*iter)[1]*angstrom_per_meter*pixels_per_angstrom);
    val_vec.append((*iter)[2]);
  }
  if( x_vec.size() > 0){
    qDebug() << tr("QVectors filled. Size of vectors: %1").arg(x_vec.size());
    QCPColorMap *colorMap = new QCPColorMap(customPlot->xAxis, customPlot->yAxis);
    int nx = qSqrt(x_vec.size());
    int ny = qSqrt(y_vec.size());
    colorMap->data()->setSize(nx, ny);
    // qDebug() << tr("Setting colurMap");
    // set range for x and y in pixels.
    colorMap->data()->setRange(QCPRange(x_vec.first(), x_vec.last()), QCPRange(y_vec.first(), y_vec.last()));

    qDebug() << tr("fill colorMap");
    // now we assign some data, by accessing the QCPColorMapData instance of the color map:
    int x_ind, y_ind;
    for (int i=0; i<nx; ++i){
      for (int j=0; j<ny; ++j){
        // get corresponding cell index from coordinate
        // qDebug() << tr("i: %1, j: %2").arg(i).arg(j);
        colorMap->data()->coordToCell(x_vec[i*nx + j], y_vec[i*nx + j], &x_ind, &y_ind);
        colorMap->data()->setCell(x_ind, y_ind, val_vec[i*nx + j]);
      }
    }
    qDebug() << tr("colorMap filled");

    // configure axis rect:
    // customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom); // this will also allow rescaling the color scale by dragging/zooming
    // customPlot->axisRect()->setupFullAxesBox(true);
    // customPlot->xAxis->setLabel("x");
    // customPlot->yAxis->setLabel("y");
    // down on graph is increase in y.
    customPlot->yAxis->setRangeReversed(true);

    // add a color scale:
    QCPColorScale *colorScale = new QCPColorScale(customPlot);
    // customPlot->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
    // colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    colorMap->setColorScale(colorScale); // associate the color map with the color scale
    // colorScale->axis()->setLabel("Magnetic Field Strength");

    // set the color gradient of the color map to one of the presets:
    colorMap->setGradient(QCPColorGradient::gpPolar);

    // rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
    colorMap->rescaleDataRange();

    // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
    // QCPMarginGroup *marginGroup = new QCPMarginGroup(customPlot);
    // customPlot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    // colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

    // rescale the key (x) and value (y) axes so the whole color map is visible:

    customPlot->xAxis->setVisible(false);
    customPlot->yAxis->setVisible(false);
    customPlot->setContentsMargins(0,0,0,0);
    customPlot->axisRect()->setAutoMargins(QCP::msNone);
    customPlot->axisRect()->setMargins(QMargins(0,0,0,0));

    customPlot->rescaleAxes();
    QPixmap potential_plot = customPlot->toPixmap();
    QRectF graph_container(QPointF(x_vec.first(),y_vec.first()), QPointF(x_vec.last(),y_vec.last()));
    emit showPotPlotOnScene(potential_plot, graph_container);
  } else {
    qDebug() << tr("QVector size 0. Please see logs for more info.");
  }
}


// void QCustomPlot::showPointToolTip(QMouseEvent *event)
// {
//   qDebug() << tr("showPointToolTip");
//   int x = this->xAxis->pixelToCoord(event->pos().x());
//   int y = this->yAxis->pixelToCoord(event->pos().y());
//
//   setToolTip(QString("%1 , %2").arg(x).arg(y));
// }

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
  emit showElecDistOnScene(show_job, dist_ind);
  return true;
}


void SimVisualize::showAverageElecDist()
{
  emit showElecDistOnScene(show_job, -1);
}


void SimVisualize::showAverageElecDistDegen()
{
  emit showElecDistOnScene(show_job, combo_job_sel->currentIndex(), true);
}


void SimVisualize::updateElecDistOptions()
{
  // slider
  if(!text_dist_selected || !slider_dist_sel || !show_job)
    return;

  if (!show_job->isComplete() || show_job->elec_dists.isEmpty()) {
    dist_group->setVisible(false);
    slider_dist_sel->setMinimum(0);
    slider_dist_sel->setMaximum(0);
    slider_dist_sel->setValue(0);
    text_dist_selected->setText("0/0");
  } else {
    dist_group->setVisible(true);
    int dist_count = show_job->elec_dists.size();
    int min_sel = dist_count > 0;
    slider_dist_sel->setMinimum(min_sel);
    slider_dist_sel->setMaximum(dist_count);
    slider_dist_sel->setValue(min_sel);
    text_dist_selected->setText(tr("%1/%2").arg(min_sel).arg(dist_count));
    distSelUpdate();
  }
}


void SimVisualize::updateOptions()
{
  if (!show_job) {
    // don't show any options
    qDebug() << tr("Job is null, exiting.");
  } else if (!show_job->isComplete()) {
    // TODO show that the job is not complete
    // maybe also set this up to catch job completion signals, so when a job is complete this panel also updates
    qDebug() << tr("Job isn't complete, exiting. Job Name: %1").arg(show_job->name());
  } else {
    // group box showing details of the simulation
    text_job_engine->setText(show_job->engineName());
    text_job_start_time->setText(show_job->startTime().toString("yyyy-MM-dd HH:mm:ss"));
    text_job_end_time->setText(show_job->endTime().toString("yyyy-MM-dd HH:mm:ss"));

    // TODO result type selector
    updateElecDistOptions();
    if(show_job->engineName() == "PoisSolver"){
      showPotPlot();
    }


    qDebug() << tr("Engine Name: %1").arg(show_job->engineName());

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

  // show the average distribution for one of the few presets
  QLabel *label_average_elec_dist = new QLabel("Show average for:");
  QPushButton *button_average_elec_dist_all = new QPushButton(tr("All distributions"));
  QPushButton *button_average_elec_dist_degen = new QPushButton(tr("Degenerate states"));

  button_dist_prev->setShortcut(tr("CTRL+H"));
  button_dist_next->setShortcut(tr("CTRL+L"));

  slider_dist_sel = new QSlider(Qt::Horizontal);
  updateElecDistOptions();

  // choose distribution
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
  dist_vl->addLayout(dist_average_hl);

  dist_group->setLayout(dist_vl);

  // signal connection
  connect(button_show_term_out, &QAbstractButton::clicked,
          this, &gui::SimVisualize::showJobTerminalOutput);
  connect(combo_job_sel, static_cast<void(QComboBox::*)(int)>(
                                              &QComboBox::currentIndexChanged),
          this, &gui::SimVisualize::jobSelUpdate);
  connect(slider_dist_sel, static_cast<void(QSlider::*)(int)>(&QSlider::valueChanged),
          this, &gui::SimVisualize::distSelUpdate);
  connect(button_dist_prev, &QAbstractButton::clicked,
          this, &gui::SimVisualize::distPrev);
  connect(button_dist_next, &QAbstractButton::clicked,
          this, &gui::SimVisualize::distNext);
  connect(button_average_elec_dist_all, &QAbstractButton::clicked,
          this, &gui::SimVisualize::showAverageElecDist);
  connect(button_average_elec_dist_degen, &QAbstractButton::clicked,
          this, &gui::SimVisualize::showAverageElecDistDegen);

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
  if (show_job->elec_dists.isEmpty())
    return;
  int elec_ind = slider_dist_sel->sliderPosition() - 1;
  text_dist_selected->setText(tr("%1/%2").arg(
      slider_dist_sel->value()).arg(show_job->elec_dists.size()));
  showElecDist(elec_ind);

  text_dist_energy->setText(QString::number(show_job->elec_dists[elec_ind].energy));
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
