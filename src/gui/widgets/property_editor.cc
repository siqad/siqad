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
    current_forms.append(new PropertyForm(item, this));
    form_tab_widget->addTab(current_forms.back(), "TODO item class name");
  }
  show();
}


void PropertyEditor::applyForms()
{
  for (PropertyForm *form : current_forms)
    form->pushPropertyChanges();
}


void PropertyEditor::discardForms()
{
  form_tab_widget->clear();
  while (!current_forms.isEmpty())
    delete current_forms.takeLast();
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
PropertyForm::PropertyForm(prim::Item *target_item,
    QWidget *parent)
  : QWidget(parent), target_item(target_item)
{
  initForm();
}


// Return a list of properties that have been changed
void PropertyForm::pushPropertyChanges()
{
  for (const QString &key : target_item->classPropertyMap()->keys()) {
    Property prop = target_item->getProperty(key);
    QLineEdit *prop_field = QObject::findChild<QLineEdit*>(key);
    if (prop.value.value<QString>() != prop_field->text()) {
      target_item->setProperty(key,
          PropertyMap::string2Type2QVariant(prop_field->text(), prop.value.type())
          );
      qDebug() << tr("Changed prop %1 from %2 to %3").arg(key).arg(prop.value.toString()).arg(prop_field->text());
    }
  }
}



// Initialize the form
void PropertyForm::initForm()
{
  PropertyMap *map = target_item->classPropertyMap();
  setWindowTitle("Property Editor");

  // generate form from map
  QFormLayout *prop_fl = new QFormLayout;
  for (const QString &key : map->keys()) {
    // get the property from the item so the local properties would be shown instead if available
    gui::Property prop = target_item->getProperty(key);

    QLabel *label_prop = new QLabel(prop.form_label);
    QLineEdit *le_prop = new QLineEdit(prop.value.value<QString>());
    le_prop->setObjectName(key);
    le_prop->setToolTip(prop.form_tip);

    prop_fl->addRow(label_prop, le_prop);
  }

  setLayout(prop_fl);
}



} // end of gui namespace
