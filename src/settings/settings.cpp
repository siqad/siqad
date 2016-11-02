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
