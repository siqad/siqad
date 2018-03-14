// @file:     settings_dialog.cc
// @author:   Samuel
// @created:  2018.02.23
// @editted:  2018.02.23  - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Settings dialog for users to alter settings

#include "settings_dialog.h"

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


// public slots
void SettingsDialog::addPendingBoolUpdate(bool new_state)
{
  QObject *sender = const_cast<QObject*>(QObject::sender());

  SettingsCategory category;
  QString name;

  QStringList splitted_name = sender->objectName().split(':');
  if (splitted_name.at(0) == "app")
    category = App;
  else if (splitted_name.at(0) == "gui")
    category = GUI;
  else if (splitted_name.at(0) == "lattice")
    category = Lattice;
  else
    qFatal("Specified a settings category that doesn't exist");

  name = splitted_name.at(1);

  pending_changes.append(
    PendingChange(category, name, QVariant(new_state))
  );

  /*pending_changes.append(
    PendingChange(SettingsCategory::App, sender->objectName(),
        QVariant(new_state))
  );*/
  //app_settings->setValue(sender->objectName(), QVariant(new_state));
  qDebug() << "Changed setting!";
}


void SettingsDialog::applyPendingChanges()
{
  // TODO deduplicate

  for (PendingChange pending_change : pending_changes) {
    settings::Settings *category_setting = settingsCategoryPointer(
        pending_change.category);
    category_setting->setValue(pending_change.name, pending_change.value);
  }

  pending_changes.clear();
}


void SettingsDialog::applyAndClose()
{
  applyPendingChanges();
  setVisible(false);
}


void SettingsDialog::discardAndClose()
{
  pending_changes.clear();
  setVisible(false);
}


// private
void SettingsDialog::initSettingsDialog()
{
  // all settings panes in a stacked widget, only one is shown at a time
  QStackedWidget *stacked_settings_panes = new QStackedWidget(this);
  stacked_settings_panes->addWidget(appSettingsPane());
  //stacked_settings_panes->addWidget(guiSettingsPane());
  //stacked_settings_panes->addWidget(latticeSettingsPane());

  // list of all categories
  QListWidget *settings_category_list = new QListWidget(this);
  settings_category_list->addItem("Application");
  //settings_category_list->addItem("Interface");
  //settings_category_list->addItem("Lattice");

  connect(settings_category_list, SIGNAL(currentRowChanged(int)),
          stacked_settings_panes, SLOT(setCurrentIndex(int)));

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

  connect(pb_apply, SIGNAL(clicked(bool)),
          this, SLOT(applyPendingChanges()));
  connect(pb_apply_and_close, SIGNAL(clicked(bool)),
          this, SLOT(applyAndClose()));
  connect(pb_cancel, SIGNAL(clicked(bool)),
          this, SLOT(discardAndClose()));

  // wrap everything together in a neat layout
  QVBoxLayout *main_layout = new QVBoxLayout;
  main_layout->addLayout(settings_hl);
  main_layout->addLayout(bottom_buttons_hl);

  setLayout(main_layout);
}

QWidget *SettingsDialog::appSettingsPane()
{
  if (app_settings_pane)
    return app_settings_pane;

  QLabel *label_hidpi = new QLabel(QObject::tr("HiDPI Mode*"));
  QLabel *label_show_debug_output = new QLabel(QObject::tr("Show debug messages*"));
  QLabel *label_req_restart = new QLabel(QObject::tr("Settings with the * indicator only take effect after restart."));

  QCheckBox *cb_hidpi = new QCheckBox(QObject::tr("Enabled"));
  QCheckBox *cb_show_debug_output = new QCheckBox(QObject::tr("Enabled"));

  cb_hidpi->setObjectName("app:view/hidpi_support");
  cb_hidpi->setChecked(app_settings->get<bool>("view/hidpi_support"));
  cb_show_debug_output->setObjectName("app:log/override");
  cb_show_debug_output->setChecked(app_settings->get<bool>("log/override"));

  connect(cb_hidpi, SIGNAL(toggled(bool)),
          this, SLOT(addPendingBoolUpdate(bool)));
  connect(cb_show_debug_output, SIGNAL(toggled(bool)),
          this, SLOT(addPendingBoolUpdate(bool)));

  QHBoxLayout *hidpi_hl = new QHBoxLayout;
  hidpi_hl->addWidget(label_hidpi);
  hidpi_hl->addWidget(cb_hidpi);

  QHBoxLayout *show_debug_output_hl = new QHBoxLayout;
  show_debug_output_hl->addWidget(label_show_debug_output);
  show_debug_output_hl->addWidget(cb_show_debug_output);

  QVBoxLayout *app_settings_pane_vl = new QVBoxLayout;
  app_settings_pane_vl->addLayout(hidpi_hl);
  app_settings_pane_vl->addLayout(show_debug_output_hl);
  app_settings_pane_vl->addWidget(label_req_restart);

  app_settings_pane = new QWidget(this);
  app_settings_pane->setLayout(app_settings_pane_vl);
  return app_settings_pane;
}

QWidget *SettingsDialog::guiSettingsPane()
{
  if (gui_settings_pane)
    return gui_settings_pane;

  // TODO implement
}

QWidget *SettingsDialog::latticeSettingsPane()
{
  if (lattice_settings_pane)
    return lattice_settings_pane;

  // TODO implement
}


settings::Settings *SettingsDialog::settingsCategoryPointer(SettingsCategory cat)
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
