// @file:     color_dialog.h
// @author:   Nathan
// @created:  2019.05.28
// @editted:  2019.05.28 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     A dialog inheriting QColorDialog for selecting colors


#ifndef _GUI_COLOR_DIALOG_H_
#define _GUI_COLOR_DIALOG_H_

#include <QColor>
#include <QtWidgets>

#include "primitives/items.h"


namespace gui{

  class ColorDialog : public QColorDialog
  {
    Q_OBJECT

    public:

      // constructor
      ColorDialog(QWidget *parent = 0);

      // destructor
      ~ColorDialog();


      void show(prim::Item *item);

      QList<prim::Item*> getTargetItems(){return target_items;}

      void clearItems(){target_items.clear();}
    private:
      QList<prim::Item*> target_items;
  };

} // end gui namespace


#endif
