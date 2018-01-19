// @file:     afm_manager.h
// @author:   Samuel
// @created:  2018.01.18
// @editted:  2018.01.18 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Widget for editing AFM Path properties and adding/removing nodes

#ifndef _GUI_AFM_PANEL_H_
#define _GUI_AFM_PANEL_H_

// TODO includes

namespace gui{

  class AFMPanel : public QWidget
  {
    Q_OBJECT

  public:

    // constructor
    AFMManager(QWidget *parent = 0);

    // destructor
    ~AFMManager() {};


    // focused path (either showing properties or editing)
    void setFocusedPath(AFMPath *path_fo) {path_focused = path_fo;}
    void unsetFocusedPath() {path_focused = 0;} // TODO call this when not making path
    AFMPath *getFocusedPath() {return path_focused;}

    // focused node (either showing properties or editing)
    void setFocusedNode(AFMNode *node_fo) {node_focused = node_fo;}
    void unsetFocusedNode() {node_focused = 0;} // TODO call this when not making path
    AFMNode *getFocusedNode() {return node_focused;}

  public slot:

    // TODO connect to DesignPanel's sig_toolChange, call unsetFocusedPath and
    // unsetFocusedNode when the tool is changed to anything but AFM

  private:

    // VAR
    AFMPath *path_focused;
    AFMNode *node_focused;


  };

} // end of gui namespace

#endif
