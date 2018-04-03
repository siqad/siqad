// @file:     property_map.h
// @author:   Samuel
// @created:  2018.03.28
// @editted:  2018.03.28  - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Provides a standard QWidget arranging properties as a form for
//            users to edit.

#ifndef _GUI_PROPERTY_EDITOR_H_
#define _GUI_PROPERTY_EDITOR_H_

#include <QtWidgets>
#include <QtCore>

#include "primitives/item.h"
#include "../../global.h"
#include "../property_map.h"

namespace gui{

  // Forward declaration of property form
  class PropertyForm;

  //! The property editor generates forms from provided PropertyMaps which
  //! allows users to edit properties of supported sub-classes of prim::Item.
  //! In the future, after editable forms are generated, they will be presented
  //! inside a tabbed interface which allows users to edit multiple items at
  //! once. The content of the forms will be parsed by the PropertyEditor class
  //! and written to the associated items.
  class PropertyEditor : public QWidget
  {
    Q_OBJECT

  public:

    //! Constructor
    PropertyEditor(QWidget *parent=0);

    //! Destructor
    ~PropertyEditor() {}

    //! Generate a user-editable form with the provided property map. Upon
    //! submission of changes, the target item will be updated.
    QWidget *generateForm(gui::PropertyMap *map, prim::Item *target_item);

  public slots:

  signals:

  private:

  };  // end of PropertyEditor class


  //! A user-editable form for editing item properties.
  class PropertyForm : public QWidget
  {
    Q_OBJECT

  public:

    //! Constructor
    PropertyForm(gui::PropertyMap *map, prim::Item *target_item, QWidget *parent);

    //! Destructor
    ~PropertyForm() {}

    //! Return a list of properties that have been changed. Compares the content
    //! in the form with the target_item's own property map.
    void pushPropertyChanges();

  private:
    //! Initialize the form
    void initForm();

    gui::PropertyMap *target_map; //! Property map of the target item, should
                                  //! already have merged default and changed.
    prim::Item *target_item;      //! Item currently being edited.
  };

} // end of gui namespace


#endif  // _GUI_PROPERTY_EDITOR_H_
