// @file:     info_panel.cc
// @author:   Jake
// @created:  2016.11.02
// @editted:  2017.05.11  - Jake
// @license:  GNU LGPL v3
//
// @desc:     InfoPanel definitions

#include "info_panel.h"

namespace gui{

InfoPanel::InfoPanel(QWidget *parent)
  : QWidget(parent)
{}

InfoPanel::~InfoPanel()
{}

void InfoPanel::initInfoPanel()
{
  QLabel l_cursor_coords = tr("Cursor coordinates");

}

}
