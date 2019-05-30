// @file:     color_dialog.cc
// @author:   Nathan
// @created:  2019.05.28
// @editted:  2019.05.28  - Nathan
// @license:  GNU LGPL v3
//
// @desc:     ColorDialog definitions

#include "color_dialog.h"

namespace gui{

ColorDialog::ColorDialog(QWidget *parent)
  : QColorDialog(parent)
{
  setOption(QColorDialog::ShowAlphaChannel,true);
  setOption(QColorDialog::DontUseNativeDialog,true);
  setCurrentColor(Qt::white);
}

ColorDialog::~ColorDialog()
{}

void ColorDialog::show(prim::Item *item)
{
  if (!target_items.contains(item))
    target_items.append(item);

  QColorDialog::show();
}

}
