// @file:     item_manager.h
// @author:   Nathan
// @created:  2018.07.06
// @editted:  2018.07.06 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     Widget that holds item information.

#ifndef _GUI_ITEM_MANAGER_H_
#define _GUI_ITEM_MANAGER_H_

#include <QtWidgets>

#include "primitives/items.h"

namespace gui{
  class ItemManager : public QWidget
  {
    Q_OBJECT

  public:
    ItemManager(QWidget *parent);
    ~ItemManager();
  };
}

#endif
