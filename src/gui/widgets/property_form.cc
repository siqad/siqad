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
PropertyForm::PropertyForm(PropertyMap t_map, QWidget *parent)
  : QWidget(parent), orig_map(t_map)
{
  initForm();
}


PropertyMap PropertyForm::finalProperties()
{
  PropertyMap final_map(orig_map);
  PropertyMap::iterator it;

  // save the values in the form (edited or not) to the final property map
  for (it = final_map.begin(); it != final_map.end(); ++it)
    it.value().value.setValue(formValue(it.key()));

  return final_map;
}


PropertyMap PropertyForm::changedProperties()
{
  PropertyMap map_changed;

  for (const QString &key : orig_map.keys()) {
    // save changed properties to props_changed
    QVariant form_val = formValue(key);
    if (orig_map[key].value != form_val)
      map_changed.insert(key, Property(form_val, orig_map[key]));
  }
  return map_changed;
}


QVariant PropertyForm::formValue(const QString &key)
{
  QVariant new_val;
  switch (orig_map[key].value_selection.type) {
    case CheckBox:
      new_val = QObject::findChild<QCheckBox*>(key)->isChecked();
      break;
    case Combo:
      new_val = QObject::findChild<QComboBox*>(key)->currentData();
      break;
    case LineEdit:
    {
      QLineEdit *prop_field = QObject::findChild<QLineEdit*>(key);
      new_val = PropertyMap::string2Type2QVariant(prop_field->text(),
                                                  orig_map[key].value.type());
      break;
    }
    default:
      break;
  }
  return new_val;
}


void PropertyForm::initForm()
{
  setWindowTitle("Property Editor");

  // populate a form with the desired number of null widgets first
  QFormLayout *prop_fl = new QFormLayout;
  for (int i=0; i<orig_map.size(); i++)
    prop_fl->addRow(new QWidget, new QWidget);

  // generate form from map
  PropertyMap::const_iterator it;
  for (it = orig_map.cbegin(); it != orig_map.cend(); ++it) {
    Property prop = it.value();

    QLabel *label_prop = new QLabel(prop.form_label);

    QWidget *prop_val_widget;
    switch (prop.value_selection.type) {
      case Combo:
      {
        prop_val_widget = new QComboBox;
        for (ComboOption combo_option : prop.value_selection.combo_options) {
          static_cast<QComboBox*>(prop_val_widget)->addItem(combo_option.label, 
                                                            combo_option.val);
        }
        int select_ind = static_cast<QComboBox*>(prop_val_widget)->findData(prop.value);
        if (select_ind != -1) {
          static_cast<QComboBox*>(prop_val_widget)->setCurrentIndex(select_ind);
        }
        break;
      }
      case LineEdit:
        prop_val_widget = new QLineEdit(prop.value.value<QString>());
        break;
      case CheckBox:
        prop_val_widget = new QCheckBox();
        static_cast<QCheckBox*>(prop_val_widget)->setChecked(prop.value.toBool());
        break;
      default:
        qFatal("Unrecognized property option.");
        break;
    }

    prop_val_widget->setObjectName(it.key());
    prop_val_widget->setToolTip(prop.form_tip);

    // delete the null widget at the target row and insert the new row
    prop_fl->removeRow(prop.index);
    prop_fl->insertRow(prop.index, label_prop, prop_val_widget);
  }

  setLayout(prop_fl);
}



} // end of gui namespace
