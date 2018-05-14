// @file:     layer_manager.h
// @author:   Samuel
// @created:  2017.12.22
// @editted:  2017.12.22 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     edit layer properties

#ifndef _GUI_LAYER_MANAGER_H_
#define _GUI_LAYER_MANAGER_H_

#include <QtWidgets>

#include "primitives/items.h"
#include "primitives/layer.h"

namespace gui{

  class LayerManager : public QWidget
  {
    Q_OBJECT

  public:

    enum LayerManagerColumn{ID, Type, Name, ZOffset, ZHeight, Visibility, Editability};
    Q_ENUM(LayerManagerColumn)

    //! Constructor.
    LayerManager(QStack<prim::Layer*> *layers, QWidget *parent);

    //! Destructor.
    ~LayerManager();

    //! Add a new layer with the given name. If no name is given, a default scheme
    //! is used. Checks if the layer already exists.
    void addLayer(const QString &name = QString(), const prim::Layer::LayerType cnt_type=prim::Layer::DB, const float zoffset = 0, const float zheight = 0);

    //! Attempt to remove a layer, by name.
    void removeLayer(const QString &name);

    //! Attempt to remove a layer, by index.
    void removeLayer(int n);

    //! Returns a pointer to the requested layer if it exists, else 0.
    prim::Layer* getLayer(const QString &name) const;

    //! Returns a pointer to the requested layer if it exists, else 0.
    prim::Layer* getLayer(int n) const;

    //! Get the top_layer index
    int getLayerIndex(prim::Layer *layer=0) const;

    //! Return the most recently used layer of the indicated type, or return
    //! the layer of that type with the smallest index if there isn't an MRU one.
    prim::Layer *getMRULayer(prim::Lattice::LayerType);

    //!

    // GUI items in layer list grid
    struct LayerTableRowContent
    {
      prim::Layer *layer;           // layer that this row corresponds to
      QTableWidgetItem *name;       // layer name (identifier in *layers)
      QTableWidgetItem *type;       // layer type
      QTableWidgetItem *zoffset;    // layer z-offset
      QTableWidgetItem *zheight;    // layer z-height
      QPushButton *bt_visibility;   // layer visibility
      QPushButton *bt_editability;  // layer editability (layer isActive)
    };

    void initLayerManager();
    void populateLayerTable();
    void refreshLayerTable();
    void clearLayerTable();

  public slots:
    void updateLayerPropFromTable(int row, int column);

  private:
    void initLayerTableHeaders();

    // functions for adding / removing layers
    void addLayerRow(); // wrapper, prompt user for new layer info and add to layer table
    void addLayerRow(prim::Layer *layer); // wrapper, add a row to layer table with layer pointer
    void addLayerRow(LayerTableRowContent *row_content); // actually adds the layer
    void removeLayerRow(prim::Layer *layer);

    // return the icon corresponding to a layer type
    QIcon layerType2Icon(const prim::Layer::LayerType);


    // vars
    QStack<prim::Layer*> *layers;
    QHash<prim::Layer::LayerType, prim::Layer*> mru_layers;
    // TODO table column order (mapped with string)

    // GUI
    QList<LayerTableRowContent*> table_row_contents;

    QVBoxLayout *layer_list_vl;
    QTableWidget *layer_table;
  };

}


#endif
