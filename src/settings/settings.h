#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <QSettings>
#include <QString>

namespace settings{

// default settings .ini files
const QString settings_default = "src/settings.ini";
const QString gui_settings_default = "src/settings/gui_settings.ini";
const QString lattice_settings_default = "src/settings/lattices/si_100_2x1.ini";

class Settings: public QSettings
{
public:
  Settings();
  ~Settings();
};

class GUISettings: public QSettings
{
public:
  GUISettings();
  ~GUISettings();
};

class LatticeSettings: public QSettings
{
public:
  // constructors
  LatticeSettings();
  LatticeSettings(const QString& fname);

  //destructor
  ~LatticeSettings();
};

} // end settings namespace

#endif
