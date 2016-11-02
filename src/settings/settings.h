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

} // end settings namespace

#endif
