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
    ~LayerEditor() {};

    void updateLayerList();

  private:
    void initLayerEditor();

    // vars
    gui::DesignPanel *dp;
    QStack<prim::Layer*> *layers;

    // GUI
    QGridLayout *layer_list_vl;
  };

}


#endif
