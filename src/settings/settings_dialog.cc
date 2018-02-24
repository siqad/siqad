// @file:     settings_dialog.cc
// @author:   Samuel
// @created:  2018.02.23
// @editted:  2018.02.23  - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Settings dialog for users to alter settings

#include "settings_dialog.h"
#include "settings.h"


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


void SettingsDialog::initSettingsDialog()
{
  // all settings panes in a stacked widget, only one is shown at a time
  QStackedWidget *stacked_settings_panes = new QStackedWidget(this);
  stacked_settings_panes->addWidget(appSettingsPane());
  stacked_settings_panes->addWidget(guiSettingsPane());
  stacked_settings_panes->addWidget(latticeSettingsPane());

  // list of all categories
  QListWidget *settings_category_list = new QListWidget(this);
  settings_category_list->addItem("Application");
  settings_category_list->addItem("Interface");
  settings_category_list->addItem("Lattice");

  connect(settings_category_list, SIGNAL(currentRowChanged(int)),
          stacked_settings_panes, SLOT(setCurrentIndex(int)));

  // horizontal layout for list on the left and settings pane on the right
  QHBoxLayout *settings_hl = new QHBoxLayout;
  settings_hl->addWidget(settings_category_list);
  settings_hl->addWidget(stacked_settings_panes);

  // TODO bottom buttons for Apply, OK, Cancel

  // wrap everything together in a neat layout
  QVBoxLayout *main_layout = new QVBoxLayout;
  main_layout->addLayout(settings_hl);

  setLayout(main_layout);
}

QWidget *SettingsDialog::appSettingsPane()
{
  if (app_settings_pane)
    return app_settings_pane;

  // TODO implement
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


} // end of settings namespace
