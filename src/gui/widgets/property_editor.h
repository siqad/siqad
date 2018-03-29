// @file:     property_map.h
// @author:   Samuel
// @created:  2018.03.22
// @editted:  2018.03.28  - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Provides a standard QWidget arranging properties as a form for
//            users to edit.

#ifndef _GUI_PROPERTY_EDITOR_H_
#define _GUI_PROPERTY_EDITOR_H_

#include <QtWidgets>
#include <QtCore>

#include "../../global.h"
#include "../property_map.h"

namespace gui{

  class PropertyEditor : public QWidget
  {
    Q_OBJECT

  public:

    //! Constructor taking the pointer of the property map of interest.
    PropertyEditor(gui::PropertyMap *map, QWidget *parent);

    //! Destructor
    ~PropertyEditor();

    void initPropertyEditor();

  public slots:

  private:

  };  // end of PropertyEditor class

} // end of gui namespace


#endif  // _GUI_PROPERTY_EDITOR_H_
