// @file:     sim_visualize_panel.h
// @author:   Samuel
// @created:  2017.10.03
// @editted:  2017.10.03 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     SimVisualize classes

#include "sim_visualize_panel.h"
#include "../../qcustomplot.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <QPixmap>


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

void SimVisualize::openPoisResult()
{
  QWidget *window = new QWidget;
  window->resize(750, 750);
  QHBoxLayout *layout = new QHBoxLayout;
  QCustomPlot *customPlot = new QCustomPlot();

  // set up the QCPColorMap:
  QCPColorMap *colorMap = new QCPColorMap(customPlot->xAxis, customPlot->yAxis);
  // number of points in x and y direction
  int nx = 50;
  int ny = 50;
  colorMap->data()->setSize(nx, ny);
  // coordinate range for x and y
  QString path = "/tmp/db-sim/phys/" + combo_job_sel->currentText();
  path += "/sim_result.xml";

  qDebug() << tr("%1").arg(path);

  boost::property_tree::ptree tree; // Create empty property tree object
  boost::property_tree::read_xml(path.toStdString(), tree); // Parse the XML into the property tree.

  QVector<qreal> x_vec;
  QVector<qreal> y_vec;
  QVector<qreal> val_vec;
  double x, y, val;
  for (boost::property_tree::ptree::value_type const &node_potential_map : tree.get_child("sim_out.potential_map")) {
  // BOOST_FOREACH(boost::property_tree::ptree::value_type &node_potential_map, tree.get_child("sim_out.potential_map")) {
    boost::property_tree::ptree subtree = node_potential_map.second; //get subtree with layer items at the top
    if( node_potential_map.first == "potential_val"){ //go into potential_val.
      x = node_potential_map.second.get<float>("<xmlattr>.x");
      y = node_potential_map.second.get<float>("<xmlattr>.y");
      val = node_potential_map.second.get<float>("<xmlattr>.val");
      x_vec.append(x);
      y_vec.append(y);
      val_vec.append(val);
    }
  }

  // set range for x and y in pixels.
  colorMap->data()->setRange(QCPRange(x_vec.first(), x_vec.last()), QCPRange(y_vec.first(), y_vec.last()));

  // now we assign some data, by accessing the QCPColorMapData instance of the color map:
  int x_ind, y_ind;
  for (int i=0; i<nx; ++i){
    for (int j=0; j<ny; ++j){
      // get corresponding cell index from coordinate
      colorMap->data()->coordToCell(x_vec[i*nx + j], y_vec[i*nx + j], &x_ind, &y_ind);
      colorMap->data()->setCell(x_ind, y_ind, val_vec[i*nx + j]);
    }
  }

  // configure axis rect:
  customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom); // this will also allow rescaling the color scale by dragging/zooming
  customPlot->axisRect()->setupFullAxesBox(true);
  customPlot->xAxis->setLabel("x");
  customPlot->yAxis->setLabel("y");
  // down on graph is increase in y.
  customPlot->yAxis->setRangeReversed(true);



  // add a color scale:
  QCPColorScale *colorScale = new QCPColorScale(customPlot);
  customPlot->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
  colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
  colorMap->setColorScale(colorScale); // associate the color map with the color scale
  colorScale->axis()->setLabel("Magnetic Field Strength");

  // set the color gradient of the color map to one of the presets:
  colorMap->setGradient(QCPColorGradient::gpPolar);

  // rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
  colorMap->rescaleDataRange();

  // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
  QCPMarginGroup *marginGroup = new QCPMarginGroup(customPlot);
  customPlot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
  colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

  // rescale the key (x) and value (y) axes so the whole color map is visible:
  customPlot->rescaleAxes();
  QPixmap potential_plot = customPlot->toPixmap();

  layout->addWidget(customPlot); //customPlot doubles as a widget
  window->setLayout(layout);
  window->show();

  // connect(this, SIGNAL(customPlot->mouseMove(QMouseEvent*)), this,SLOT(QCustomPlot::showPointToolTip(QMouseEvent*)));

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
  // TODO emit signal telling design_panel to show dist_ind of show_job->elec_dists
  emit showElecDistOnScene(show_job, dist_ind);
  return true;
}


void SimVisualize::updateElecDistOptions()
{
  // slider
  if(!text_dist_selected || !slider_dist_sel || !show_job)
    return;

  if(!show_job->isComplete() || show_job->elec_dists.isEmpty()){
    slider_dist_sel->setMinimum(0);
    slider_dist_sel->setMaximum(0);
    text_dist_selected->setText("0/0");
  }
  else{
    int dist_count = show_job->elec_dists.size();
    int min_sel = dist_count > 0;
    slider_dist_sel->setMinimum(min_sel);
    slider_dist_sel->setMaximum(dist_count);
    slider_dist_sel->setValue(min_sel);
    text_dist_selected->setText(tr("%1/%2").arg(min_sel).arg(dist_count));
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
    updateElecDistOptions();

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
  button_open_window = new QPushButton("Open Window");

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
  job_info_layout->addWidget(button_open_window);
  job_info_group->setLayout(job_info_layout);

  // Elec Distribution Group
  QGroupBox *dist_group = new QGroupBox(tr("Electron Distribution"));
  QLabel *label_dist_sel = new QLabel(tr("Dist:"));
  QPushButton *button_dist_prev = new QPushButton(tr("<"));
  QPushButton *button_dist_next = new QPushButton(tr(">"));
  text_dist_selected = new QLabel("0/0");

  button_dist_prev->setShortcut(tr("CTRL+H"));
  button_dist_next->setShortcut(tr("CTRL+L"));

  slider_dist_sel = new QSlider(Qt::Horizontal);
  updateElecDistOptions();

  QHBoxLayout *dist_sel_hl = new QHBoxLayout;
  dist_sel_hl->addWidget(label_dist_sel);
  dist_sel_hl->addWidget(slider_dist_sel);

  QHBoxLayout *dist_sel_buttons_hl = new QHBoxLayout;
  dist_sel_buttons_hl->addWidget(button_dist_prev);
  dist_sel_buttons_hl->addWidget(text_dist_selected);
  dist_sel_buttons_hl->addWidget(button_dist_next);

  QVBoxLayout *dist_vl = new QVBoxLayout;
  dist_vl->addLayout(dist_sel_hl);
  dist_vl->addLayout(dist_sel_buttons_hl);

  dist_group->setLayout(dist_vl);

  // signal connection
  connect(button_show_term_out, &QAbstractButton::clicked, this, &gui::SimVisualize::showJobTerminalOutput);
  connect(button_open_window, &QAbstractButton::clicked, this, &gui::SimVisualize::openPoisResult);
  connect(combo_job_sel, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &gui::SimVisualize::jobSelUpdate);
  connect(slider_dist_sel, static_cast<void(QSlider::*)(int)>(&QSlider::valueChanged), this, &gui::SimVisualize::distSelUpdate);
  connect(button_dist_prev, &QAbstractButton::clicked, this, &gui::SimVisualize::distPrev);
  connect(button_dist_next, &QAbstractButton::clicked, this, &gui::SimVisualize::distNext);

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
  text_dist_selected->setText(tr("%1/%2").arg(slider_dist_sel->value()).arg(show_job->elec_dists.size()));
  showElecDist(slider_dist_sel->sliderPosition());
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
  if(!slider_dist_sel)
    return;

  if(slider_dist_sel->value() != slider_dist_sel->maximum())
    slider_dist_sel->setValue(slider_dist_sel->value() + 1);
}


} // end gui namespace
