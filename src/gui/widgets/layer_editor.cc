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

  layer_list_vl = new QGridLayout; // TODO make into class pointer if needed

  setLayout(layer_list_vl);


  // TODO a struct that contains all graphical objects related to a layer

  // TODO change add/remove to signal based
}

void LayerEditor::updateLayerList()
{
  qDebug() << "entered update layer list";

  // TODO better way of updating, like using signals
  delete layer_list_vl;
  layer_list_vl = new QGridLayout;
  setLayout(layer_list_vl);

  // header row
  layer_list_vl->addWidget(new QLabel("Name"),0,0);
  layer_list_vl->addWidget(new QLabel("Z-Height"),0,1);
  layer_list_vl->addWidget(new QLabel("Visible"),0,2);
  layer_list_vl->addWidget(new QLabel("Editable"),0,3); // TODO wording

  // add layer info to LayerEditor, 1 row per layer
  int i = 1;
  for(prim::Layer* layer : *layers) {
    // TODO maybe make this thing a function "addLayerRow"
    QLabel *label_layer_name = new QLabel(layer->getName());
    QLineEdit *le_zheight = new QLineEdit(QString::number(layer->getZHeight()));
    // TODO visibility checkbox
    // TODO editability checkbox
    // TODO use icons instead of checkboxes for V and E

    layer_list_vl->addWidget(label_layer_name,i,0);
    layer_list_vl->addWidget(le_zheight,i,1);

    i++;
  }

  // TODO apply button to update layer properties, revert button to revert
  // TODO simply reload the list if revert is pushed, I guess
  // TODO close button

  // TODO UNDOable z-height change
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
