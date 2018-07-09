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
#include "layer_manager.h"

namespace gui{
  class ItemManager : public QWidget
  {
    Q_OBJECT

  public:

    enum ItemManagerColumn{Type, Layer, Index, Properties};
    Q_ENUM(ItemManagerColumn)


    ItemManager(QWidget *parent, LayerManager* layman_in);
    ~ItemManager();

    struct ItemTableRowContent
    {
      prim::Item *item;
      QTableWidgetItem *type;
      QTableWidgetItem *layer;
      QTableWidgetItem *index;
      QPushButton *bt_show_properties;
    };

  public slots:
    void updateTable();

  private:
    void initItemManager();
    void initItemTableHeaders();
    void addItemRow();

    LayerManager *layman;
    QTableWidget *item_table;
  };
}

#endif
