// @file:     potential_landscape_visualizer.cc
// @author:   Samuel
// @created:  2019.03.22
// @license:  GNU LGPL v3
//
// @desc:     Widgets for visualizing potential landscapes.

#include "potential_landscape_visualizer.h"

using namespace gui;

typedef comp::PotentialLandscape PL;
typedef gui::PotentialLandscapeVisualizer PLVisualizer;

PLVisualizer::PotentialLandscapeVisualizer(DesignPanel *design_pan, QWidget *parent)
  : QWidget(parent), design_pan(design_pan)
{
  // TODO initilize GUI
  QFormLayout *fl_pot_landscape = new QFormLayout();
  l_has_static_plot = new QLabel();
  l_has_animation = new QLabel();
  fl_pot_landscape->addRow(new QLabel("Has static plot"), l_has_static_plot);
  fl_pot_landscape->addRow(new QLabel("Has animation"), l_has_animation);
  fl_pot_landscape->setLabelAlignment(Qt::AlignLeft);
  setLayout(fl_pot_landscape);
  // TODO check which images/animations are available
  // TODO for animations, let user choose to show step by step or animation in GUI
}

void PLVisualizer::clearVisualizer()
{
  clearPotentialResultOverlay();
  pot_landscape = nullptr;
}

void PLVisualizer::setPotentialLandscape(PL *t_pot_landscape)
{
  clearPotentialResultOverlay();
  pot_landscape = t_pot_landscape;

  if (t_pot_landscape == nullptr)
    return;

  l_has_static_plot->setText(pot_landscape->staticPlotPath().isEmpty() ? "No" : "Yes");
  l_has_animation->setText(pot_landscape->animationPath().isEmpty() ? "No" : "Yes");
  showPotentialResultOverlay();
}

void PLVisualizer::showPotentialResultOverlay()
{
  // future implementation:
  // TODO what to show should be specified through input variables
  // TODO add them to GUI
  // TODO move the functions in Design Panel over here
  // TODO potplot creation and removal probably don't need to be undoable

  // current re-implementation of Nathan's code:
  clearPotentialResultOverlay();  // clean up existing results
  QVector<float> pot_vec_top_left = pot_landscape->potentials().front();
  QVector<float> pot_vec_bot_right = pot_landscape->potentials().back();
  QPointF p_top_left(pot_vec_top_left[0], pot_vec_top_left[1]);
  QPointF p_bot_right(pot_vec_bot_right[0], pot_vec_bot_right[1]);
  p_top_left *= prim::Item::scale_factor;
  p_bot_right *= prim::Item::scale_factor;

  qDebug() << tr("Static path: %1").arg(pot_landscape->staticPlotPath());

  QRectF graph_container(p_top_left, p_bot_right);
  design_pan->displayPotentialPlot(pot_landscape->staticPlotPath(),
                                   graph_container,
                                   pot_landscape->animationPath());
}

void PLVisualizer::clearPotentialResultOverlay()
{
  // TODO move design panel control over the plots to this visualizer
  // remove potplots from scene and clean up
  design_pan->clearPlots();
}
