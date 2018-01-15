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
  //main_vl->addLayout(top_buttons_hl); TODO add this back when add function is implemented
  main_vl->addWidget(layer_table);

  setLayout(main_vl);

  // TODO change add/remove to signal based
}


void LayerEditor::initLayerTable()
{
  QStringList table_headers;
  table_headers << 
    "Layer ID" << // Layer ID, hidden
    "Type" <<     // Type (lattice, db, electrode) TODO types of default layers can't be changed
    "Name" <<     // Name TODO names of default layers can't be changed
    "Z-Height" << // Z-Height (vertical offset from surface) TODO surface zheight can't be changed
    "" <<  // Visibility
    "";   // Editability

  layer_table->setColumnCount(table_headers.count());
  // TODO don't hard code column ID
  layer_table->setColumnHidden(0, true); // hide Layer ID
  layer_table->resizeColumnToContents(4); // reduce width of visibility column
  layer_table->resizeColumnToContents(5); // reduce width of visibility column
  layer_table->setHorizontalHeaderLabels(table_headers);

  // header tooltips TODO there's probably a more elegant way
  layer_table->horizontalHeaderItem(1)->setToolTip("Layer Type: lattice, db, or electrode");
  layer_table->horizontalHeaderItem(2)->setToolTip("Layer Name");
  layer_table->horizontalHeaderItem(3)->setToolTip("Z-Height: vertical offset from surface.\nPositive for objects above surface, negative for objects under surface.");
  layer_table->horizontalHeaderItem(4)->setToolTip("Visibility of the layer");
  layer_table->horizontalHeaderItem(5)->setToolTip("Editability of the layer");

  // signals originating from the table
  connect(layer_table, SIGNAL(cellChanged(int,int)), this, SLOT(updateLayerPropFromTable(int,int)));

  // populate table with layer info
  int layer_i = 0;
  for (prim::Layer* layer : *layers) {
    int curr_row = layer_table->rowCount();
    int curr_col = 0;

    QTableWidgetItem *twi_type = new QTableWidgetItem(layer->getContentTypeString());
    twi_type->setIcon(layerType2Icon(layer->getContentType()));
    twi_type->setToolTip(layer->getContentTypeString());

    QPushButton *bt_visibility = new QPushButton(QIcon(":/ico/visible.svg"), "", this);
    QPushButton *bt_editability = new QPushButton(QIcon(":/ico/editable.svg"), "", this);

    bt_visibility->setCheckable(true);
    bt_visibility->setChecked(layer->isVisible());

    bt_editability->setCheckable(true);
    bt_editability->setChecked(layer->isActive());


    connect(bt_visibility, SIGNAL(toggled(bool)), layer, SLOT(visibilityPushButtonChanged(bool)));
    connect(bt_editability, SIGNAL(toggled(bool)), layer, SLOT(editabilityPushButtonChanged(bool)));

    // insert row
    layer_table->insertRow(curr_row); // insert row at the bottom
    layer_table->setItem(curr_row, curr_col++, new QTableWidgetItem(QString::number(layer_i)));
    layer_table->setItem(curr_row, curr_col++, twi_type);
    layer_table->setItem(curr_row, curr_col++, new QTableWidgetItem(layer->getName()));
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


void LayerEditor::updateLayerPropFromTable(int row, int column)
{
  // TODO really need to not hard code layer ID and column position, need some sort of table that translates between readable name and row/col number
  prim::Layer* layer = layers->at(layer_table->item(row, 0)->text().toInt()); // get layer according to Layer ID
  // TODO edit layer property
}


// update widget
void LayerEditor::addLayerRow()
{
  // TODO
}

QIcon LayerEditor::layerType2Icon(const prim::Layer::LayerType layer_type)
{
  // TODO make enumerated layer type instead of hard code string
  if (layer_type == prim::Layer::Lattice)
    return QIcon(":/ico/lattice.svg");
  /*else if (layer_type == prim::Layer::DB)
    return QIcon(":/ico/db.svg");
  else if (layer_type == prim::Layer::Electrode)
    return QIcon(":/ico/electrode.svg");*/
  else
    return QIcon(":/ico/unknown.svg");
}

// TODO
// z-height of db-surface is 0
// +ve for overhanging layers, -ve for buried
// ability to edit z-height for non-surface layers
// option to "always show layer distance from surface"
// states (checkboxes): visible, editable

}
