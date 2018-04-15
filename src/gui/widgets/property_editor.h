// @file:     property_editor.h
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
    void showForms(QList<prim::Item*> target_items);

    //! Apply updated settings to items currently being edited.
    void applyForms();

    //! Discard a form without applying settings.
    void discardForms();


  public slots:

    //! Form cancellation
    void cancel() {discardForms(); hide();}

    //! Form OK
    void okay() {applyForms(); discardForms(); hide();}

  signals:

  private:

    //! Initialize property editor
    void initPropertyEditor();

    // VARS

    QList<PropertyForm*> current_forms;   //! Forms that are currently in use.

    QTabWidget *form_tab_widget=0;        //! a tab widget showing all current forms


  };  // end of PropertyEditor class


  //! A user-editable form for editing item properties.
  class PropertyForm : public QWidget
  {
    Q_OBJECT

  public:

    //! Constructor
    PropertyForm(prim::Item *target_item, QWidget *parent);

    //! Destructor
    ~PropertyForm() {}

    //! Return a list of properties that have been changed. Compares the content
    //! in the form with the target_item's own property map.
    void pushPropertyChanges();

  private:
    //! Initialize the form
    void initForm();

    prim::Item *target_item;      //! Item currently being edited.
  };

} // end of gui namespace


#endif  // _GUI_PROPERTY_EDITOR_H_
