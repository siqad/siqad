#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <QSettings>
#include <QString>

namespace settings{

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
  LatticeSettings();
  ~LatticeSettings();
};

} // end settings namespace

#endif
