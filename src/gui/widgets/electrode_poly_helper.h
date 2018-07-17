// @file:     electrode_poly_helper.h
// @author:   Nathan
// @created:  2018.07.17
// @editted:  2018.07.17 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     Widget to create and edit polygonal electrodes

#ifndef _GUI_ELECTRODE_POLY_HELPER_H_
#define _GUI_ELECTRODE_POLY_HELPER_H_

// TODO includes
#include "../../global.h"
#include "primitives/electrode_poly.h"

namespace gui{

  class ElectrodePolyHelper : public QWidget
  {
    Q_OBJECT

  public:

    // constructor
    // TODO change active_afm_layer_index to pointer to layer manager
    ElectrodePolyHelper(QWidget *parent = 0);

    // destructor
    ~ElectrodePolyHelper() {};

    void addPoint(QPointF point);
    void clearPoints();
    QList<QPointF> getPoints(){return points;}

    // focused path (either showing properties or editing)
    // prim::AFMPath *focusedPath() {return path_focused;}

    // focused node index in the path's node list (either showing properties or editing)
    // int focusedNodeIndex() {return node_index_focused;}

    // ghost node for indicating where the next AFMNode will be placed
    // prim::AFMNode *ghostNode() {return ghost_node;}
    // prim::AFMSeg *ghostSegment() {return ghost_seg;}

    // show or hide the ghost
    // void showGhost(bool);

  // public slots:

    // slots for setting focused path and node index
    // void setFocusedPath(prim::AFMPath *path_fo);
    // void setFocusedNodeIndex(int node_ind);

    // tool change actions
    // void toolChangeResponse(gui::ToolType tool_type);

    // TODO connect to layer manager's layer change signal, change ghost properties accordingly

    // TODO rm
    //void updateFocusToNewItem(prim::Item::ItemType, prim::Item *);
    //void updateFocusAfterItemRemoval(prim::Item::ItemType, prim::Item *);

    // TODO connect to DesignPanel's sig_toolChange, call unsetFocusedPath and
    // unsetFocusedNode when the tool is changed to anything but AFM

  private:

    // VAR
    QList<QPointF> points;
    // prim::AFMPath *path_focused = 0;  // AFMPath currently being manipulated
    // int node_index_focused = -1;  // index of AFMNode currently being manipulated, -1 means there's no focused node

    // prim::AFMNode *ghost_node = 0;    // Ghost AFMNode for showing where the next AFMNode will go
    // prim::AFMSeg  *ghost_seg = 0;     // Ghost AFMSeg for showing where the next AFMSeg will go


  };

} // end of gui namespace

#endif
