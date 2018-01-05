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
  // TODO make into dock widget, or option to make it so

  // grid layout that show all layers
  layer_list_vl = new QVBoxLayout;
  layer_grid_l = new QGridLayout; // this grid is destroyed and recreated everytime editor reloads
  layer_list_vl->addLayout(layer_grid_l);

  // bottom buttons
  QHBoxLayout *bot_buttons_hl = new QHBoxLayout;

  QPushButton *bt_add = new QPushButton(tr("Add")); // TODO implement function
  QPushButton *bt_close = new QPushButton(tr("Close"));

  connect(bt_close, SIGNAL(clicked()), this, SLOT(hide()));

  bt_close->setShortcut(tr("Esc"));

  bot_buttons_hl->addWidget(bt_add);
  bot_buttons_hl->addWidget(bt_close);

  // Main layout
  QVBoxLayout *main_vl = new QVBoxLayout;
  main_vl->addLayout(layer_list_vl);
  main_vl->addLayout(bot_buttons_hl);

  // TODO apply button to update layer properties, revert button to revert
  // TODO simply reload the list if revert is pushed, I guess

  setLayout(main_vl);

  // TODO a struct that contains all graphical objects related to a layer

  // TODO change add/remove to signal based
}

void LayerEditor:: addLayerRow()
{
  // TODO
}

void LayerEditor::updateLayerList()
{
  // TODO layer update signals that trigger this function

  // TODO function to remake layout
  //delete layer_grid_l;
  //layer_grid_l = new QGridLayout;
  // TODO buttons

  // update layer list
    // delete old layer grid and remake
    // populate the new grid with existing layer info
      // TODO sort the layers according to z-height, in descending order


  // header row
  layer_grid_l->addWidget(new QLabel("Name"),0,0);
  layer_grid_l->addWidget(new QLabel("Type"),0,1);
  layer_grid_l->addWidget(new QLabel("Z-Height"),0,2);
  layer_grid_l->addWidget(new QLabel("Visible"),0,3);  // TODO change to icon
  layer_grid_l->addWidget(new QLabel("Editable"),0,4); // TODO change to icon


  // add layer info to LayerEditor, 1 row per layer
  int i = 1;
  for(prim::Layer* layer : *layers) {
    QLabel *label_layer_name = new QLabel(layer->getName());
    QLabel *label_content_type = new QLabel(layer->getContentType()); // TODO change to icon
    QLineEdit *le_zheight = new QLineEdit(QString::number(layer->getZHeight()));
    // TODO meter scale (um,nm,etc)
    QCheckBox *cb_visibility = new QCheckBox(this); // TODO not sure if "this" is needed
    // TODO editability checkbox

    cb_visibility->setChecked(layer->isVisible());

    connect(cb_visibility, SIGNAL(stateChanged(int)), layer, SLOT(visibilityCheckBoxChanged(int)));

    layer_grid_l->addWidget(label_layer_name,i,0);
    layer_grid_l->addWidget(label_content_type,i,1);
    layer_grid_l->addWidget(le_zheight,i,2);
    layer_grid_l->addWidget(cb_visibility,i,3);

    i++;
  }

  // TODO restructure the widget such that the following items are only created
  // during initialization, instead of being remade every time the widget is
  // updated.


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
