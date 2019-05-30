// @file:     rotate_dialog.cc
// @author:   Nathan
// @created:  2019.05.28
// @editted:  2019.05.28  - Nathan
// @license:  GNU LGPL v3
//
// @desc:     RotateDialog definitions

#include "rotate_dialog.h"

namespace gui{

RotateDialog::RotateDialog(QWidget *parent)
  : QInputDialog(parent)
{
  setInputMode(QInputDialog::DoubleInput);
  setWindowTitle(QObject::tr("Set rotation"));
  setLabelText(QObject::tr("Rotation angle in degrees"));
  setDoubleRange(-10000, 10000);
}

RotateDialog::~RotateDialog()
{}

void RotateDialog::show(prim::Item *item)
{
  if (!target_items.contains(item))
    target_items.append(item);
  setDoubleValue(static_cast<prim::Electrode *>(item)->getAngleDegrees());
  QInputDialog::show();
}

}
