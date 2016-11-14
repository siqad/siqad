#include "settings.h"


settings::Settings::Settings()
  :QSettings("src/settings/settings.ini", QSettings::IniFormat)
{}

settings::Settings::~Settings()
{}

settings::GUISettings::GUISettings()
  :QSettings("src/settings/gui_settings.ini", QSettings::IniFormat)
{}

settings::GUISettings::~GUISettings()
{}

settings::LatticeSettings::LatticeSettings()
  :QSettings("src/settings/lattices/si_100_2x1.ini", QSettings::IniFormat)
{}

settings::LatticeSettings::LatticeSettings(const QString& fname)
  :QSettings(fname, QSettings::IniFormat)
{}

settings::LatticeSettings::~LatticeSettings()
{}
