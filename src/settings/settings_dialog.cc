// @file:     settings_dialog.cc
// @author:   Samuel
// @created:  2018.02.23
// @editted:  2018.02.23  - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Settings dialog for users to alter settings

#include <algorithm>
#include <QMessageBox>

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
  for (gui::PropertyForm *s_form : s_forms) {
    gui::PropertyMap changed_settings = s_form->changedProperties();
    for (gui::Property changed_prop : changed_settings) {
      settings::Settings *s_cat = settingsCategory(changed_prop.meta["category"]);
      s_cat->setValue(changed_prop.meta["key"], changed_prop.value);
    }
  }
}


// private
void SettingsDialog::initSettingsDialog()
{
  QDir s_dir(":/settings/");
  if (!s_dir.exists()) {
    qWarning() << "Settings resource directory not found.";
    return;
  }

  // construct list of all settings categories and construct the forms
  QFileInfoList s_f_infos = s_dir.entryInfoList();
  QListWidget *settings_category_list = new QListWidget(this);
  QStackedWidget *stacked_settings_panes = new QStackedWidget(this);

  for (QFileInfo s_f_info : s_f_infos) {
    QString s_path = s_f_info.absoluteFilePath();
    qDebug() << tr("Settings file %1").arg(s_path);
    gui::PropertyMap s_map(s_path);
    for (gui::Property &prop : s_map)
      setPropertyWithUserSetting(prop);
    s_forms.append(new gui::PropertyForm(s_map));

    // add to appropriate lists
    settings_category_list->addItem(s_map.map_label);
    stacked_settings_panes->addWidget(s_forms.back());
  }

  connect(settings_category_list, &QListWidget::currentRowChanged,
          stacked_settings_panes, &QStackedWidget::setCurrentIndex);

  // if the settings are always overriden (settings.h DEFAULT_OVERRIDE), then 
  // disable the settings widgets as a visual cue that the settings won't be 
  // applied.
  stacked_settings_panes->setDisabled(DEFAULT_OVERRIDE);

  // horizontal layout for list on the left and settings pane on the right
  QHBoxLayout *settings_hl = new QHBoxLayout;
  settings_hl->addWidget(settings_category_list);
  settings_hl->addWidget(stacked_settings_panes);

  // TODO bottom buttons for Apply, Confirm, Cancel
  QPushButton *pb_apply = new QPushButton("Apply");
  QPushButton *pb_apply_and_close = new QPushButton("OK");
  QPushButton *pb_cancel = new QPushButton("Cancel");
  QPushButton *pb_reset = new QPushButton("Reset");

  QHBoxLayout *bottom_buttons_hl = new QHBoxLayout;
  bottom_buttons_hl->addStretch();
  bottom_buttons_hl->addWidget(pb_apply);
  bottom_buttons_hl->addWidget(pb_apply_and_close);
  bottom_buttons_hl->addWidget(pb_cancel);
  bottom_buttons_hl->addWidget(pb_reset);

  connect(pb_apply, &QAbstractButton::clicked,
          this, &SettingsDialog::applyPendingChanges);
  connect(pb_apply_and_close, &QAbstractButton::clicked,
          [this](){
            applyPendingChanges();
            setVisible(false);
          });
  connect(pb_cancel, &QAbstractButton::clicked,
          [this](){
            // TODO reset form settings to original state
            resetForms();
            setVisible(false);
          });
  connect(pb_reset, &QAbstractButton::clicked,
          [this](){
            QMessageBox msg_reset;
            msg_reset.setText("Reset all settings?");
            msg_reset.setInformativeText("Are you sure you want to reset all \
              settings? This will take effect after restarting the application.");
            msg_reset.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            if (msg_reset.exec()) {
              emit sig_resetSettings();
            }
          });

  // main layout
  QVBoxLayout *main_layout = new QVBoxLayout;
  main_layout->addLayout(settings_hl);
  main_layout->addLayout(bottom_buttons_hl);

  setLayout(main_layout);
}

void SettingsDialog::resetForms()
{
  for (gui::PropertyForm *s_form : s_forms)
    s_form->resetFormValues();
}

void SettingsDialog::setPropertyWithUserSetting(gui::Property &t_prop) {
  if (!t_prop.meta.contains("category") || !t_prop.meta.contains("key"))
    qFatal("'key' or 'category' not found in the meta member of the given property.");

  settings::Settings *s_cat = settingsCategory(t_prop.meta["category"]);
  QVariant s_val = s_cat->get(t_prop.meta["key"]);
  QMetaType s_type = t_prop.value.metaType();
  s_val.convert(s_type);
  t_prop.value.setValue(s_val);
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
