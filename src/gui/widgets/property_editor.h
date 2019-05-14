// @file:     property_editor.h
// @author:   Samuel
// @created:  2018.03.28
// @editted:  2018.03.28  - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Allows users to edit item properties.

#ifndef _GUI_PROPERTY_EDITOR_H_
#define _GUI_PROPERTY_EDITOR_H_

#include <QtWidgets>
#include <QtCore>

#include "../../global.h"
#include "../property_map.h"
#include "property_form.h"
#include "primitives/item.h"

namespace gui{

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

    //! Apply currently shown settings to all items in the editor.
    void applyFormsToAll();

    //! Discard a form without applying settings.
    void discardForms();


  public slots:

    //! Form cancellation
    void cancel() {discardForms(); hide();}

    //! Form OK
    void okay() {applyForms(); discardForms(); hide();}

  signals:

  protected:
    // void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    virtual void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
  private:

    //! Initialize property editor
    void initPropertyEditor();

    // VARS

    QList<QPair<PropertyForm*, prim::Item*>> form_item_pair; //! Forms that are currently in use.

    QTabWidget *form_tab_widget=0;        //! a tab widget showing all current forms
    QShortcut *shortcut_enter;

  };  // end of PropertyEditor class

} // end of gui namespace


#endif  // _GUI_PROPERTY_EDITOR_H_
