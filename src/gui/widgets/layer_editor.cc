// @file:     layer_editor.cc
// @author:   Samuel
// @created:  2017.12.22
// @editted:  2017.12.22 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     widget for configuring 

#include "layer_editor.h"

namespace gui {

// constructor
LayerEditor::LayerEditor(gui::DesignPanel *design_pan, QWidget *parent)
  : QWidget(parent, Qt::Dialog), dp(design_pan)
{
  layers = dp->getLayers();
  initLayerEditor();
}


// PRIVATE

// initialize widget
void LayerEditor::initLayerEditor()
{
  // populate widget with existing layers

  // TEMP just show all the layers with no order for a start
  l_layers.clear();
  for(prim::Layer* layer : *layers) {
    l_layers.append(new QLabel(layer->getName()));
  }

  layer_list_vl = new QVBoxLayout; // TODO make into class pointer if needed

  for(QLabel *l_layer : l_layers)
    layer_list_vl->addWidget(l_layer);

  setLayout(layer_list_vl);


  // TODO a struct that contains all graphical objects related to a layer

  // TODO signal for adding/removing layers, LayerEditor receives signals and updates everytime
}

void LayerEditor::updateLayerList()
{
  qDebug() << "entered update layer list";

  l_layers.clear();
  qDebug() << "cleared l_layers";

  for(prim::Layer* layer : *layers) {
    l_layers.append(new QLabel(layer->getName()));
  }
  qDebug() << "added labels of layers";

  // TODO removal of layer labels from VL

  //layer_list_vl->clear();
  for(QLabel *l_layer : l_layers){
    layer_list_vl->addWidget(l_layer);
  }
  qDebug() << "added layers to vl";
}

// update widget

// TODO
// z-height of db-surface is 0
// +ve for overhanging layers, -ve for buried
// ability to edit z-height for non-surface layers
// option to "always show layer distance from surface"
// states (checkboxes): visible, editable

}
