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
}

void ItemManager::initItemManager()
{
  item_table = new QTableWidget(this);
  QVBoxLayout *main_vl = new QVBoxLayout;
  main_vl->addWidget(item_table);
  initItemTableHeaders();
  setLayout(main_vl);
}


void ItemManager::initItemTableHeaders()
{
  qDebug() << "Initializing item table headers";
  QStringList table_headers;

  // TODO take enum type instead of this stringlist
  table_headers <<
    "Type" <<       // item_type QString
    "Layer" <<      // owning layer's index
    "Index" <<      // item's index within the layer
    "Properties"; // button to show properties

  item_table->setColumnCount(table_headers.length());
  item_table->setHorizontalHeaderLabels(table_headers);
  item_table->resizeColumnToContents(static_cast<int>(Type)); // reduce width of visibility column
  item_table->resizeColumnToContents(static_cast<int>(Layer)); // reduce width of visibility column
  item_table->resizeColumnToContents(static_cast<int>(Index)); // reduce width of visibility column
  item_table->resizeColumnToContents(static_cast<int>(Properties)); // reduce width of visibility column

  // header tooltips
  item_table->horizontalHeaderItem(static_cast<int>(Type))->
      setToolTip("Item type: DBDot, Electrode, etc.");
  item_table->horizontalHeaderItem(static_cast<int>(Layer))->
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
      addItemRow(item, layer);
    }
  }
}

void ItemManager::addItemRow(prim::Item *item, prim::Layer *layer)
{
  for (ItemTableRowContent* row_content: table_row_contents) {
    if (row_content->item == item) {
      return;
    }
  }
  ItemTableRowContent *new_content = new ItemTableRowContent();
  new_content->item = item;
  new_content->type = new QTableWidgetItem(QString::number(item->item_type));
  new_content->layer = new QTableWidgetItem(QString::number(item->layer_id));
  new_content->index = new QTableWidgetItem(QString::number(layer->getItemIndex(item)));
  new_content->bt_show_properties = new QPushButton(QString("Show properties"), this);
  table_row_contents.append(new_content);
  int curr_row = item_table->rowCount();
  item_table->insertRow(curr_row);
  item_table->setItem(curr_row, static_cast<int>(Type), new_content->type);
  item_table->setItem(curr_row, static_cast<int>(Layer), new_content->layer);
  item_table->setItem(curr_row, static_cast<int>(Index), new_content->index);
  item_table->setCellWidget(curr_row, static_cast<int>(Properties), new_content->bt_show_properties);
}

void ItemManager::updateTableRemove(prim::Item *item)
{
  qDebug() << "REMOVE";
}


}
