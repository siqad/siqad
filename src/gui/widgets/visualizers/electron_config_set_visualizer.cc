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

typedef comp::ChargeConfigSet ECS;
typedef gui::ChargeConfigSetVisualizer ECSVisualizer;

ECSVisualizer::ChargeConfigSetVisualizer(prim::Lattice *lattice, QWidget *parent)
  : QWidget(parent), lattice(lattice)
{
  // config set selection
  l_energy_val = new QLabel();
  l_net_charge_val = new QLabel();
  l_is_valid = new QLabel();
  l_pop_occ = new QLabel();
  l_config_occ = new QLabel();
  l_charge_config_set_ind = new QLabel();
  s_charge_config_list = new QSlider(Qt::Horizontal);
  QPushButton *pb_charge_config_set_left = new QPushButton("<");
  QPushButton *pb_charge_config_set_right = new QPushButton(">");
  QHBoxLayout *hl_charge_config_set_slider = new QHBoxLayout();
  hl_charge_config_set_slider->addWidget(pb_charge_config_set_left);
  hl_charge_config_set_slider->addWidget(s_charge_config_list);
  hl_charge_config_set_slider->addWidget(pb_charge_config_set_right);
  w_config_slider_complex = new QWidget();
  w_config_slider_complex->setLayout(hl_charge_config_set_slider);

  // config config set left and right push buttons
  // no bound checks necessary because QSlider::setValue() itself performs them
  connect(pb_charge_config_set_left, &QPushButton::clicked,
          [this](){s_charge_config_list->setValue(s_charge_config_list->value()-1);});
  connect(pb_charge_config_set_right, &QPushButton::clicked,
          [this](){s_charge_config_list->setValue(s_charge_config_list->value()+1);});

  // respond to slider change
  connect(s_charge_config_list, &QSlider::valueChanged,
          this, &ECSVisualizer::showChargeConfigResultFromSlider);


  // filter
  pb_degenerate_states = new QPushButton("Degenerate states");
  cb_net_charge_filter = new QCheckBox("Filter: all configs");
  cb_phys_valid_filter = new QCheckBox("Only physically valid states");
  s_net_charge_filter = new QSlider(Qt::Horizontal);
  QPushButton *pb_net_charge_filter_left = new QPushButton("<");
  QPushButton *pb_net_charge_filter_right = new QPushButton(">");
  QHBoxLayout *hl_net_charge_filter_slider = new QHBoxLayout();
  hl_net_charge_filter_slider->addWidget(pb_net_charge_filter_left);
  hl_net_charge_filter_slider->addWidget(s_net_charge_filter);
  hl_net_charge_filter_slider->addWidget(pb_net_charge_filter_right);
  w_net_charge_slider_complex = new QWidget();
  w_net_charge_slider_complex->setLayout(hl_net_charge_filter_slider);

  // return the net_charge corresponding to the current charge count filter
  // slider value
  auto netChargeSliderToValue = [this]() -> int
  {
    if (charge_config_set == nullptr)
      return -1;
    int filter_ind = s_net_charge_filter->value();
    return charge_config_set->netCharges().at(filter_ind);
  };

  // return the net charge filter slider position corresponding to the 
  // given net charge
  auto netChargeSliderPositionOfValue = [this](int net_charge) -> int
  {
    if (charge_config_set == nullptr)
      return -1;
    return charge_config_set->netCharges().indexOf(net_charge);
  };

  // update charge count filter state if appropriate
  auto updateNetChargeFilterState = [this, netChargeSliderToValue]()
  {
    if (charge_config_set == nullptr)
      return;
    bool filter_state = cb_net_charge_filter->checkState() == Qt::Checked;
    int net_charge = filter_state ? netChargeSliderToValue() : -1;
    applyNetChargeFilter(false, net_charge);
  };

  // update net charge filter state
  connect(cb_net_charge_filter, &QCheckBox::stateChanged,
          [this, updateNetChargeFilterState, netChargeSliderPositionOfValue]()
          {
            updateGUIFilterStateChange();
            s_net_charge_filter->setValue(
                netChargeSliderPositionOfValue(curr_charge_config.netNegCharge()));
            updateNetChargeFilterState();
          });
  w_net_charge_slider_complex->setEnabled(cb_net_charge_filter->checkState() == Qt::Checked);

  // link net charge filter slider to filter action
  connect(s_net_charge_filter, &QSlider::valueChanged,
          updateNetChargeFilterState);

  // left and right buttons for net charge filter slider
  connect(pb_net_charge_filter_left, &QPushButton::clicked,
          [this](){s_net_charge_filter->setValue(s_net_charge_filter->value()-1);});
  connect(pb_net_charge_filter_right, &QPushButton::clicked,
          [this](){s_net_charge_filter->setValue(s_net_charge_filter->value()+1);});

  // show degenerate states
  connect(pb_degenerate_states, &QPushButton::pressed,
          [this]()
          {
            if (charge_config_set == nullptr || curr_charge_config.config.empty()) 
              return;
            visualizeDegenerateStates(curr_charge_config);
          });

  // physically valid state filter
  connect(cb_phys_valid_filter, &QCheckBox::stateChanged,
          updateNetChargeFilterState);

  // whole layout
  QLabel *help_text = new QLabel("<a href=\"https://siqad.readthedocs.io/en/latest/details/gs-finders.html#interpreting-results\">Interpreting the results</a>");
  help_text->setTextFormat(Qt::RichText);
  help_text->setTextInteractionFlags(Qt::TextBrowserInteraction);
  help_text->setOpenExternalLinks(true);
  QFormLayout *fl_charge_configs = new QFormLayout();
  fl_charge_configs->setLabelAlignment(Qt::AlignLeft);
  fl_charge_configs->addRow(help_text);
  fl_charge_configs->addRow(new QLabel("Config energy (not power cost)"), l_energy_val);
  fl_charge_configs->addRow(new QLabel("Net negative charge"), l_net_charge_val);
  fl_charge_configs->addRow(new QLabel("Physically valid"), l_is_valid);
  fl_charge_configs->addRow(new QLabel("Config occurance"), l_config_occ);
  fl_charge_configs->addRow(new QLabel("Net charge occurance"), l_pop_occ);
  fl_charge_configs->addRow(new QLabel("Config set"), l_charge_config_set_ind);
  fl_charge_configs->addRow(pb_degenerate_states);
  fl_charge_configs->addRow(w_config_slider_complex);
  /*
    NOTE: removed net charge filter for now because it is kind of buggy and 
          probably has to be improved to deal with coexistence of DB+ and DB-
  fl_charge_configs->addRow(cb_net_charge_filter);
  fl_charge_configs->addRow(w_net_charge_slider_complex);
  */
  fl_charge_configs->addRow(cb_phys_valid_filter);
  setLayout(fl_charge_configs);
  show();
}

void ECSVisualizer::clearVisualizer()
{
  setChargeConfigSet(nullptr, false);
}

void ECSVisualizer::setChargeConfigSet(comp::ChargeConfigSet *t_set,
                                         bool show_results_now,
                                         PreferredSelection preferred_sel)
{
  // clean up past results
  clearChargeConfigResult();
  charge_config_list.clear();
  curr_charge_config = ECS::ChargeConfig();

  // set up new results
  charge_config_set = t_set;
  updateGUIConfigSetChange();
  bool phys_valid_filter = cb_phys_valid_filter->isChecked();
  setChargeConfigList(t_set == nullptr ? QList<ECS::ChargeConfig>() : charge_config_set->chargeConfigs(phys_valid_filter));
  if (t_set != nullptr && show_results_now) {
    showChargeConfigResultFromSlider();
    if (preferred_sel == LowestPhysicallyValidState) {
      // select the lowest energy configuration that is physically valid
      int gs_ind = ECS::lowestPhysicallyValidInd(charge_config_list);
      s_charge_config_list->setValue(gs_ind);
    } else if (preferred_sel == LowestInMostPopularNetCharge) {
      // filter to the most popular net charge occurance
      QMap<int, int> net_charge_occ = t_set->netChargeOccurances();
      QMap<int, int>::iterator it, it_max_val=net_charge_occ.begin();
      for (it = net_charge_occ.begin(); it != net_charge_occ.end(); it++) {
        if (it.value() > it_max_val.value()) {
          it_max_val = it;
        }
      }
      applyNetChargeFilter(it_max_val.key());
    }
  }

}

void ECSVisualizer::setChargeConfigList(const QList<comp::ChargeConfigSet::ChargeConfig> &ec)
{
  // cache the current config (curr_charge_config can be affected by GUI update)
  ECS::ChargeConfig curr_config_cache = curr_charge_config;

  // bookkeeping and GUI update
  charge_config_list = ec;
  updateGUIConfigListChange();

  // try to re-select the same charge config as before, if not possible then
  // select the default index
  if (curr_config_cache.config.isEmpty()) {
    return;
  }
  bool found = false;
  for (int i=0; i<charge_config_list.length(); i++) {
    if (ec.at(i) == curr_config_cache) {
      s_charge_config_list->setValue(i);
      found = true;
      break;
    }
  }
  if (!found) {
    s_charge_config_list->setValue(0);
  }

  // force update GUI in case the slider position didn't change from before
  showChargeConfigResultFromSlider();
}

void ECSVisualizer::showChargeConfigResultFromSlider()
{
  int charge_config_ind = s_charge_config_list->value();
  if (charge_config_set == nullptr
      || charge_config_ind < 0
      || charge_config_ind >= charge_config_list.length()) {
    qCritical() << tr("Charge config set slider value out of bound");
    return;
  }
  showChargeConfigResult(charge_config_list.at(charge_config_ind),
      charge_config_set->dbPhysicalLocations());
  updateGUIConfigSelectionChange(charge_config_ind);
}

void ECSVisualizer::showChargeConfigResult(const ECS::ChargeConfig &charge_config,
                                             const QList<QPointF> &db_phys_locs,
                                             const QList<float> &db_fill)
{
  // clean up previous result
  clearChargeConfigResult();

  curr_charge_config = charge_config;

  // set the charge fill state of the provided set of DBs
  showing_db_sites = lattice->dbsAtPhysLocs(db_phys_locs);
  if (db_phys_locs.size() != 0 && showing_db_sites.empty()) {
    qCritical() << tr("Failed to retrieve all DB locations, aborting charge \
        config result display.");
    return;
  }
  for (int i=0; i<curr_charge_config.config.length(); i++) {
    float t_fill = db_fill.empty() ? charge_config.config.at(i) : db_fill.at(i);
    showing_db_sites.at(i)->setShowElec(t_fill);
    /*
    qDebug() << tr("DB site (%1, %2) set to %3 filled")
      .arg(showing_db_sites.at(i)->x())
      .arg(showing_db_sites.at(i)->y())
      .arg(charge_config.config.at(i));
      */
  }
}

void ECSVisualizer::visualizeDegenerateStates(const ECS::ChargeConfig &charge_config)
{
  QList<ECS::ChargeConfig> degen_configs = charge_config_set->degenerateConfigs(charge_config);
  QList<float> db_fill;

  // add all of the degen configs
  bool init=true;
  for (ECS::ChargeConfig t_config : degen_configs) {
    for (int i=0; i<t_config.config.size(); i++) {
      if (init)
        db_fill.append(t_config.config.at(i));
      else 
        db_fill[i] += t_config.config.at(i);
    }
    init=false;
  }

  // divide the degen config count and sqrt
  for (int i=0; i<db_fill.size(); i++) {
    db_fill[i] = pow((db_fill[i] / degen_configs.size()), 2);
  }

  showChargeConfigResult(curr_charge_config, 
                           charge_config_set->dbPhysicalLocations(),
                           db_fill);
}

void ECSVisualizer::applyNetChargeFilter(const bool &use_slider, const int &net_charge)
{
  bool phys_valid_filter = cb_phys_valid_filter->isChecked();
  if (!use_slider) {
    setChargeConfigList(charge_config_set->chargeConfigs(phys_valid_filter, true, net_charge));
    s_net_charge_filter->setValue(charge_config_set->netCharges().indexOf(net_charge));
  } else {
    setChargeConfigList(charge_config_set->chargeConfigs(phys_valid_filter));
  }
  updateGUIFilterSelectionChange(net_charge);
}

void ECSVisualizer::clearChargeConfigResult()
{
  // TODO clear simulation result related flags

  for (prim::DBDot *db : showing_db_sites)
    db->setShowElec(0);

  showing_db_sites.clear();
}

QWidget *ECSVisualizer::scatterPlotChargeConfigSet()
{
  using namespace QtCharts;
  QScatterSeries *series = new QScatterSeries();
  series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
  series->setMarkerSize(15.0);

  if (charge_config_set != nullptr) {
    for (comp::ChargeConfigSet::ChargeConfig config : charge_config_set->chargeConfigs()) {
      series->append(config.netNegCharge(), config.energy);
    }
  } else {
    qCritical() << tr("No charge config set selected/available.");
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
  // enable or disable GUI elements depending on whether the charge_config_set
  // is a nullptr or not
  bool enable = charge_config_set != nullptr;
  w_config_slider_complex->setEnabled(enable);
  w_net_charge_slider_complex->setEnabled(enable);
  cb_net_charge_filter->setEnabled(enable);
  pb_degenerate_states->setEnabled(enable);
  /*
  s_charge_config_list->setEnabled(enable);
  s_net_charge_filter->setEnabled(enable);
  cb_net_charge_filter->setEnabled(enable);
  */

  if (charge_config_set != nullptr) {
    // update filter slider if a config is set
    s_net_charge_filter->setMinimum(0);
    s_net_charge_filter->setMaximum(charge_config_set->netCharges().length()-1);
  } else {
    // clear information field values if no config set
    l_energy_val->setText("");
    l_net_charge_val->setText("");
    l_is_valid->setText("");
    l_pop_occ->setText("");
    l_config_occ->setText("");
    cb_net_charge_filter->setChecked(false);
    cb_net_charge_filter->setText("Filter: all configs");
    cb_phys_valid_filter->setChecked(false);
  }
}

void ECSVisualizer::updateGUIConfigListChange()
{
  if (charge_config_set == nullptr)
    return;

  if (charge_config_list.isEmpty()) {
    // the list is empty, no sliding on the sliders
    s_charge_config_list->setMaximum(0);
    s_net_charge_filter->setMaximum(0);
    return;
  }

  // update config selection slider
  s_charge_config_list->setMinimum(0);
  s_charge_config_list->setMaximum(charge_config_list.length()-1);
}

void ECSVisualizer::updateGUIConfigSelectionChange(const int &charge_config_list_ind)
{
  // information
  int config_occ = curr_charge_config.config_occ;
  int pop_occ = charge_config_set->netChargeOccurances().value(curr_charge_config.netNegCharge());
  int total_occ = charge_config_set->totalConfigCount();

  l_energy_val->setText(tr("%1 eV").arg(curr_charge_config.energy));
  l_net_charge_val->setText(QString::number(curr_charge_config.netNegCharge()));
  if (curr_charge_config.is_valid == 0)
    l_is_valid->setText("No");
  else if (curr_charge_config.is_valid == 1)
    l_is_valid->setText("Yes");
  else
    l_is_valid->setText("Unknown");
  l_pop_occ->setText(tr("%1 (%2\%)").arg(pop_occ).arg((float)pop_occ/total_occ*100));
  l_config_occ->setText(tr("%1 (%2\%)").arg(config_occ).arg((float)config_occ/total_occ*100));

  // slider
  int max_ind = (charge_config_set != nullptr) ? charge_config_list.length() : 0;
  l_charge_config_set_ind->setText(tr("%1 / %2").arg(charge_config_list_ind + 1).arg(max_ind));
}

void ECSVisualizer::updateGUIFilterStateChange()
{
  w_net_charge_slider_complex->setEnabled(cb_net_charge_filter->isChecked());
}

void ECSVisualizer::updateGUIFilterSelectionChange(const int &net_charge)
{
  cb_net_charge_filter->setChecked(net_charge > -1);
  if (net_charge > -1) {
    cb_net_charge_filter->setText(tr("Filter: %1 net charge").arg(net_charge));
  } else {
    cb_net_charge_filter->setText("Filter: all configs");
  }
}

/*
void ECSVisualizer::updateChargeConfigSetSelectionGUI()
{
  if (charge_config_list.isEmpty()) {
    // the set is empty, reset all elements.
    s_charge_config_list->setMaximum(0);
    s_net_charge_filter->setMaximum(0);
    return;
  }

  // update selection slider
  s_charge_config_list->setMinimum(0);
  s_charge_config_list->setMaximum(charge_config_list.length()-1);

  // update filter ranges
  int default_filter_ind = 0; // TODO default to most popular population
  s_net_charge_filter->setMinimum(default_filter_ind);
  s_net_charge_filter->setMaximum(charge_config_set->netCharges().length()-1);
  updateElectronCountFilterLabel(default_filter_ind);

  // create a scatter plot of energy vs electron count
  //scatterPlotChargeConfigSet();
}
*/

/*
void ECSVisualizer::updateChargeConfigSetIndexLabel(const int &charge_config_list_ind)
{
  int max_ind = (charge_config_set != nullptr) ? charge_config_list.length() : 0;
  l_charge_config_set_ind->setText(tr("%1 / %2").arg(charge_config_list_ind + 1).arg(max_ind));
}
*/

/*
void ECSVisualizer::updateElectronCountFilterLabel(const int &elec_count)
{
  if (elec_count > -1) {
    cb_net_charge_filter->setText(tr("Filter: %1 electrons").arg(elec_count));
  } else {
    cb_net_charge_filter->setText("Filter: all configs");
  }
}
*/
