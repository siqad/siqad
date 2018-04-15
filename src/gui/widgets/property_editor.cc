// @file:     property_editor.cc
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
  : QWidget(parent, Qt::Dialog)
{
  initPropertyEditor();
}

// Generate a user-editable form for the provided property map, and connect
// appropriate signals to update the target item.
void PropertyEditor::showForms(QList<prim::Item*> target_items)
{
  for (prim::Item *item : target_items) {
    if (!item || !item->classPropertyMap()) {
      continue;
    }
    form_item_pair.append(qMakePair(new PropertyForm(item->properties(), this), item));
    form_tab_widget->addTab(form_item_pair.back().first, "TODO item class name");
  }
  show();
}


void PropertyEditor::applyForms()
{
  /*for (PropertyForm *form : form_item_pair)
    form->pushPropertyChanges();*/
  for (QPair<PropertyForm*, prim::Item*> p : form_item_pair) {
    PropertyMap final_map = p.first->finalProperties();
    prim::Item *item = p.second;

    for (const QString &key : item->properties().keys()) {
      if (item->getProperty(key).value != final_map.value(key).value) {
        item->setProperty(key, final_map.value(key).value);
      }
    }
  }
}


void PropertyEditor::discardForms()
{
  form_tab_widget->clear();
  while (!form_item_pair.isEmpty())
    delete form_item_pair.takeLast().first;
}


void PropertyEditor::initPropertyEditor()
{
  // tab for showing forms
  form_tab_widget = new QTabWidget(this);

  // editor buttons
  QHBoxLayout *buttons_hl = new QHBoxLayout;
  QPushButton *pb_apply = new QPushButton("Apply");
  QPushButton *pb_ok = new QPushButton("OK");
  QPushButton *pb_cancel = new QPushButton("Cancel");
  buttons_hl->addWidget(pb_apply);
  buttons_hl->addWidget(pb_ok);
  buttons_hl->addWidget(pb_cancel);

  // full form structure
  QVBoxLayout *editor_container = new QVBoxLayout;
  editor_container->addWidget(form_tab_widget);
  editor_container->addLayout(buttons_hl);
  setLayout(editor_container);

  // connect signals to parse form submission (generate a list of changed items)
  connect(pb_apply, &QAbstractButton::clicked,
          this, &PropertyEditor::applyForms);
  connect(pb_ok, &QAbstractButton::clicked,
          this, &PropertyEditor::okay);
  connect(pb_cancel, &QAbstractButton::clicked,
          this, &PropertyEditor::cancel);
}






// PropertyForm class

// Constructor
PropertyForm::PropertyForm(PropertyMap map, QWidget *parent)
  : QWidget(parent), map(map)
{
  initForm();
}


// Return a list of properties that have been changed
/*void PropertyForm::pushPropertyChanges()
{
  for (const QString &key : default_map->keys()) {
    Property prop = target_item->getProperty(key);
    QLineEdit *prop_field = QObject::findChild<QLineEdit*>(key);
    if (prop.value.value<QString>() != prop_field->text()) {
      target_item->setProperty(key,
          PropertyMap::string2Type2QVariant(prop_field->text(), prop.value.type())
          );
      qDebug() << tr("Changed prop %1 from %2 to %3").arg(key).arg(prop.value.toString()).arg(prop_field->text());
    }
  }
}*/


// Return a list of properties that have been changed
/*PropertyMap PropertyForm::changedProperties()
{
  PropertyMap changed_props;
  for (const QString &key : target_item->classPropertyMap()->keys()) {
    Property prop = target_item->getProperty(key);
    QLineEdit *prop_field = QObject::findChild<QLineEdit*>(key);
    if (prop.value.value<QString>() != prop_field->text()) {
      // save the property with the correct type and the rest of the attributes
      // taken from the original prop
      changed_props[key] = Property(PropertyMap::string2Type2QVariant(
          prop_field->text(), prop.value.type()), prop);
    }
  }
  return changed_props;
}*/


// Return a map of properties containing everything, changed or not
PropertyMap PropertyForm::finalProperties()
{
  for (const QString &key : map.keys()) {
    // save the property with the correct type
    QLineEdit *prop_field = QObject::findChild<QLineEdit*>(key);
    map[key].value = PropertyMap::string2Type2QVariant(prop_field->text(), map[key].value.type());
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
    QLineEdit *le_prop = new QLineEdit(prop.value.value<QString>());
    le_prop->setObjectName(it.key());
    le_prop->setToolTip(prop.form_tip);

    prop_fl->addRow(label_prop, le_prop);
  }

  setLayout(prop_fl);
}



} // end of gui namespace
