// @file:     property_form.cc
// @author:   Samuel
// @created:  2018.04.15
// @editted:  2018.04.15  - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Provides a standard QWidget arranging properties into a form for
//            users to edit.

#include "property_form.h"
#include "settings/settings.h"

namespace gui{

// Constructor
PropertyForm::PropertyForm(PropertyMap t_map, QWidget *parent)
  : QWidget(parent), orig_map(t_map)
{
  prop_fl = new QFormLayout;
  initForm();
  setLayout(prop_fl);
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


void PropertyForm::resetFormValues()
{
  // delete existing form entries
  if (prop_fl != nullptr)
    while (!prop_fl->isEmpty())
      prop_fl->removeRow(0);

  initForm();
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
      if (!prop_field) {
        qWarning() << tr("Cannot find property field with key %1").arg(key);
      }
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

  // make a map with the sort index as the key
  QMap<int, QPair<QWidget*, QWidget*>> form_map;

  // generate form from property map
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
      {
        if (prop.dp != -1 && static_cast<QMetaType::Type>(prop.value.type()) == QMetaType::Float) {
          prop_val_widget = new QLineEdit(QString::number(prop.value.value<float>(), 'f', prop.dp));
        } else {
          prop_val_widget = new QLineEdit(prop.value.value<QString>());
        }
        prop_val_widget->setMinimumWidth(settings::GUISettings::instance()->get<int>("SIMMAN/mw"));
        break;
      }
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

    form_map.insert(prop.index, qMakePair(label_prop, prop_val_widget));
  }

  // populate the form layout with contents from the form map
  for (auto it = form_map.begin(); it != form_map.end(); it++)
    prop_fl->addRow(it.value().first, it.value().second);
}



} // end of gui namespace
