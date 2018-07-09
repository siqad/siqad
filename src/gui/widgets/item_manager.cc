// @file:     item_manager.cc
// @author:   Nathan
// @created:  2018.07.06
// @editted:  2018.07.06 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     Function definitions for widget displaying item information

#include "item_manager.h"

namespace gui{

ItemManager::ItemManager(QWidget *parent)
  : QWidget(parent, Qt::Dialog)
{
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

void ItemManager::updateTable()
{
  qDebug() << "UPDATING TABLE";
}



}
