// @file:     layer_manager.cc
// @author:   Samuel
// @created:  2017.12.22
// @editted:  2017.12.22 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     widget for configuring

#include "layer_manager.h"

namespace gui {

// constructor
LayerManager::LayerManager(QWidget *parent)
  : QWidget(parent, Qt::Dialog)
{
  initLayerManager();
  //initDockWidget();
}

// destructor
LayerManager::~LayerManager()
{
  clearLayerTable();

  // delete the layers
  removeAllLayers();
}

void LayerManager::addLattice(prim::Lattice *lattice)
{
  lattice->setLayerID(layers.size());
  layers.append(lattice);
}

void LayerManager::addLayer(const QString &name, const prim::Layer::LayerType cnt_type, const float zoffset, const float zheight)
{
  // there can only be one layer of misc type
  for (prim::Layer *layer : layers) {
    if (cnt_type == prim::Layer::Misc && layer->contentType() == prim::Layer::Misc) {
      qWarning() << tr("A layer with type Misc already exists, more cannot be created.");
      return;
    }
  }

  // check if name already taken
  for(prim::Layer *layer : layers) {
    if(layer->getName() == name){
      qWarning() << tr("A layer already exists with the name : %1").arg(name);
      return;
    }
  }

  // layer is added to the end of layers stack, so ID = layers.size() before it was added
  prim::Layer *layer = new prim::Layer(name, cnt_type, zoffset, zheight, layers.size());
  layers.append(layer);
}

void LayerManager::removeLayer(const QString &name)
{
  bool removed = false;
  for (prim::Layer *layer : layers) {
    if (layer->getName() == name) {
      removeLayer(layer);
      removed=true;
      break;
    }
  }

  if(!removed)
    qWarning() << tr("Requested layer removal of %1 failed").arg(name);
}

void LayerManager::removeLayer(int n)
{
  if (n < 0 || n >= layers.count())
    qFatal("Layer index out of bounds, cannot delete layer");

  prim::Layer *layer = layers.takeAt(n);
  delete layer;

  // update layer_id for subsequent layers in the stack and their contained items
  for (int i=n; i<layers.count(); i++)
    layers.at(i)->setLayerID(i);

  // if active layer was removed, default to surface if available else 0
  if (active_layer == layer)
    active_layer = layers.count() > 1 ? layers.at(1) : 0;
}

void LayerManager::removeLayer(prim::Layer *layer)
{
  if(!layers.contains(layer))
    qFatal("Cannot remove layer, layer pointer doesn't exist");

  removeLayer(indexOf(layer));
}

void LayerManager::removeAllLayers()
{
  while (!layers.isEmpty())
    removeLayer(layers.count()-1);
}

prim::Layer* LayerManager::getLayer(const QString &name) const
{
  for(prim::Layer *layer : layers)
    if(layer->getName() == name)
      return layer;

  // no layer had a matching name, return 0
  qWarning() << tr("Failed to find layer : %1").arg(name);
  return 0;
}

prim::Layer* LayerManager::getLayer(int n) const
{
  if(n<0 || n>=layers.count()){
    qWarning() << tr("Layer index %1 out of bounds...").arg(n);
    return 0;
  }
  else
    return layers.at(n);
}

QList<prim::Layer*> LayerManager::getLayers(prim::Layer::LayerType type)
{
  QList<prim::Layer*> layers_found;
  for (prim::Layer *layer : layers)
    if (layer->contentType() == type)
      layers_found.append(layer);
  return layers_found;
}

void LayerManager::setActiveLayer(const QString &name)
{
  for(prim::Layer *layer : layers)
    if(layer->getName() == name){
      setActiveLayer(layer);
      return;
    }

  // no layer had a matching name, do nothing...
  qWarning() << tr("Failed to find layer : %1").arg(name);
}

void LayerManager::setActiveLayer(int n)
{
  if(n<0 || n>=layers.count())
    qWarning() << tr("Layer index out of bounds...");
  else
    setActiveLayer(layers.at(n));
}

void LayerManager::setActiveLayer(prim::Layer *layer)
{
  active_layer = layer;
  // TODO GUI stuff
  //update active layer indicator
  label_active_layer->setText(tr("Active layer: %1").arg(activeLayer()->getName()));
}

int LayerManager::indexOf(prim::Layer *layer) const
{
  return layer==0 ? layers.indexOf(active_layer) : layers.indexOf(layer);
}

prim::Layer *LayerManager::getMRULayer(prim::Layer::LayerType type)
{
  // find that layer in the MRU hash table
  if (mru_layers.contains(type))
    return mru_layers[type];

  // if no record in MRU, return the first occurance of the type in layers
  for (prim::Layer *layer : layers)
    if (layer->contentType() == type)
      return layer;

  return 0;
}

void LayerManager::saveLayers(QXmlStreamWriter *ws) const
{
  for(prim::Layer *layer : layers)
    layer->saveLayer(ws);
}

void LayerManager::saveLayerItems(QXmlStreamWriter *ws, DesignInclusionArea inclusion_area) const
{
  for(prim::Layer *layer : layers){
    ws->writeComment(layer->getName());
    layer->saveItems(ws, inclusion_area);
  }
}

void LayerManager::tableSelectionChanged(int row)
{
  //table selection was changed, change the active layer to the one selected.
  setActiveLayer(row);
  qDebug() << "Active layer changed to layer " << row;
}

// PRIVATE

// initialize widget
void LayerManager::initLayerManager()
{
  // top buttons
  QPushButton *bt_add = new QPushButton(tr("Add")); // TODO implement function

  QHBoxLayout *top_buttons_hl = new QHBoxLayout;
  top_buttons_hl->addWidget(bt_add);

  // grid layout that show all layers
  layer_table = new QTableWidget(this);
  initLayerTableHeaders();

  //trigger a slot whenever a cell is clicked, or when a row is selected.
  connect(layer_table->verticalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(tableSelectionChanged(int)));
  connect(layer_table, SIGNAL(cellClicked(int, int)), this, SLOT(tableSelectionChanged(int)));

  // Main layout
  QVBoxLayout *main_vl = new QVBoxLayout;
  // main_vl->addLayout(top_buttons_hl); TODO add this back when add function is implemented
  main_vl->addWidget(layer_table);

  setLayout(main_vl);

  close_shortcut_return = new QShortcut(QKeySequence(Qt::Key_Return), this, SLOT(hide()));
  close_shortcut_esc = new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(hide()));

  // TODO change add/remove to signal based

  initWizard();
}

void LayerManager::initWizard()
{
  //create new engine wizard
  add_layer_dialog = new QWidget(this, Qt::Dialog);

  // layouts
  add_layer_vl = new QVBoxLayout;
  QHBoxLayout *add_layer_name_hl = new QHBoxLayout;
  QHBoxLayout *add_layer_content_hl = new QHBoxLayout;
  QHBoxLayout *add_layer_offset_hl = new QHBoxLayout;
  QHBoxLayout *add_layer_height_hl = new QHBoxLayout;
  QHBoxLayout *add_layer_button_hl = new QHBoxLayout;
  add_layer_name_hl->addStretch(1);
  add_layer_content_hl->addStretch(1);
  add_layer_offset_hl->addStretch(1);
  add_layer_height_hl->addStretch(1);
  add_layer_button_hl->addStretch(1);

  QLabel *label_layer_name = new QLabel(tr("Layer Name:"));
  QLabel *label_layer_content = new QLabel(tr("Layer Content:"));
  QLabel *label_layer_offset = new QLabel(tr("Layer Offset:"));
  QLabel *label_layer_height = new QLabel(tr("Layer Height:"));
  le_layer_name = new QLineEdit();
  cb_layer_content = new QComboBox();
  le_layer_offset = new QLineEdit();
  le_layer_height = new QLineEdit();
  QPushButton *pb_add_layer = new QPushButton(tr("Add"));
  QPushButton *pb_cancel = new QPushButton(tr("Cancel"));

  add_layer_name_hl->addWidget(label_layer_name);
  add_layer_name_hl->addWidget(le_layer_name);
  add_layer_content_hl->addWidget(label_layer_content);
  add_layer_content_hl->addWidget(cb_layer_content);
  add_layer_offset_hl->addWidget(label_layer_offset);
  add_layer_offset_hl->addWidget(le_layer_offset);
  add_layer_height_hl->addWidget(label_layer_height);
  add_layer_height_hl->addWidget(le_layer_height);
  add_layer_button_hl->addWidget(pb_add_layer);
  add_layer_button_hl->addWidget(pb_cancel);

  add_layer_vl->addLayout(add_layer_name_hl);
  add_layer_vl->addLayout(add_layer_content_hl);
  add_layer_vl->addLayout(add_layer_offset_hl);
  add_layer_vl->addLayout(add_layer_height_hl);
  add_layer_vl->addLayout(add_layer_button_hl);

  add_layer_dialog->setLayout(add_layer_vl);

  //Getting the enum names for layer content
  QMetaObject layer_mojb = prim::Layer::staticMetaObject;
  QMetaEnum layer_types = layer_mojb.enumerator(layer_mojb.indexOfEnumerator("LayerType"));

  //populating the layer content combobox
  for( int i = 0; i < layer_types.keyCount(); i++){
    cb_layer_content->addItem(layer_types.key(i));
  }

  //taking care of buttons
  connect(pb_add_layer, &QAbstractButton::clicked, this, &LayerManager::addByWizard);
  connect(pb_cancel, &QAbstractButton::clicked, add_layer_dialog, &QWidget::hide);

  add_layer_dialog->setWindowTitle(tr("Add new layer"));
}

void LayerManager::addByWizard(){
  QMetaObject layer_mojb = prim::Layer::staticMetaObject;
  QMetaEnum layer_types = layer_mojb.enumerator(layer_mojb.indexOfEnumerator("LayerType"));
  //grab the data from the dialog
  QString layer_name = le_layer_name->text();
  prim::Layer::LayerType content_type = static_cast<prim::Layer::LayerType>(cb_layer_content->currentIndex());
  float offset = le_layer_offset->text().toFloat();
  float height = le_layer_height->text().toFloat();
  //create a new layer
  addLayer(layer_name, content_type, offset, height);
  //get the newly created layer
  prim::Layer* layer = getLayer(layerCount()-1);
  //add it to the layer table
  addLayerRow(layer);
  //clear the dialog
  le_layer_name->clear();
  le_layer_offset->clear();
  le_layer_height->clear();
  //hide the dialog
  add_layer_dialog->hide();
}

void LayerManager::initSideWidget()
{
  QList<QPair<float, prim::Layer*>> sorted_layers;
  for (prim::Layer *layer : layers)
    sorted_layers.append(qMakePair(layer->zOffset(), layer));
  std::sort(sorted_layers.begin(), sorted_layers.end(), QPairFirstReverseComparer());

  //create label to show user the current active layer
  label_active_layer = new QLabel(tr("Active layer: "));
  QHBoxLayout *active_layer_hl = new QHBoxLayout;
  active_layer_hl->addWidget(label_active_layer);

  // construct layer widgets row by row
  QVBoxLayout *layers_vl = new QVBoxLayout;
  layers_vl->setAlignment(Qt::AlignTop);
  QHBoxLayout *row_hl = new QHBoxLayout;
  layers_vl->addLayout(active_layer_hl);
  for (int i=0; i<sorted_layers.size(); i++) {
    prim::Layer *layer = sorted_layers[i].second;
    QLabel *lb_layer_nm = new QLabel(layer->getName());
    QPushButton *pb_vsb = new QPushButton(QIcon(":/ico/visible.svg"), "", this);
    QPushButton *pb_atv = new QPushButton(QIcon(":/ico/editable.svg"), "", this);
    qDebug() << tr("Creating entry for %1 with zOffset %2").arg(layer->getName()).arg(layer->zOffset());

    pb_vsb->setCheckable(true);
    pb_vsb->setChecked(layer->isVisible());
    pb_atv->setCheckable(true);
    pb_atv->setChecked(layer->isActive());

    if (layer->contentType() == prim::Layer::Lattice) {
      connect(pb_vsb, &QAbstractButton::toggled,
              static_cast<prim::Lattice*>(layer), &prim::Lattice::setVisible);
    } else {
      connect(pb_vsb, &QAbstractButton::toggled, layer, &prim::Layer::setVisible);
    }
    connect(pb_atv, &QAbstractButton::toggled, layer, &prim::Layer::setActive);

    QHBoxLayout *layer_hl = new QHBoxLayout;
    layer_hl->addWidget(lb_layer_nm);
    layer_hl->addStretch();
    layer_hl->addWidget(pb_vsb);
    layer_hl->addWidget(pb_atv);

    QGroupBox *layer_wg = new QGroupBox;
    layer_wg->setLayout(layer_hl);

    if (i > 0 && layer->zOffset() == sorted_layers[i-1].second->zOffset()) {
      // add to the same row as before
      row_hl->addWidget(layer_wg);
      if (i == sorted_layers.size()-1) {
        layers_vl->addLayout(row_hl);
      }
    } else if (i == sorted_layers.size()-1) {
      // last layer to add
      layers_vl->addLayout(row_hl);
      row_hl = new QHBoxLayout;
      row_hl->addWidget(layer_wg);
      layers_vl->addLayout(row_hl);
    } else {
      layers_vl->addLayout(row_hl);
      row_hl = new QHBoxLayout;
      row_hl->addWidget(layer_wg);
    }
  }

  QPushButton *pb_adv = new QPushButton("Advanced");
  connect(pb_adv, &QAbstractButton::clicked, this, &QWidget::show);
  QPushButton *pb_add = new QPushButton("Add Layer");
  connect(pb_add, SIGNAL(clicked()),
            this, SLOT(addLayerRow()));

  QHBoxLayout *btn_hl = new QHBoxLayout;
  btn_hl->addStretch();
  btn_hl->addWidget(pb_adv);
  btn_hl->addWidget(pb_add);
  layers_vl->addLayout(btn_hl);

  side_widget = new QWidget(0, Qt::Widget);
  side_widget->setLayout(layers_vl);

}

void LayerManager::initLayerTableHeaders()
{
  qDebug() << "Initializing layer table headers";
  QStringList table_headers;
  // TODO take enum type instead of this stringlist
  table_headers <<
    "ID" <<       // Layer ID, layer's position in layers* stack
    "Type" <<     // Type (lattice, db, electrode) TODO types of default layers can't be changed
    "Name" <<     // Name TODO names of default layers can't be changed
    "Z-Offset" << // Z-Offset (vertical offset from surface) TODO surface zheight can't be changed
    "Z-Height" << // Z-Height (vertical height of layer)
    "" <<  // Visibility
    "";   // Editability

  layer_table->setColumnCount(table_headers.length());
  layer_table->setHorizontalHeaderLabels(table_headers);
  layer_table->setColumnHidden(static_cast<int>(ID), true);
  layer_table->resizeColumnToContents(static_cast<int>(Visibility)); // reduce width of visibility column
  layer_table->resizeColumnToContents(static_cast<int>(Editability)); // reduce width of visibility column

  // header tooltips
  layer_table->horizontalHeaderItem(static_cast<int>(Type))->
      setToolTip("Layer Type: lattice, db, or electrode");
  layer_table->horizontalHeaderItem(static_cast<int>(Name))->
      setToolTip("Layer Name");
  layer_table->horizontalHeaderItem(static_cast<int>(ZOffset))->
      setToolTip("Z-Offset: vertical offset from surface.\nPosition for objects above surface, negative for objects below surface.");
  layer_table->horizontalHeaderItem(static_cast<int>(ZHeight))->
      setToolTip("Z-Height: vertical height of the layer");
  layer_table->horizontalHeaderItem(static_cast<int>(Visibility))->
      setToolTip("Visibility of the layer");
  layer_table->horizontalHeaderItem(static_cast<int>(Editability))->
      setToolTip("Editability of the layer");
}


void LayerManager::populateLayerTable()
{
  clearLayerTable();

  // populate table with layer info
  qDebug() << "Populating layer table";
  for (prim::Layer* layer : layers)
    addLayerRow(layer);

  // signals originating from the table
  connect(layer_table, SIGNAL(cellChanged(int,int)),
            this, SLOT(updateLayerPropFromTable(int,int)));

  adjustSize();
  // TODO UNDOable z-height change
  // TODO UNDOable layer creation in DesignPanel
}


void LayerManager::refreshLayerTable()
{
  // reload all rows and update with changes, including added/deleted layers
  // TODO
}


void LayerManager::clearLayerTable()
{
  // Delete layer rows and disconnect all signals within the table.
  // Called by destructor on exit or by design panel when loading new file.
  while (!table_row_contents.isEmpty()) {
    LayerTableRowContent *row_content = table_row_contents.takeLast();
    row_content->bt_visibility->disconnect();
    row_content->bt_editability->disconnect();
    delete row_content;
  }
  layer_table->setRowCount(0);  // delete all rows from layer table
}


void LayerManager::updateLayerPropFromTable(int row, int column)
{
  // TODO really need to not hard code layer ID and column position, need some sort of table that translates between readable name and row/col number
  QString layer_name = layer_table->item(row, static_cast<int>(Name))->text();
  //qDebug() << QObject::tr("Row=%1, Col=%2, Layer name=%3").arg(row).arg(column).arg(layer_name);
  prim::Layer* layer = getLayer(layer_name); // get layer according to Layer Name

  switch(column) {
    case static_cast<int>(ID):
      // not supposed to get this
      break;
    case static_cast<int>(Type):
      // not supposed to get this
      break;
    case static_cast<int>(Name):
      // not supposed to get this
      break;
    case static_cast<int>(ZOffset):
      layer->setZOffset(layer_table->item(row, column)->text().toFloat());
      break;
    case static_cast<int>(ZHeight):
      layer->setZHeight(layer_table->item(row, column)->text().toFloat());
      break;
    case static_cast<int>(Visibility):
      // not supposed to get this
      break;
    case static_cast<int>(Editability):
      // not supposed to get this
      break;
    default:
      // not supposed to get this
      break;
  }
  // TODO edit layer property
}


void LayerManager::addLayerRow()
{
  add_layer_dialog->show();
  // populateLayerTable();
  // qDebug() << layerCount();
  return;
}

// update widget
void LayerManager::addLayerRow(prim::Layer *layer)
{
  LayerTableRowContent *curr_row_content = new LayerTableRowContent;
  curr_row_content->layer = layer;

  //qDebug() << tr("Constructing layer row GUI elements for layer %1").arg(layer->getName());

  // items that require signal disconnection at removal
  curr_row_content->bt_visibility = new QPushButton(QIcon(":/ico/visible.svg"), "", this);
  curr_row_content->bt_editability = new QPushButton(QIcon(":/ico/editable.svg"), "", this);

  curr_row_content->bt_visibility->setCheckable(true);
  curr_row_content->bt_visibility->setChecked(layer->isVisible());
  curr_row_content->bt_editability->setCheckable(true);
  curr_row_content->bt_editability->setChecked(layer->isActive());

  connect(curr_row_content->bt_visibility, &QAbstractButton::toggled,
          layer, &prim::Layer::setVisible);
  connect(curr_row_content->bt_editability, &QAbstractButton::toggled,
          layer, &prim::Layer::setActive);

  // other items
  curr_row_content->type = new QTableWidgetItem(layer->contentTypeString());
  curr_row_content->type->setIcon(layerType2Icon(layer->contentType()));
  curr_row_content->type->setToolTip(layer->contentTypeString());

  curr_row_content->name = new QTableWidgetItem(layer->getName());
  curr_row_content->zoffset = new QTableWidgetItem(QString::number(layer->zOffset()));
  curr_row_content->zheight = new QTableWidgetItem(QString::number(layer->zHeight()));

  //qDebug() << QObject::tr("type=%1").arg(curr_row_content->type->text());

  // add to table
  addLayerRow(curr_row_content);
}


void LayerManager::addLayerRow(LayerTableRowContent *row_content)
{
  table_row_contents.append(row_content);

  // add elems to table
  int curr_row = layer_table->rowCount();
  //qDebug() << QObject::tr("curr_row is %1").arg(curr_row);

  //qDebug() << tr("Inserting layer GUI elements to table into row %1").arg(curr_row);

  layer_table->insertRow(curr_row);
  //qDebug() << QObject::tr("type=%1").arg(row_content->type->text());
  layer_table->setItem(curr_row, static_cast<int>(Name), row_content->name);
  // segfault would occur when loading files if type was placed before name...
  layer_table->setItem(curr_row, static_cast<int>(Type), row_content->type);
  layer_table->setItem(curr_row, static_cast<int>(ZOffset), row_content->zoffset);
  layer_table->setItem(curr_row, static_cast<int>(ZHeight), row_content->zheight);

  layer_table->setCellWidget(curr_row, static_cast<int>(Visibility), row_content->bt_visibility);
  layer_table->setCellWidget(curr_row, static_cast<int>(Editability), row_content->bt_editability);

}


QIcon LayerManager::layerType2Icon(const prim::Layer::LayerType layer_type)
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
