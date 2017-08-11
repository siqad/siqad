// @file:     settings.h
// @author:   Jake
// @created:  2016.10.31
// @editted:  2017.05.08  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Custom classes for storing persistent and temporary settings.
//            Most instances will be singletons and hence there is no load
//            time for accessing settings.


#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <QtWidgets>
#include <QtCore>

#define DEFAULT_OVERRIDE true // always use default settings

namespace settings{

// TODO: should move the default settings .ini files into the resources and the
//      .qrc to make the path more reobust.

// default settings .ini files
const QString app_settings_default = "src/settings/app_settings.ini";
const QString gui_settings_default = "src/settings/gui_settings.ini";
const QString lattice_settings_default = "src/settings/lattices/si_100_2x1.ini";


// abstract container class for settings with a templated accessor function.
// The typical use case is throintoor now, assert .ini format for OS compatability.
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
    qDebug() << tr("Getting key %1, defaults pointer %2").arg(key).arg((size_t)defaults);
    QVariant var = DEFAULT_OVERRIDE ? defaults->value(key) : this->value(key);
    T val;
    qDebug() << tr("Valid? %1").arg(var.isValid());
    qDebug() << var.typeName();
    // if key not found, get value from defaults
    if(!var.isValid() && defaults != 0){
      var = defaults->value(key);

      // if key not in defaults, prompt and abort
      if(!var.isValid()){   // terminate
        qDebug() << tr("Searching for key: %1").arg(key);
        qFatal(tr("Requested key missing in defaults... terminating").toLatin1().constData(),0);
      }
      else
        val = var.value<T>();

      // save default value to current local settings
      this->setValue(key, val);
    }
    else if(defaults==0)    // terminate if no defaults
      qFatal(tr("No default settings available... terminating").toLatin1().constData(),0);
    else if(var.isValid())
      val = var.value<T>();
    else
      qFatal(tr("Unexpected key in Settings::get").toLatin1().constData(),0);

    return val;
  }

protected:

  // pointer to defaults, no special considerations for derived classes
  QSettings *defaults;
};



// Application settings, singleton
class AppSettings: public Settings
{
public:
  // get or create static instance of Emitter object
  static AppSettings *instance();

  // destructor, should possibly free and reset the default settings pointer
  ~AppSettings() {delete defs; defs=0;}

private:

  static AppSettings *inst;   // static pointer to the instance

  // constructor private: singleton
  AppSettings() : Settings(app_settings_default){defaults = defs;}

  // default values
  static QSettings *defs;     // pointer to default settings, initalized before first call
  static QSettings* m_defs(); // constructs the defaults settings
};



// GUI settings, singleton
class GUISettings: public Settings
{
public:
  // get or create static instance of Emitter object
  static GUISettings *instance();

  // destructor, should possibly free and reset the default settings pointer
  ~GUISettings() {delete defs; defs=0;}

private:

  static GUISettings *inst;   // static pointer to the instance

  // constructor private: singleton
  GUISettings() : Settings(gui_settings_default){defaults = defs;}

  // default values
  static QSettings *defs;     // pointer to default settings, initalized before first call
  static QSettings* m_defs(); // constructs the defaults settings
};


// Settings describing the unit cell structure of the surface lattice, singleton
// It may be better to remove the singleton constraint in future.
class LatticeSettings: public Settings
{
public:
  // get or create static instance of Emitter object
  static LatticeSettings *instance();

  // destructor, should possibly free and reset the default settings pointer
  ~LatticeSettings() {delete defs; defs=0;}

  // update to a new lattice settings file
  static void updateLattice(const QString &fname = QString());

private:

  static LatticeSettings *inst;   // static pointer to the instance

  // constructors private: singleton
  LatticeSettings() : Settings(lattice_settings_default){defaults = defs;}
  LatticeSettings(const QString &fname) : Settings(fname){defaults = defs;}

  // default values
  static QSettings *defs;     // pointer to default settings, initalized before first call
  static QSettings* m_defs(); // constructs the defaults settings
};

} // end settings namespace

#endif
