// @file:     afm_panel.h
// @author:   Samuel
// @created:  2018.01.18
// @editted:  2018.01.18 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Widget for editing AFM Path properties and adding/removing nodes

#ifndef _GUI_AFM_PANEL_H_
#define _GUI_AFM_PANEL_H_

// TODO includes
#include "primitives/afmpath.h"

namespace gui{

  class AFMPanel : public QWidget
  {
    Q_OBJECT

  public:

    // constructor
    // TODO change active_afm_layer_index to pointer to layer manager
    AFMPanel(int active_afm_layer_index, QWidget *parent = 0);

    // destructor
    ~AFMPanel() {};


    // focused path (either showing properties or editing)
    prim::AFMPath *focusedPath() {return path_focused;}

    // focused node index in the path's node list (either showing properties or editing)
    int focusedNodeIndex() {return node_index_focused;}

    // ghost node for indicating where the next AFMNode will be placed
    prim::AFMNode *ghostNode() {return ghost_node;}
    prim::AFMSeg *ghostSegment() {return ghost_seg;}

    // show or hide the ghost
    void showGhost(bool);

  public slots:

    // slots for setting focused path and node index
    void setFocusedPath(prim::AFMPath *path_fo);
    void setFocusedNodeIndex(int node_ind);

    // tool change actions
    // TODO void toolChangeActions(global::ToolType tool_type);

    // TODO connect to layer manager's layer change signal, change ghost properties accordingly

    // TODO rm
    //void updateFocusToNewItem(prim::Item::ItemType, prim::Item *);
    //void updateFocusAfterItemRemoval(prim::Item::ItemType, prim::Item *);

    // TODO connect to DesignPanel's sig_toolChange, call unsetFocusedPath and
    // unsetFocusedNode when the tool is changed to anything but AFM

  private:

    // VAR
    prim::AFMPath *path_focused = 0;  // AFMPath currently being manipulated
    int node_index_focused = -1;  // index of AFMNode currently being manipulated, -1 means there's no focused node

    prim::AFMNode *ghost_node = 0;    // Ghost AFMNode for showing where the next AFMNode will go
    prim::AFMSeg  *ghost_seg = 0;     // Ghost AFMSeg for showing where the next AFMSeg will go


  };

} // end of gui namespace

#endif
