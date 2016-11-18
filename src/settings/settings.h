#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <QSettings>
#include <QString>
#include <QDebug>

#include <QVariant>

#define DEFAULT_OVERRIDE true // always use default settings

namespace settings{

// default settings .ini files
const QString app_settings_default = "src/settings/app_settings.ini";
const QString gui_settings_default = "src/settings/gui_settings.ini";
const QString lattice_settings_default = "src/settings/lattices/si_100_2x1.ini";

class Settings: public QSettings
{
public:
  Settings(const QString &fname)
    : QSettings(fname, QSettings::IniFormat), defaults(0) {}
  ~Settings() {}

  // attempt to read the keyed value from the Settings file. If the key is not
  // found, it checks for default values. If a default value is found, the
  // value is stored in the current settings file; otherwise, the app terminates
  template<typename T>
  T get(QString key)
  {
    QVariant var = DEFAULT_OVERRIDE ? defaults->value(key) : this->value(key);
    T val;
    // if key not found, get value from defaults
    if(!var.isValid() && defaults != 0){
      var = defaults->value(key);

      // if key not in defaults, prompt and abort
      if(!var.isValid()){ // terminate
        qDebug() << QString("Searching for key: %1").arg(key);
        qFatal("Requested key missing in defaults... terminating");
      }
      else
        val = var.value<T>();

      // save default value to current settings
      this->setValue(key, val);
    }
    else if(defaults==0) // terminate
      qFatal("No default settings available... terminating");
    else
      val = var.value<T>();

    return val;
  }

protected:

  QSettings *defaults;
};




class AppSettings: public Settings
{
public:
  AppSettings() : Settings(app_settings_default){defaults = defs;}
  ~AppSettings() {}

  // default values
  static QSettings *defs;
  static QSettings* m_defs();
};



class GUISettings: public Settings
{
public:
  GUISettings() : Settings(gui_settings_default){defaults = defs;}
  ~GUISettings() {}

  // default values
  static QSettings *defs;
  static QSettings* m_defs();
};



class LatticeSettings: public Settings
{
public:
  // constructors
  LatticeSettings() : Settings(lattice_settings_default){defaults = defs;}
  LatticeSettings(const QString& fname) : Settings(fname){defaults = defs;}

  //destructor
  ~LatticeSettings() {}

  // default values
  static QSettings *defs;
  static QSettings* m_defs();
};

} // end settings namespace

#endif
