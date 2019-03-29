// @file:     electron_config_set_visualizer.cc
// @author:   Samuel
// @created:  2019.03.20
// @license:  GNU LGPL v3
//
// @desc:     Widgets for visualizing electron config sets.

#include <QtCharts/QChartView>
#include <QtCharts/QScatterSeries>

#include "electron_config_set_visualizer.h"

using namespace gui;

typedef comp::ElectronConfigSet ECS;
typedef gui::ElectronConfigSetVisualizer ECSVisualizer;

ECSVisualizer::ElectronConfigSetVisualizer(DesignPanel *design_pan, QWidget *parent)
  : QWidget(parent), design_pan(design_pan)
{
  // config set selection
  l_energy_val = new QLabel();
  l_elec_count_val = new QLabel();
  l_elec_config_set_ind = new QLabel();
  s_elec_config_list = new QSlider(Qt::Horizontal);
  QPushButton *pb_elec_config_set_left = new QPushButton("<");
  QPushButton *pb_elec_config_set_right = new QPushButton(">");
  QHBoxLayout *hl_elec_config_set_slider = new QHBoxLayout();
  hl_elec_config_set_slider->addWidget(pb_elec_config_set_left);
  hl_elec_config_set_slider->addWidget(s_elec_config_list);
  hl_elec_config_set_slider->addWidget(pb_elec_config_set_right);
  w_config_slider_complex = new QWidget();
  w_config_slider_complex->setLayout(hl_elec_config_set_slider);

  // electron config set left and right push buttons
  // no bound checks necessary because QSlider::setValue() itself performs them
  connect(pb_elec_config_set_left, &QPushButton::clicked,
          [this](){s_elec_config_list->setValue(s_elec_config_list->value()-1);});
  connect(pb_elec_config_set_right, &QPushButton::clicked,
          [this](){s_elec_config_list->setValue(s_elec_config_list->value()+1);});

  // respond to slider change
  connect(s_elec_config_list, &QSlider::valueChanged,
          this, &ECSVisualizer::showElectronConfigResultFromSlider);


  // filter
  cb_elec_count_filter = new QCheckBox("Filter: all configs");
  s_elec_count_filter = new QSlider(Qt::Horizontal);
  QPushButton *pb_elec_count_filter_left = new QPushButton("<");
  QPushButton *pb_elec_count_filter_right = new QPushButton(">");
  QHBoxLayout *hl_elec_count_filter_slider = new QHBoxLayout();
  hl_elec_count_filter_slider->addWidget(pb_elec_count_filter_left);
  hl_elec_count_filter_slider->addWidget(s_elec_count_filter);
  hl_elec_count_filter_slider->addWidget(pb_elec_count_filter_right);
  w_elec_count_slider_complex = new QWidget();
  w_elec_count_slider_complex->setLayout(hl_elec_count_filter_slider);

  // return the elec_count corresponding to the current electron count filter
  // slider value
  auto electronCountSliderToValue = [this]() -> int
  {
    if (elec_config_set == nullptr)
      return -1;
    int filter_ind = s_elec_count_filter->value();
    return elec_config_set->electronCounts().at(filter_ind);
  };

  // return the electron counter filter slider position corresponding to the 
  // given electron count
  auto electronCountSliderPositionOfValue = [this](int elec_count) -> int
  {
    if (elec_config_set == nullptr)
      return -1;
    return elec_config_set->electronCounts().indexOf(elec_count);
  };

  // update electron count filter state if appropriate
  auto updateElectronCountFilterState = [this, electronCountSliderToValue]()
  {
    if (elec_config_set == nullptr)
      return;
    bool filter_state = cb_elec_count_filter->checkState() == Qt::Checked;
    int elec_count = filter_state ? electronCountSliderToValue() : -1;
    applyElectronCountFilter(elec_count);
  };

  // update electron filter state
  connect(cb_elec_count_filter, &QCheckBox::stateChanged,
          [this, updateElectronCountFilterState, electronCountSliderPositionOfValue]()
          {
            updateGUIFilterStateChange();
            s_elec_count_filter->setValue(
                electronCountSliderPositionOfValue(curr_elec_config.elec_count));
            updateElectronCountFilterState();
          });
  w_elec_count_slider_complex->setEnabled(cb_elec_count_filter->checkState() == Qt::Checked);

  // link electron count filter slider to filter action
  connect(s_elec_count_filter, &QSlider::valueChanged,
          updateElectronCountFilterState);

  // left and right buttons for electron count filter slider
  connect(pb_elec_count_filter_left, &QPushButton::clicked,
          [this](){s_elec_count_filter->setValue(s_elec_count_filter->value()-1);});
  connect(pb_elec_count_filter_right, &QPushButton::clicked,
          [this](){s_elec_count_filter->setValue(s_elec_count_filter->value()+1);});


  // whole layout
  QFormLayout *fl_elec_configs = new QFormLayout();
  fl_elec_configs->setLabelAlignment(Qt::AlignLeft);
  fl_elec_configs->addRow(new QLabel("Energy"), l_energy_val);
  fl_elec_configs->addRow(new QLabel("Electron count"), l_elec_count_val);
  fl_elec_configs->addRow(new QLabel("Config set"), l_elec_config_set_ind);
  fl_elec_configs->addRow(w_config_slider_complex);
  fl_elec_configs->addRow(cb_elec_count_filter);
  fl_elec_configs->addRow(w_elec_count_slider_complex);
  setLayout(fl_elec_configs);
  show();
}

void ECSVisualizer::clearVisualizer()
{
  setElectronConfigSet(nullptr, false, false);
}

void ECSVisualizer::setElectronConfigSet(comp::ElectronConfigSet *t_set,
                                         bool most_popular_elec_count,
                                         bool show_results_now)
{
  // clean up past results
  clearElectronConfigResult();
  elec_config_list.clear();
  curr_elec_config = ECS::ElectronConfig();

  // set up new results
  elec_config_set = t_set;
  updateGUIConfigSetChange();
  setElectronConfigList(t_set == nullptr ? QList<ECS::ElectronConfig>() : elec_config_set->electronConfigs());
  if (t_set != nullptr && show_results_now) {
    showElectronConfigResultFromSlider();
    if (most_popular_elec_count) {
      // filter to the most popular electron count
      QMap<int, int> elec_count_occ = t_set->electronCountOccurances();
      QMap<int, int>::iterator it, it_max_val=elec_count_occ.begin();
      for (it = elec_count_occ.begin(); it != elec_count_occ.end(); it++) {
        if (it.value() > it_max_val.value()) {
          it_max_val = it;
        }
      }
      applyElectronCountFilter(it_max_val.key());
    }
  }

}

void ECSVisualizer::setElectronConfigList(const QList<comp::ElectronConfigSet::ElectronConfig> &ec)
{
  // cache the current config (curr_elec_config can be affected by GUI update)
  ECS::ElectronConfig curr_config_cache = curr_elec_config;

  // bookkeeping and GUI update
  elec_config_list = ec;
  updateGUIConfigListChange();

  // try to re-select the same electron config as before, if not possible then
  // select the default index
  if (curr_config_cache.config.isEmpty()) {
    return;
  }
  bool found = false;
  for (int i=0; i<elec_config_list.length(); i++) {
    if (ec.at(i) == curr_config_cache) {
      s_elec_config_list->setValue(i);
      found = true;
      break;
    }
  }
  if (!found) {
    s_elec_config_list->setValue(0);
  }

  // force update GUI in case the slider position didn't change from before
  showElectronConfigResultFromSlider();
}

void ECSVisualizer::showElectronConfigResultFromSlider()
{
  int elec_config_ind = s_elec_config_list->value();
  if (elec_config_set == nullptr
      || elec_config_ind < 0
      || elec_config_ind >= elec_config_list.length()) {
    qCritical() << tr("Electron config set slider value out of bound");
    return;
  }
  showElectronConfigResult(elec_config_list.at(elec_config_ind),
      elec_config_set->dbPhysicalLocations());
  updateGUIConfigSelectionChange(elec_config_ind);
}

void ECSVisualizer::showElectronConfigResult(const ECS::ElectronConfig &elec_config,
                                             const QList<QPointF> &db_phys_locs)
{
  // clean up previous result
  clearElectronConfigResult();

  curr_elec_config = elec_config;

  // design panel display mode
  design_pan->setDisplayMode(DisplayMode::SimDisplayMode);

  // set the charge fill state of the provided set of DBs
  showing_db_sites = design_pan->getDBsAtLocs(db_phys_locs);
  for (int i=0; i<curr_elec_config.config.length(); i++) {
    showing_db_sites.at(i)->setShowElec(elec_config.config.at(i));
    /*
    qDebug() << tr("DB site (%1, %2) set to %3 filled")
      .arg(showing_db_sites.at(i)->x())
      .arg(showing_db_sites.at(i)->y())
      .arg(elec_config.config.at(i));
      */
  }
}

void ECSVisualizer::applyElectronCountFilter(const int &elec_count)
{
  if (elec_count >= 0) {
    setElectronConfigList(elec_config_set->electronConfigs(elec_count));
    s_elec_count_filter->setValue(elec_config_set->electronCounts().indexOf(elec_count));
  } else {
    setElectronConfigList(elec_config_set->electronConfigs());
  }
  updateGUIFilterSelectionChange(elec_count);
}

void ECSVisualizer::clearElectronConfigResult()
{
  // TODO clear simulation result related flags

  for (prim::DBDot *db : showing_db_sites)
    db->setShowElec(0);

  showing_db_sites.clear();
}

QWidget *ECSVisualizer::scatterPlotElectronConfigSet()
{
  using namespace QtCharts;
  QScatterSeries *series = new QScatterSeries();
  series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
  series->setMarkerSize(15.0);

  if (elec_config_set != nullptr) {
    for (comp::ElectronConfigSet::ElectronConfig config : elec_config_set->electronConfigs()) {
      series->append(config.elec_count, config.energy);
    }
  } else {
    qCritical() << tr("No electron config set selected/available.");
  }

  QChartView *chart_view = new QChartView();
  chart_view->chart()->addSeries(series);
  chart_view->chart()->createDefaultAxes();
  chart_view->chart()->zoomOut();
  chart_view->show();

  return chart_view;
}


// PRIVATE

void ECSVisualizer::updateGUIConfigSetChange()
{
  // enable or disable GUI elements depending on whether the elec_config_set
  // is a nullptr or not
  bool enable = elec_config_set != nullptr;
  w_config_slider_complex->setEnabled(enable);
  w_elec_count_slider_complex->setEnabled(enable);
  cb_elec_count_filter->setEnabled(enable);
  /*
  s_elec_config_list->setEnabled(enable);
  s_elec_count_filter->setEnabled(enable);
  cb_elec_count_filter->setEnabled(enable);
  */

  if (elec_config_set != nullptr) {
    // update filter slider if a config is set
    s_elec_count_filter->setMinimum(0);
    s_elec_count_filter->setMaximum(elec_config_set->electronCounts().length()-1);
  } else {
    // clear information field values if no config set
    l_energy_val->setText("");
    l_elec_count_val->setText("");
    cb_elec_count_filter->setChecked(false);
    cb_elec_count_filter->setText("Filter: all configs");
  }
}

void ECSVisualizer::updateGUIConfigListChange()
{
  if (elec_config_set == nullptr)
    return;

  if (elec_config_list.isEmpty()) {
    // the list is empty, no sliding on the sliders
    s_elec_config_list->setMaximum(0);
    s_elec_count_filter->setMaximum(0);
    return;
  }

  // update config selection slider
  s_elec_config_list->setMinimum(0);
  s_elec_config_list->setMaximum(elec_config_list.length()-1);
}

void ECSVisualizer::updateGUIConfigSelectionChange(const int &elec_config_list_ind)
{
  // information
  l_energy_val->setText(tr("%1 eV").arg(curr_elec_config.energy));
  l_elec_count_val->setText(QString::number(curr_elec_config.elec_count));

  // slider
  int max_ind = (elec_config_set != nullptr) ? elec_config_list.length() : 0;
  l_elec_config_set_ind->setText(tr("%1 / %2").arg(elec_config_list_ind + 1).arg(max_ind));
}

void ECSVisualizer::updateGUIFilterStateChange()
{
  w_elec_count_slider_complex->setEnabled(cb_elec_count_filter->isChecked());
}

void ECSVisualizer::updateGUIFilterSelectionChange(const int &elec_count)
{
  cb_elec_count_filter->setChecked(elec_count > -1);
  if (elec_count > -1) {
    cb_elec_count_filter->setText(tr("Filter: %1 electrons").arg(elec_count));
  } else {
    cb_elec_count_filter->setText("Filter: all configs");
  }
}

/*
void ECSVisualizer::updateElectronConfigSetSelectionGUI()
{
  if (elec_config_list.isEmpty()) {
    // the set is empty, reset all elements.
    s_elec_config_list->setMaximum(0);
    s_elec_count_filter->setMaximum(0);
    return;
  }

  // update selection slider
  s_elec_config_list->setMinimum(0);
  s_elec_config_list->setMaximum(elec_config_list.length()-1);

  // update filter ranges
  int default_filter_ind = 0; // TODO default to most popular population
  s_elec_count_filter->setMinimum(default_filter_ind);
  s_elec_count_filter->setMaximum(elec_config_set->electronCounts().length()-1);
  updateElectronCountFilterLabel(default_filter_ind);

  // create a scatter plot of energy vs electron count
  //scatterPlotElectronConfigSet();
}
*/

/*
void ECSVisualizer::updateElectronConfigSetIndexLabel(const int &elec_config_list_ind)
{
  int max_ind = (elec_config_set != nullptr) ? elec_config_list.length() : 0;
  l_elec_config_set_ind->setText(tr("%1 / %2").arg(elec_config_list_ind + 1).arg(max_ind));
}
*/

/*
void ECSVisualizer::updateElectronCountFilterLabel(const int &elec_count)
{
  if (elec_count > -1) {
    cb_elec_count_filter->setText(tr("Filter: %1 electrons").arg(elec_count));
  } else {
    cb_elec_count_filter->setText("Filter: all configs");
  }
}
*/
