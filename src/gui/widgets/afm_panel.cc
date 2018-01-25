// @file:     afm_panel.cc
// @author:   Samuel
// @created:  2018.01.22
// @editted:  2018.01.22 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Widget for editing AFM Path properties and adding/removing nodes

#include "afm_panel.h"

namespace gui{

AFMPanel::AFMPanel(QWidget *parent)
  : QWidget(parent)
{
  // TODO init GUI
}

void AFMPanel::setFocusedPath(prim::AFMPath *path_fo)
{
  path_focused = path_fo;
  if (path_fo->nodeCount() > 0)
    setFocusedNode(path_focused->getLastNode());
  else
    setFocusedNode(0);
}

void AFMPanel::setFocusedNode(prim::AFMNode *node_fo)
{
  node_focused = node_fo;
  //seg_ghost->setOrigin(node_focused);
}

/* TODO remove
void showGhost(bool show)
{
  // alwayd show node ghost at cursor position
  node_ghost->setVisible(show);

  // only show seg ghost if there will be a segment connection
  if (node_focused)
    seg_ghost->setVisible(show);
}*/


// SLOTS
void AFMPanel::updateFocusedToNewItem(prim::Item::ItemType item_type, prim::Item *new_item)
{
  if (item_type == prim::Item::AFMNode) {
    setFocusedPath(static_cast<prim::AFMPath*>(new_item->parentItem()));
    setFocusedNode(static_cast<prim::AFMNode*>(new_item));
  } else if (item_type == prim::Item::AFMPath) {
    prim::AFMPath *new_path = static_cast<prim::AFMPath*>(new_item);
    setFocusedPath(new_path);
    //setFocusedNode(new_path->getLastNode());
  }
}


} // end of gui namespace
