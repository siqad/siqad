// @file:     settings_dialog.cc
// @author:   Samuel
// @created:  2018.02.23
// @editted:  2018.02.23  - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Settings dialog for users to alter settings

#include <algorithm>

#include "settings_dialog.h"
#include "../global.h"

extern QString gui::python_path;

namespace settings{

SettingsDialog::SettingsDialog(QWidget *parent)
  : QWidget(parent, Qt::Dialog)
{
  // store the QSetting singletons
  app_settings = AppSettings::instance();
  gui_settings = GUISettings::instance();
  lattice_settings = LatticeSettings::instance();

  initSettingsDialog();
}


void SettingsDialog::applyPendingChanges()
{
  // TODO specific implementation for app settings pane for now, make generic later
  gui::PropertyMap changed_settings = app_settings_pane->changedProperties();

  for (gui::Property changed_prop : changed_settings) {
    settings::Settings *s_cat = settingsCategory(changed_prop.meta["category"]);
    s_cat->setValue(changed_prop.meta["key"], changed_prop.value);
  }
}


// private
void SettingsDialog::initSettingsDialog()
{
  // all settings panes reside in a stacked widget, only one is shown at a time.
  QStackedWidget *stacked_settings_panes = new QStackedWidget(this);

  if (appSettingsPane() != nullptr)
    stacked_settings_panes->addWidget(appSettingsPane());

  if (guiSettingsPane() != nullptr)
    stacked_settings_panes->addWidget(guiSettingsPane());

  if (latticeSettingsPane() != nullptr)
    stacked_settings_panes->addWidget(latticeSettingsPane());

  // list of all categories
  QListWidget *settings_category_list = new QListWidget(this);
  settings_category_list->addItem("Application");
  //settings_category_list->addItem("Interface");
  //settings_category_list->addItem("Lattice");

  connect(settings_category_list, &QListWidget::currentRowChanged,
          stacked_settings_panes, &QStackedWidget::setCurrentIndex);

  // horizontal layout for list on the left and settings pane on the right
  QHBoxLayout *settings_hl = new QHBoxLayout;
  settings_hl->addWidget(settings_category_list);
  settings_hl->addWidget(stacked_settings_panes);

  // TODO bottom buttons for Apply, Confirm, Cancel
  QPushButton *pb_apply = new QPushButton("Apply");
  QPushButton *pb_apply_and_close = new QPushButton("OK");
  QPushButton *pb_cancel = new QPushButton("Cancel");

  QHBoxLayout *bottom_buttons_hl = new QHBoxLayout;
  bottom_buttons_hl->addStretch();
  bottom_buttons_hl->addWidget(pb_apply);
  bottom_buttons_hl->addWidget(pb_apply_and_close);
  bottom_buttons_hl->addWidget(pb_cancel);

  connect(pb_apply, &QAbstractButton::clicked,
          this, &SettingsDialog::applyPendingChanges);
  connect(pb_apply_and_close, &QAbstractButton::clicked,
          this, [this](){
            applyPendingChanges();
            setVisible(false);
          });
  connect(pb_cancel, &QAbstractButton::clicked,
          this, [this](){
            pending_changes.clear();
            setVisible(false);
          });

  // main layout
  QVBoxLayout *main_layout = new QVBoxLayout;
  main_layout->addLayout(settings_hl);
  main_layout->addLayout(bottom_buttons_hl);

  setLayout(main_layout);
}

void SettingsDialog::setPropertyWithUserSetting(gui::Property &t_prop) {
  if (!t_prop.meta.contains("category") || !t_prop.meta.contains("key"))
    qFatal("'key' or 'category' not found in the meta member of the given property.");

  settings::Settings *s_cat = settingsCategory(t_prop.meta["category"]);
  QVariant s_val = s_cat->get(t_prop.meta["key"]);
  int s_type = t_prop.value.type();
  s_val.convert(s_type);
  t_prop.value.setValue(s_val);
}

gui::PropertyForm *SettingsDialog::appSettingsPane()
{
  // return the existing settings pane if available
  if (app_settings_pane != nullptr)
    return app_settings_pane;

  // initilize the settings pane from property map
  gui::PropertyMap app_settings_map(":/settings/general.xml");
  for (gui::Property &prop : app_settings_map)
    setPropertyWithUserSetting(prop);

  app_settings_pane = new gui::PropertyForm(app_settings_map);
  return app_settings_pane;
}

gui::PropertyForm *SettingsDialog::guiSettingsPane()
{
  if (gui_settings_pane != nullptr)
    return gui_settings_pane;

  // TODO implement
  return nullptr;
}

gui::PropertyForm *SettingsDialog::latticeSettingsPane()
{
  if (lattice_settings_pane != nullptr)
    return lattice_settings_pane;

  // TODO implement
  return nullptr;
}


settings::Settings *SettingsDialog::settingsCategory(const QString &t_cat) 
{
  QMetaEnum settings_enum = QMetaEnum::fromType<SettingsCategory>();
  SettingsCategory s_cat = static_cast<SettingsCategory>(settings_enum.keyToValue(t_cat.toLatin1()));
  return settingsCategory(s_cat);
}

settings::Settings *SettingsDialog::settingsCategory(SettingsCategory cat)
{
  switch (cat) {
    case App:
      return static_cast<settings::Settings*>(app_settings);
      break;
    case GUI:
      return static_cast<settings::Settings*>(gui_settings);
      break;
    case Lattice:
      return static_cast<settings::Settings*>(lattice_settings);
      break;
    default:
      qFatal("Trying to access nonexistent setting category");
      break;
  }
  return 0;
}


} // end of settings namespace
