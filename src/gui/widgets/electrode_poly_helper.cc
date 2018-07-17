// @file:     electrode_poly_helper.cc
// @author:   Nathan
// @created:  2018.07.17
// @editted:  2018.07.17 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     Widget to create and edit polygonal electrodes

#include "electrode_poly_helper.h"

namespace gui{

ElectrodePolyHelper::ElectrodePolyHelper(QWidget *parent)
  : QWidget(parent)
{
  // ghost_node = new prim::AFMNode(active_afm_layer_index, QPointF(0,0), 0);
  // ghost_seg = new prim::AFMSeg(active_afm_layer_index, 0, ghost_node);
  // showGhost(false);

  // TODO global tool enum, connect toolchange to panel to determine afm ghost visibility

  // TODO init GUI

  qDebug() << "EPH created";

      //connect(this, &gui::DesignPanel::sig_toolChanged, this, &gui::DesignPanel::afmGhostVisibilityWithTool);

    /*if (!ghost_afm_node) {
      ghost_afm_node = new prim::AFMNode(getLayerIndex(afm_layer), QPointF(0,0), afm_layer->zOffset());
      scene->addItem(ghost_afm_node);
    }
    if (!ghost_afm_seg) {
      ghost_afm_seg = new prim::AFMSeg(getLayerIndex(afm_layer), 0, ghost_afm_node);
      scene->addItem(ghost_afm_seg);
    }*/
}

void ElectrodePolyHelper::addPoint(QPointF point)
{
  points.append(point);
}

void ElectrodePolyHelper::clearPoint()
{
  points.clear();
}


// void AFMPanel::setFocusedPath(prim::AFMPath *path_fo)
// {
//   path_focused = path_fo;
//   if (path_fo != 0 && path_fo->nodeCount() > 0)
//     setFocusedNodeIndex(path_focused->getLastNodeIndex());
//   else
//     setFocusedNodeIndex(-1);
// }
//
// void AFMPanel::setFocusedNodeIndex(int node_ind)
// {
//   node_index_focused = node_ind;
//
//   // generate ghost segment if a focused node exists
//   if (focusedPath() && node_ind > -1)
//     ghost_seg->setOriginNode(path_focused->getNode(node_ind));
//   else
//     ghost_seg->setOriginNode(0);
// }
//
// void AFMPanel::showGhost(bool show)
// {
//   // alwayd show node ghost at cursor position
//   ghost_node->setVisible(show);
//
//   // only show seg ghost if there will be a segment connection
//   if (node_index_focused > -1) {
//     ghost_seg->updatePoints();
//     ghost_seg->setVisible(show);
//   } else {
//     ghost_seg->setVisible(false);
//   }
// }
//
//
// void AFMPanel::toolChangeResponse(gui::ToolType tool_type)
// {
//   if (tool_type != AFMPathTool)
//     showGhost(false);
// }
//
//
// // SLOTS
// /* TODO rm
// void AFMPanel::updateFocusToNewItem(prim::Item::ItemType item_type, prim::Item *new_item)
// {
//   switch (item_type) {
//   case prim::Item::AFMNode:
//     {
//       setFocusedPath(static_cast<prim::AFMPath*>(new_item->parentItem()));
//       setFocusedNode(static_cast<prim::AFMNode*>(new_item));
//       break;
//     }
//   case prim::Item::AFMPath:
//     {
//       prim::AFMPath *new_path = static_cast<prim::AFMPath*>(new_item);
//       setFocusedPath(new_path);
//       //setFocusedNode(new_path->getLastNode());
//       break;
//     }
//   default:
//     break;
//   }
// }*/
//
// /* TODO rm
// void AFMPanel::updateFocusAfterItemRemoval(prim::Item::ItemType item_type, prim::Item *rm_item)
// {
//   switch (item_type) {
//   case prim::Item::AFMNode:
//     {
//       if (rm_item == focusedNode()) {
//         if (focusedPath()) {
//           // TODO update to select the node before the originally focused node
//           qDebug() << QObject::tr("Updating focused node after item removal");
//           setFocusedNode(focusedPath()->getLastNode());
//         } else {
//           setFocusedNode(0);
//         }
//       }
//       break;
//     }
//   case prim::Item::AFMPath:
//     {
//       setFocusedPath(0);
//       setFocusedNode(0);
//       break;
//     }
//   default:
//     break;
//   }
// }*/


} // end of gui namespace
