// @file:     layer_editor.h
// @author:   Samuel
// @created:  2017.12.22
// @editted:  2017.12.22 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     edit layer properties

#ifndef _GUI_LAYER_EDITOR_H_
#define _GUI_LAYER_EDITOR_H_

#include <QtWidgets>

#include "design_panel.h"
#include "primitives/items.h"
#include "primitives/layer.h"

namespace gui{

  class LayerEditor : public QWidget
  {
    Q_OBJECT
  public:
    // constructor
    LayerEditor(gui::DesignPanel *design_pan, QWidget *parent);

    // destructor
    ~LayerEditor();

    // GUI items in layer list grid TODO remove if not needed
    struct LayerTableRowItems
    {
      prim::Layer *layer;           // layer that this row corresponds to
      QTableWidgetItem *name;       // layer name (identifier in *layers)
      QTableWidgetItem *type;       // layer type
      QTableWidgetItem *zheight;    // layer z-height
      QPushButton *bt_visibility;   // layer visibility
      QPushButton *bt_editability;  // layer editability (layer isActive)
    };

    void initLayerTable();
    void refreshLayerTable();
    void clearLayerTable();

  public slots:
    void updateLayerPropFromTable(int row, int column);

  private:
    void initLayerEditor();

    // functions for adding / removing layers
    void addLayerRow(); // wrapper, prompt user for new layer info and add to layer table
    void addLayerRow(prim::Layer *layer); // wrapper, add a row to layer table with layer pointer
    void addLayerRow(LayerTableRowItems *row_items); // actually adds the layer
    void removeLayerRow(prim::Layer *layer);

    // corresponding column number for specified column header
    int findColumn(const QString &header_text); 

    // return the icon corresponding to a layer type
    QIcon layerType2Icon(const prim::Layer::LayerType);


    // vars
    gui::DesignPanel *dp;
    QStack<prim::Layer*> *layers;
    // TODO table column order (mapped with string)

    // GUI
    QList<LayerTableRowItems*> row_items;

    QVBoxLayout *layer_list_vl;
    QTableWidget *layer_table;
  };

}


#endif
