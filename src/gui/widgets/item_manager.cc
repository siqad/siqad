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
  qDebug() << "ITEM MANAGER WOOOOO";
}

ItemManager::~ItemManager()
{
  qDebug() << "@@Destructor for ItemManager@@";
}

}
