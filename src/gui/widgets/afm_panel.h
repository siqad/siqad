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
    void unsetFocusedPath() {path_focused = 0;} // TODO call this when not making path
    prim::AFMPath *focusedPath() {return path_focused;}

    // focused node (either showing properties or editing)
    void setFocusedNode(prim::AFMNode *node_fo) {node_focused = node_fo;}
    void unsetFocusedNode() {node_focused = 0;} // TODO call this when not making path
    prim::AFMNode *focusedNode() {return node_focused;}

  public slots:

    void updateFocusedToNewItem(prim::Item::ItemType, prim::Item *);

    // TODO connect to DesignPanel's sig_toolChange, call unsetFocusedPath and
    // unsetFocusedNode when the tool is changed to anything but AFM

  private:

    // VAR
    prim::AFMPath *path_focused = 0;
    prim::AFMNode *node_focused = 0;


  };

} // end of gui namespace

#endif
