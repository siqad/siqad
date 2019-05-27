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

#include "../primitives/items.h"
#include "layer_manager.h"

namespace gui{
  class ItemManager : public QWidget
  {
    Q_OBJECT

  public:

    enum ItemManagerColumn{Type, LayerName, LayerID, Index, Properties};
    Q_ENUM(ItemManagerColumn)


    ItemManager(QWidget *parent, LayerManager* layman_in);
    ~ItemManager();

    // bool eventFilter(QObject *object, QEvent *event);

    struct ItemTableRowContent
    {
      prim::Item *item;
      QTableWidgetItem *type;
      QTableWidgetItem *layer_name;
      QTableWidgetItem *layer_id;
      QTableWidgetItem *index;
      QPushButton *bt_show_properties;
    };

  signals:
    void sig_deselect();
    void sig_delete_selected();

  public slots:
    void updateTableAdd();
    void updateTableRemove(prim::Item* item);
    void showProperties();
    void updateItemSelection();
    void deleteItemSelection();

  private:
    void initItemManager();
    void initItemTableHeaders();
    void addItemRow(prim::Item *item);
    void clearItemTable();

    LayerManager *layman;
    QTableWidget *item_table;
    QList<ItemTableRowContent*> table_row_contents;
    QVBoxLayout *main_vl;
  };

  class TableWidget: public QTableWidget
  {
    Q_OBJECT
  public:
    TableWidget(QWidget *parent = 0);

  signals:
    void sig_update_selection();
    void sig_delete_selection();

  public slots:
    void showContextMenu(const QPoint& p);
    void deleteItems();

  protected:
    void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;

  private:
    QMenu menu;
  };
}

#endif
