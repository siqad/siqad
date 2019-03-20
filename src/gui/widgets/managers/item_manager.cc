// @file:     item_manager.cc
// @author:   Nathan
// @created:  2018.07.06
// @editted:  2018.07.06 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     Function definitions for widget displaying item information

#include "item_manager.h"

namespace gui{

ItemManager::ItemManager(QWidget *parent, LayerManager* layman_in)
  : QWidget(parent, Qt::Dialog)
{
  layman = layman_in;
  initItemManager();
}

ItemManager::~ItemManager()
{
  clearItemTable();
  layman = 0;
  delete item_table;
  delete main_vl;
}

void ItemManager::initItemManager()
{
  item_table = new QTableWidget(this);
  main_vl = new QVBoxLayout;
  main_vl->addWidget(item_table);
  initItemTableHeaders();
  setLayout(main_vl);
}

void ItemManager::clearItemTable()
{
  while (!table_row_contents.isEmpty()) {
    ItemTableRowContent *row_content = table_row_contents.takeLast();
    row_content->bt_show_properties->disconnect();
    delete row_content;
  }
  item_table->setRowCount(0);  // delete all rows from layer table
}

void ItemManager::initItemTableHeaders()
{
  qDebug() << "Initializing item table headers";
  QStringList table_headers;

  // TODO take enum type instead of this stringlist
  table_headers <<
    "Type" <<       // item_type QString
    "Layer Name" <<      // owning layer's index
    "Layer ID" <<      // owning layer's index
    "Index" <<      // item's index within the layer
    "Properties"; // button to show properties

  item_table->setColumnCount(table_headers.length());
  item_table->setHorizontalHeaderLabels(table_headers);
  item_table->resizeColumnToContents(static_cast<int>(Type)); // reduce width of visibility column
  item_table->resizeColumnToContents(static_cast<int>(LayerName)); // reduce width of visibility column
  item_table->resizeColumnToContents(static_cast<int>(LayerID)); // reduce width of visibility column
  item_table->resizeColumnToContents(static_cast<int>(Index)); // reduce width of visibility column
  item_table->resizeColumnToContents(static_cast<int>(Properties)); // reduce width of visibility column

  // header tooltips
  item_table->horizontalHeaderItem(static_cast<int>(Type))->
      setToolTip("Item type: DBDot, Electrode, etc.");
  item_table->horizontalHeaderItem(static_cast<int>(LayerName))->
      setToolTip("Layer Name");
  item_table->horizontalHeaderItem(static_cast<int>(LayerID))->
      setToolTip("Layer ID");
  item_table->horizontalHeaderItem(static_cast<int>(Index))->
      setToolTip("Item index");
  item_table->horizontalHeaderItem(static_cast<int>(Properties))->
      setToolTip("Show properties");
}

void ItemManager::updateTableAdd()
{
  for (int i=0;i < layman->layerCount(); i++) {
    prim::Layer* layer = layman->getLayer(i);
    for (prim::Item* item : layer->getItems()) {
      addItemRow(item);
    }
  }
}

void ItemManager::addItemRow(prim::Item *item)
{
  if (item == 0)
    return;
  for (ItemTableRowContent* row_content: table_row_contents) {
    if (row_content->item == item) {
      return;
    }
  }
  ItemTableRowContent *new_content = new ItemTableRowContent();
  new_content->item = item;
  new_content->type = new QTableWidgetItem(item->getQStringItemType());
  new_content->layer_name = new QTableWidgetItem(layman->getLayer(item->layer_id)->getName());
  new_content->layer_id = new QTableWidgetItem(QString::number(item->layer_id));
  new_content->index = new QTableWidgetItem(QString::number(layman->getLayer(item->layer_id)->getItemIndex(item)));
  //the QString in buttons must be exactly "Show properties" in order to trigger showProps() from items
  new_content->bt_show_properties = new QPushButton(QString("Show properties"), this);
  connect(new_content->bt_show_properties, &QAbstractButton::clicked, this, &ItemManager::showProperties);

  table_row_contents.append(new_content);
  int curr_row = item_table->rowCount();
  item_table->insertRow(curr_row);
  item_table->setItem(curr_row, static_cast<int>(Type), new_content->type);
  item_table->setItem(curr_row, static_cast<int>(LayerName), new_content->layer_name);
  item_table->setItem(curr_row, static_cast<int>(LayerID), new_content->layer_id);
  item_table->setItem(curr_row, static_cast<int>(Index), new_content->index);
  item_table->setCellWidget(curr_row, static_cast<int>(Properties), new_content->bt_show_properties);
}


void ItemManager::showProperties()
{
  for (ItemTableRowContent* row_content: table_row_contents) {
    if (row_content->bt_show_properties == static_cast<QPushButton *>(sender())) {
      QAction temp_action;
      temp_action.setText(row_content->bt_show_properties->text());
      row_content->item->performAction(&temp_action);
      return;
    }
  }
}


void ItemManager::updateTableRemove(prim::Item *item)
{
  for (ItemTableRowContent* row_content: table_row_contents) {
    if (row_content->item == item) {
      table_row_contents.removeAt(table_row_contents.indexOf(row_content));
      item_table->removeRow(item_table->row(row_content->type));
      row_content->bt_show_properties->disconnect();
      return;
    }
  }
}


}
