// @file:     settings_dialog.h
// @author:   Samuel
// @created:  2018.02.23
// @editted:  2018.02.23  - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Settings dialog for users to alter settings


#ifndef _SETTINGS_DIALOG_H_
#define _SETTINGS_DIALOG_H_

#include <QtWidgets>
#include <QtCore>

#include "settings.h"

namespace settings{

  class SettingsDialog: public QWidget
  {
    Q_OBJECT

  public:
    SettingsDialog(QWidget *parent=0);
    ~SettingsDialog() {};

  public slots:
    void boolUpdate(bool new_state);

  private:
    // initialise the dialog and panes
    void initSettingsDialog();
    QWidget *appSettingsPane();
    QWidget *guiSettingsPane();
    QWidget *latticeSettingsPane();

    // VARS
    AppSettings *app_settings=0;
    GUISettings *gui_settings=0;
    LatticeSettings *lattice_settings=0;

    QWidget *app_settings_pane=0;
    QWidget *gui_settings_pane=0;
    QWidget *lattice_settings_pane=0;
  };

} // end of settings namespace

#endif
