// @file:     rotate_dialog.h
// @author:   Nathan
// @created:  2019.05.28
// @editted:  2019.05.28 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     A dialog inheriting QInputDialog for rotating items


#ifndef _GUI_ROTATE_DIALOG_H_
#define _GUI_ROTATE_DIALOG_H_

#include <QtWidgets>

#include "primitives/items.h"


namespace gui{

  class RotateDialog : public QInputDialog
  {
    Q_OBJECT

    public:

      // constructor
      RotateDialog(QWidget *parent = 0);

      // destructor
      ~RotateDialog();

      void show(prim::Item *item);

      QList<prim::Item*> getTargetItems(){return target_items;}

      void clearItems(){target_items.clear();}
    private:
      QList<prim::Item*> target_items;
  };

} // end gui namespace


#endif
