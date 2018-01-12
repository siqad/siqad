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
  // top buttons
  QPushButton *bt_add = new QPushButton(tr("Add")); // TODO implement function

  QHBoxLayout *top_buttons_hl = new QHBoxLayout;
  top_buttons_hl->addWidget(bt_add);

  // grid layout that show all layers
  layer_table = new QTableWidget(this);

  // Main layout
  QVBoxLayout *main_vl = new QVBoxLayout;
  main_vl->addLayout(top_buttons_hl);
  main_vl->addWidget(layer_table);

  setLayout(main_vl);

  // TODO change add/remove to signal based
}


void LayerEditor::initLayerTable()
{
  QStringList table_headers;
  table_headers << 
    "Layer ID" << // Layer ID, hidden
    "Name" <<     // Name
    "Type" <<     // Type (lattice, db, electrode)
    "Z-Height" << // Z-Height (vertical offset from surface)
    "" <<  // Visibility
    "";   // Editability

  layer_table->setColumnCount(table_headers.count());
  // TODO don't hard code column ID
  layer_table->setColumnHidden(0, true); // hide Layer ID
  layer_table->resizeColumnToContents(4); // reduce width of visibility column
  layer_table->resizeColumnToContents(5); // reduce width of visibility column
  layer_table->setHorizontalHeaderLabels(table_headers);

  // signals originating from the table
  connect(layer_table, SIGNAL(cellChanged(int,int)), this, SLOT(updateLayerPropFromTable(int,int)));

  // populate table with layer info
  int layer_i = 0;
  for (prim::Layer* layer : *layers) {
    int curr_row = layer_table->rowCount();
    int curr_col = 0;
    QPushButton *bt_visibility = new QPushButton(QIcon(":/ico/visible.svg"), "", this);
    QPushButton *bt_editability = new QPushButton(QIcon(":/ico/editable.svg"), "", this);

    bt_visibility->setCheckable(true);
    bt_visibility->setChecked(layer->isVisible());

    // TODO editability has not been implemented in layer.cc yet
    //bt_editability->setCheckable(true);
    //bt_editability->setChecked(layer->isEditable());
    bt_editability->setChecked(true); // TODO remove this after implementing the above


    connect(bt_visibility, SIGNAL(toggled(bool)), layer, SLOT(visibilityCheckBoxChanged(bool)));
    //connect(bt_editability, SIGNAL(toggled(bool)), layer, SLOT(editabilityCheckBoxChanged(bool)));

    // insert row
    layer_table->insertRow(curr_row); // insert row at the bottom
    layer_table->setItem(curr_row, curr_col++, new QTableWidgetItem(QString::number(layer_i)));
    layer_table->setItem(curr_row, curr_col++, new QTableWidgetItem(layer->getName()));
    layer_table->setItem(curr_row, curr_col++, new QTableWidgetItem(layer->getContentType()));
    layer_table->setItem(curr_row, curr_col++, new QTableWidgetItem(QString::number(layer->getZHeight())));
    // TODO meter prefix (mm, um, nm, etc)
    layer_table->setCellWidget(curr_row, curr_col++, bt_visibility);
    layer_table->setCellWidget(curr_row, curr_col++, bt_editability);

    layer_i++;
    // TODO fold into addLayerRow()
  }

  // TODO restructure the widget such that the following items are only created
  // during initialization, instead of being remade every time the widget is
  // updated.


  // TODO UNDOable z-height change
  // TODO UNDOable layer creation in DesignPanel
}




// update widget
void LayerEditor:: addLayerRow()
{
  // TODO
}

// TODO
// z-height of db-surface is 0
// +ve for overhanging layers, -ve for buried
// ability to edit z-height for non-surface layers
// option to "always show layer distance from surface"
// states (checkboxes): visible, editable

}
