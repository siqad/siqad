#include "settings.h"

settings::Settings::Settings()
  :QSettings(settings::settings_default, QSettings::IniFormat)
{}

settings::Settings::~Settings()
{}

settings::GUISettings::GUISettings()
  :QSettings(settings::gui_settings_default, QSettings::IniFormat)
{}

settings::GUISettings::~GUISettings()
{}

settings::LatticeSettings::LatticeSettings()
  :QSettings(settings::lattice_settings_default, QSettings::IniFormat)
{}

settings::LatticeSettings::LatticeSettings(const QString& fname)
  :QSettings(fname, QSettings::IniFormat)
{}

settings::LatticeSettings::~LatticeSettings()
{}
