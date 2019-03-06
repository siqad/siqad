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

  signals:

    //! Signal application.cc to reset all settings at application destruction.
    void sig_resetSettings();

  public slots:

    //! Apply changes made in the settings forms.
    void applyPendingChanges();

  private:
    //! Initialise the settings dialog and panes of contained categories.
    void initSettingsDialog();

    //! Reset the form value of all forms.
    void resetForms();

    //! Write user setting to the provided property, with the setting entry
    //! specified by the "category" and "key" entries in the property's "meta"
    //! member.
    void setPropertyWithUserSetting(gui::Property &prop);

    //! Return the Settings class pointer to the specified category QString.
    settings::Settings *settingsCategory(const QString &t_cat);

    //! Return the Settings class pointer to the specified category.
    settings::Settings *settingsCategory(SettingsCategory);

    // VARS
    AppSettings *app_settings=nullptr;
    GUISettings *gui_settings=nullptr;
    LatticeSettings *lattice_settings=nullptr;

    //! List of pointers to property forms of settings
    QList<gui::PropertyForm*> s_forms;
  };

} // end of settings namespace

#endif
