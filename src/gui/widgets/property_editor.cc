// @file:     property_map.cc
// @author:   Samuel
// @created:  2018.03.29
// @editted:  2018.03.29  - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Provides a standard QWidget arranging properties as a form for
//            users to edit.

#include "property_editor.h"

namespace gui{

// Constructor
PropertyEditor::PropertyEditor(QWidget *parent)
  : QWidget(parent)
{

}

// Generate a user-editable form for the provided property map, and connect
// appropriate signals to update the target item.
QWidget *PropertyEditor::generateForm(gui::PropertyMap *map,
    prim::Item *target_item)
{
  // TODO set the new form's parent as PropertyEditor

  // TODO generate form from map

  // TODO connect signals to parse form submission (generate a list of changed items)

  // TODO connect signals to push the changed stuff to the item
}




// PropertyForm class

// Constructor
PropertyForm::PropertyForm(gui::PropertyMap *map, prim::Item *target_item,
    QWidget *parent)
  : QWidget(parent), target_map(map), target_item(target_item)
{
  initForm();
}


// Return a list of properties that have been changed
gui::PropertyMap PropertyForm::pushPropertyChanges()
{
  QMapIterator<QString, gui::Property> prop_it(*target_map);
  while (prop_it.hasNext()) {
    prop_it.next();
    gui::Property prop = prop_it.value();

    QLineEdit *prop_field = QObject::findChild<QLineEdit *>(prop_it.key());
    // update the item's property value if changes have been made
    if (prop.value<QString>() != prop_field->text()) {
      target_item->setProperty(prop_it.key(),
          gui::PropertyMap::string2Type2QVariant(prop_field->text()));
    }
  }
}



// Initialize the form
void PropertyForm::initForm()
{
  QMapIterator<QString, gui::Property> prop_it(*target_map);
  QVBoxLayout vl_props = new QVBoxLayout;
  while (prop_it.hasNext()) {
    prop_it.next();
    gui::Property prop = prop_it.value();

    QLabel *label_prop = new QLabel(prop.form_label);
    QLineEdit *le_prop = new QLineEdit(prop.value<QString>());
    le_prop->setObjectName(prop_it.key());

    QHBoxLayout *hl_prop = new QHBoxLayout;
    hl_prop->addWidget(label_prop);
    hl_prop->addWidget(le_prop);

    vl_props->addLayout(hl_prop);
  }
  // TODO apply / revert / cancel buttons

  // TODO signals

  setLayout(vl_props);
}



} // end of gui namespace
