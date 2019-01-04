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

#include "settings.h"
#include "../gui/property_map.h"
#include "../gui/widgets/property_form.h"

namespace settings{

  class SettingsDialog: public QWidget
  {
    Q_OBJECT

  public:

    enum SettingsCategory{App, GUI, Lattice};
    Q_ENUM(SettingsCategory)

    SettingsDialog(QWidget *parent=0);
    ~SettingsDialog() {};

    //! A struct that holds pending changes to one setting
    struct PendingChange {
      PendingChange(const SettingsCategory &category, const QString &name,
          const QVariant &value)
        : category(category), name(name), value(value)
      {}
      SettingsCategory category;
      QString name;
      QVariant value;
    };

  public slots:
    void addPendingBoolUpdate(bool new_state);
    void addPendingStringUpdate(QString new_text);

    void applyPendingChanges();
    void applyAndClose();
    void discardAndClose();

  private:
    //! Initialise the settings dialog and panes of contained categories.
    void initSettingsDialog();

    //! Write user setting to the provided property, with the setting entry
    //! specified by the "category" and "key" entries in the property's "meta"
    //! member.
    void writeUserSettingToProperty(gui::Property &prop);

    //! Return the application settings pane. Initilize the pane if first called.
    gui::PropertyForm *appSettingsPane();

    //! Return the application settings pane. Initilize the pane if first called.
    gui::PropertyForm *guiSettingsPane();

    //! Return the application settings pane. Initilize the pane if first called.
    gui::PropertyForm *latticeSettingsPane();

    //! Return the Settings class pointer to the specified category QString.
    settings::Settings *settingsCategory(const QString &t_cat);

    //! Return the Settings class pointer to the specified category.
    settings::Settings *settingsCategory(SettingsCategory);

    // VARS
    AppSettings *app_settings=nullptr;
    GUISettings *gui_settings=nullptr;
    LatticeSettings *lattice_settings=nullptr;

    gui::PropertyForm *app_settings_pane=nullptr;
    gui::PropertyForm *gui_settings_pane=nullptr;
    gui::PropertyForm *lattice_settings_pane=nullptr;

    QList<PendingChange> pending_changes;
  };

} // end of settings namespace

#endif
