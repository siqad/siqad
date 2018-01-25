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
    AFMPanel(QWidget *parent = 0);

    // destructor
    ~AFMPanel() {};


    // focused path (either showing properties or editing)
    void setFocusedPath(prim::AFMPath *path_fo);
    prim::AFMPath *focusedPath() {return path_focused;}

    // focused node (either showing properties or editing)
    void setFocusedNode(prim::AFMNode *node_fo) {node_focused = node_fo;}
    prim::AFMNode *focusedNode() {return node_focused;}

    // ghost node for indicating where the next AFMNode will be placed TODO remove
    //prim::AFMNode *ghostNode() {return node_ghost;}

    // show or hide the ghost TODO remove
    //void showGhost(bool);

  public slots:

    void updateFocusedToNewItem(prim::Item::ItemType, prim::Item *);

    // TODO connect to DesignPanel's sig_toolChange, call unsetFocusedPath and
    // unsetFocusedNode when the tool is changed to anything but AFM

  private:

    // VAR
    prim::AFMPath *path_focused = 0;  // AFMPath currently being manipulated
    prim::AFMNode *node_focused = 0;  // AFMNode currently being manipulated

    /* TODO remove
    prim::AFMNode *node_ghost = 0;    // Ghost AFMNode for showing where the next AFMNode will go
    prim::AFMSeg  *seg_ghost = 0;     // Ghost AFMSeg for showing where the next AFMSeg will go*/


  };

} // end of gui namespace

#endif
