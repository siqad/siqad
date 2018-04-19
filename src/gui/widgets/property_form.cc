// @file:     property_form.cc
// @author:   Samuel
// @created:  2018.04.15
// @editted:  2018.04.15  - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Provides a standard QWidget arranging properties into a form for
//            users to edit.

#include "property_form.h"

namespace gui{

// Constructor
PropertyForm::PropertyForm(PropertyMap map, QWidget *parent)
  : QWidget(parent), map(map)
{
  initForm();
}


// Return a map of properties containing everything, changed or not
PropertyMap PropertyForm::finalProperties()
{
  for (const QString &key : map.keys()) {
    // save the property with the correct type
    if (map[key].value_selection.type == Combo) {
      QComboBox *prop_combo = QObject::findChild<QComboBox*>(key);
      map[key].value = prop_combo->currentData();
    } else {
      // the field is a line edit if no value selection has been set
      QLineEdit *prop_field = QObject::findChild<QLineEdit*>(key);
      map[key].value = PropertyMap::string2Type2QVariant(prop_field->text(), map[key].value.type());
    }
  }
  return map;
}


// Initialize the form
void PropertyForm::initForm()
{
  setWindowTitle("Property Editor");

  // generate form from map
  QFormLayout *prop_fl = new QFormLayout;
  for (PropertyMap::const_iterator it = map.cbegin(), end = map.cend(); it != end; ++it) {
    Property prop = it.value();

    QLabel *label_prop = new QLabel(prop.form_label);

    QWidget *prop_val_widget;
    if (prop.value_selection.type == Combo) {
      // if there are combo options, treat this as a combo box
      prop_val_widget = new QComboBox;
      for (ComboOption combo_option : prop.value_selection.combo_options) {
        static_cast<QComboBox*>(prop_val_widget)->addItem(combo_option.label, combo_option.val);
      }
    } else {
      prop_val_widget = new QLineEdit(prop.value.value<QString>());
    }
    prop_val_widget->setObjectName(it.key());
    prop_val_widget->setToolTip(prop.form_tip);
    prop_fl->addRow(label_prop, prop_val_widget);
  }

  setLayout(prop_fl);
}



} // end of gui namespace
