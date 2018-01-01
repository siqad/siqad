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

  // TODO removal of previous entries

  // header row
  // TODO align columns
  QHBoxLayout *layer_entry_hl = new QHBoxLayout;
  layer_entry_hl->addWidget(new QLabel("Name"));
  layer_entry_hl->addWidget(new QLabel("Z-Height"));
  layer_entry_hl->addWidget(new QLabel("Visible"));
  layer_entry_hl->addWidget(new QLabel("Editable")); // TODO wording
  layer_list_vl->addLayout(layer_entry_hl);

  // add layer info to LayerEditor, 1 row per layer
  for(prim::Layer* layer : *layers) {
    // TODO maybe make this thing a function "addLayerRow"
    QLabel *label_layer_name = new QLabel(layer->getName());
    QLineEdit *le_zheight = new QLineEdit(QString::number(layer->getZHeight()));
    // TODO visibility checkbox
    // TODO editability checkbox

    QHBoxLayout *layer_entry_hl = new QHBoxLayout;
    layer_entry_hl->addWidget(label_layer_name);
    layer_entry_hl->addWidget(le_zheight);

    layer_list_vl->addLayout(layer_entry_hl);
  }

  // TODO apply button to update layer properties, revert button to revert
  // TODO simply reload the list if revert is pushed, I guess

  // TODO UNDOable layer creation in DesignPanel
}

// update widget

// TODO
// z-height of db-surface is 0
// +ve for overhanging layers, -ve for buried
// ability to edit z-height for non-surface layers
// option to "always show layer distance from surface"
// states (checkboxes): visible, editable

}
