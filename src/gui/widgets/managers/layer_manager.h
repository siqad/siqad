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
#include "../primitives/dblayer.h"

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
    LayerManagerSidebar(const QStack<prim::Layer*> layers, 
        const QStack<prim::Layer*> simvislayers, QWidget *parent);

    //! Refresh layer manager sidebar contents (each layer addition/removal 
    //! requires the full list to be updated.
    //! @layers list of existing layers.
    void refreshLists(const QStack<prim::Layer*> layers, 
        const QStack<prim::Layer*> simvislayers);

    //! Update current layer. The information text is updated and the layer 
    //! is highlighted.
    //! @layer pointer to the layer
    void updateCurrentLayer(prim::Layer *layer);

    //! Activate/deactivate simulation result display. Original layers are 
    //! hidden. When result display is enabled.
    //! Result display cannot be enabled if no simvislayers had been provided.
    void setSimVisualizeMode(const bool &enable);

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
      while ((item = layout->takeAt(0)) != nullptr) {
        if (QLayout *childLayout = item->layout()) {
          clearLayout(childLayout);
          delete childLayout;
        } else if (QWidget *widget = item->widget()) {
          delete widget;
        }
      }
    }

    // VARS
    QMap<prim::Layer*, LayerControlWidget*> lay_widgets;

    // GUI VARS
    QLabel *l_curr_lay;
    QGroupBox *gb_overlays;
    QGroupBox *gb_layers;
    QGroupBox *gb_result_layers;
    QVBoxLayout *vl_overlays;
    QVBoxLayout *vl_layers;
    QVBoxLayout *vl_result_layers;
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

    //! Activate/deactivate simulation result display. Hide all design layers
    //! and show result layers.
    //! When deactivating, the simvislayers content is cleared.
    void setSimVisualizeMode(const bool &enable);

    //! Add a lattice.
    void addLattice(prim::Lattice *lattice, prim::Layer::LayerRole role=prim::Layer::Design);

    //! Return the lattice in the specified role (design or simvis).
    //! Assumes that only one lattice is present in either role.
    //! If that list doesn't have a lattice, return a nullptr.
    prim::Lattice *getLattice(bool design_role);

    //! Add a new layer with the given name. If no name is given, a default scheme
    //! is used. Checks if the layer already exists.
    //! Layers with Result role are tracked on a separate list, name clashes
    //! between Result role layers and other layer roles are allowed.
    //! Returns the pointer to the layer, or nullptr if unsuccessful.
    prim::Layer *addLayer(const QString &name = QString(),
        const prim::Layer::LayerType &cnt_type=prim::Layer::DB,
        const prim::Layer::LayerRole &role=prim::Layer::LayerRole::Design,
        const float &zoffset = 0, const float &zheight = 0);

    //! Add a DB layer.
    prim::DBLayer *addDBLayer(prim::Lattice *lattice, const QString &name=QString(),
        const prim::Layer::LayerRole &role=prim::Layer::Design);

    //! Attempt to remove a layer, by name (everything except for Result layers).
    void removeLayer(const QString &name);

    //! Attempt to remove a layer, by index (everything except for Result layers).
    void removeLayer(int n);

    //! Attempt to remove a layer, by pointer (everything except for Result layers).
    void removeLayer(prim::Layer *layer);

    //! Attempt to remove all layers (everything except for Result layers).
    void removeAllLayers();

    //! Remove all result layers.
    void removeAllResultLayers(bool keep_lattice=true);

    //! Returns a pointer to the requested layer if it exists, else 0.
    prim::Layer* getLayer(const QString &name, bool design_role=true) const;

    //! Returns a pointer to the requested layer if it exists, else 0.
    prim::Layer* getLayer(int n, bool design_role=true) const;

    //! Returns all layer pointers for the specified layer type, or an empty list
    //! if none exists.
    QList<prim::Layer*> getLayers(prim::Layer::LayerType, bool design_layers=true);

    //! Returns the number of layers in the layers stack.
    int layerCount() const {return layers.count();}

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
    prim::Layer *getMRULayer(prim::Layer::LayerType) const;

    //! Similar to getMRULayer but return the ID.
    int getMRULayerID(prim::Layer::LayerType lt) const {return indexOf(getMRULayer(lt));}

    //! Return whether the layer manager is in simlayermode
    bool isSimLayerMode() {return simlayermode;}

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

    //! Return whether this layer name exists in the provided list of layers.
    bool nameExists(const QString &nm, const QStack<prim::Layer*> &laylist);

    // return the icon corresponding to a layer type
    QIcon layerType2Icon(const prim::Layer::LayerType);


    // vars
    prim::Layer *active_layer=0;
    QStack<prim::Layer*> layers;
    QStack<prim::Layer*> simvislayers;
    QHash<prim::Layer::LayerType, prim::Layer*> mru_layers;
    bool simlayermode;
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
