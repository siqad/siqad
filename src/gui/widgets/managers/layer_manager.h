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

#include "../primitives/items.h"
#include "../primitives/layer.h"
#include "../primitives/lattice.h"

namespace gui{

  //! Mini widget for controlling layer
  class LayerControlWidget : public QFrame
  {
    Q_OBJECT

  public:

    //! Construct the widget.
    LayerControlWidget(prim::Layer *layer);

    //! Update this widget to reflect whether it's the currently selected layer.
    void setCurrent(const bool &selected);

  protected:

    //! Override mouse press event to allow users to select this layer as the 
    //! current layer.
    void mousePressEvent(QMouseEvent *) override;

  signals:

    //! Request for the layer represented by this widget to be selected.
    void sig_requestLayerSelection(prim::Layer *);

  private:

    //! Store the layer pointer.
    prim::Layer *layer;

  };

  class LayerManagerSidebar : public QWidget
  {
    Q_OBJECT

  public:
    
    //! Construct the layer manager sidebar widget.
    LayerManagerSidebar(const QStack<prim::Layer*> layers, QWidget *parent);

    //! Refresh layer manager sidebar contents (each layer addition/removal 
    //! requires the full list to be updated.
    //! @layers list of existing layers.
    void refreshLists(const QStack<prim::Layer*> layers);

    //! Update current layer. The information text is updated and the layer 
    //! is highlighted.
    //! @layer pointer to the layer
    void updateCurrentLayer(prim::Layer *layer);

    //! Activate simulation result display. Original layers are hidden.
    //! TODO implement
    //! TODO GUI interaction with original layers should be disabled
    void activateSimVisLayers(const QStack<prim::Layer*>) {};

    //! Deactivate simulation result display, original layers are restored to 
    //! their cached states.
    //! TODO implement
    void deactivateSimVisLayers() {};

  public slots:


  signals:

    //! Tell layer manager to show advanced layer panel.
    void sig_showAdvancedPanel();

    //! Tell layer manager to show add layer dialog.
    void sig_showAddLayerDialog();

    //! Request layer manager to select the specified layer.
    void sig_requestLayerSelection(prim::Layer *);

  private:

    //! Initialize the widget.
    void initialize();

    //! Clear a layout.
    void clearLayout(QLayout *layout)
    {
      QLayoutItem *item;
      while((item = layout->takeAt(0))) {
        if (item->layout()) {
          clearLayout(item->layout());
          delete item->layout();
        }
        if (item->widget()) {
          delete item->widget();
        }
      }
    }

    // VARS
    QMap<prim::Layer*, LayerControlWidget*> lay_widgets;

    // GUI VARS
    QLabel *l_curr_lay;
    QVBoxLayout *vl_overlays;
    QVBoxLayout *vl_layers;
  };

  class LayerManager : public QWidget
  {
    Q_OBJECT

  public:

    enum LayerManagerColumn{ID, Type, Name, ZOffset, ZHeight, Visibility, Editability};
    Q_ENUM(LayerManagerColumn)

    //! Constructor.
    LayerManager(QWidget *parent);

    //! Destructor.
    ~LayerManager();

    //! Add a lattice.
    void addLattice(prim::Lattice *lattice);

    //! Add a new layer with the given name. If no name is given, a default scheme
    //! is used. Checks if the layer already exists.
    bool addLayer(const QString &name = QString(),
                  const prim::Layer::LayerType cnt_type=prim::Layer::DB,
                  const prim::Layer::LayerRole role=prim::Layer::LayerRole::Design,
                  const float zoffset = 0, const float zheight = 0);

    //! Attempt to remove a layer, by name (everything except for Result layers).
    void removeLayer(const QString &name);

    //! Attempt to remove a layer, by index (everything except for Result layers).
    void removeLayer(int n);

    //! Attempt to remove a layer, by pointer (everything except for Result layers).
    void removeLayer(prim::Layer *layer);

    //! Attempt to remove all layers (everything except for Result layers).
    void removeAllLayers();

    //! Remove all result layers.
    void removeResultLayers();

    //! Returns a pointer to the requested layer if it exists, else 0.
    prim::Layer* getLayer(const QString &name) const;

    //! Returns a pointer to the requested layer if it exists, else 0.
    prim::Layer* getLayer(int n) const;

    //! Returns all layer pointers for the specified layer type, or an empty list
    //! if none exists.
    QList<prim::Layer*> getLayers(prim::Layer::LayerType);

    //! Returns the number of layers in the layers stack.
    int layerCount() {return layers.count();}

    //! Returns the pointer to the active layer.
    prim::Layer* activeLayer() {return active_layer;}

    //! Set the active layer to the given layer name.
    void setActiveLayer(const QString &name);

    //! Set the active layer to the given layer index.
    void setActiveLayer(int n);

    //! Set the active layer to the given layer pointer.
    void setActiveLayer(prim::Layer *layer);

    //! Get the top_layer index
    //int getLayerIndex(prim::Layer *layer=0) const;
    int indexOf(prim::Layer *layer=0) const;

    //! Return the most recently used layer of the indicated type, or return
    //! the layer of that type with the smallest index if there isn't an MRU one.
    //! Returns 0 if none is found.
    prim::Layer *getMRULayer(prim::Layer::LayerType);

    //! Similar to getMRULayer but return the ID.
    int getMRULayerID(prim::Layer::LayerType lt) {return indexOf(getMRULayer(lt));}

    // SAVE / LOAD

    //! Save properties of all layers to XML stream. TODO improve structure
    void saveLayers(QXmlStreamWriter *) const;

    //! Save items contained in all layers to XML stream. TODO improve structure
    void saveLayerItems(QXmlStreamWriter *, DesignInclusionArea) const;


    // GUI

    //! Return a pointer to the layer dock widget, which is intended to be owned
    //! by ApplicaionGUI and has reduced functionality from the full layer manager.
    QWidget *sideWidget() {return side_widget;}

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
    void initSideWidget();
    void populateLayerTable();
    void refreshLayerTable();
    void clearLayerTable();

  public slots:
    void updateLayerPropFromTable(int row, int column);
    void addLayerRow(); // wrapper, prompt user for new layer info and add to layer table
    void tableSelectionChanged(int row);

  private:
    void initLayerTableHeaders();
    void initWizard();
    void addByWizard();

    // functions for adding / removing layers
    // void addLayerRow(); // wrapper, prompt user for new layer info and add to layer table
    void addLayerRow(prim::Layer *layer); // wrapper, add a row to layer table with layer pointer
    void addLayerRow(LayerTableRowContent *row_content); // actually adds the layer
    void removeLayerRow(prim::Layer *layer);

    // return the icon corresponding to a layer type
    QIcon layerType2Icon(const prim::Layer::LayerType);


    // vars
    prim::Layer *active_layer=0;
    QStack<prim::Layer*> layers;
    QStack<prim::Layer*> simvislayers;
    QHash<prim::Layer::LayerType, prim::Layer*> mru_layers;
    // TODO table column order (mapped with string)

    // GUI
    LayerManagerSidebar *side_widget=nullptr;
    QWidget *add_layer_dialog=0;
    QList<LayerTableRowContent*> table_row_contents;

    QVBoxLayout *layer_list_vl;
    QTableWidget *layer_table;
    QShortcut* close_shortcut_return;
    QShortcut* close_shortcut_esc;

    //new layer wizard

    QVBoxLayout *add_layer_vl;
    QLineEdit *le_layer_name;
    QComboBox *cb_layer_content;
    QLineEdit *le_layer_offset;
    QLineEdit *le_layer_height;

  };


}


#endif
