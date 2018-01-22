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
  setFocusedNode(path_focused->getLastNode());
}


} // end of gui namespace
