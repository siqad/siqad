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
#include <QRegExp>

#ifdef QT_NO_DEBUG
#define DEFAULT_OVERRIDE false  // use user settings for release compilation
#else
#define DEFAULT_OVERRIDE true   // use default settings for debug compilation
#endif

namespace settings{

const QString app_settings_path = "<CONFIG>/app_settings.ini";
const QString gui_settings_path = "<CONFIG>/gui_settings.ini";
const QString lattice_settings_path = "<CONFIG>/lattices/si_100_2x1.ini";


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
  T get(const QString &key)
  {
    QVariant var = DEFAULT_OVERRIDE ? defaults->value(key) : this->value(key);
    T val;
    // if key not found, get value from defaults
    if (!var.isValid() && defaults != 0) {
      var = defaults->value(key);

      // if key not in defaults, prompt and abort
      if (!var.isValid()) {   // terminate
        qDebug() << tr("Searching for key: %1").arg(key);
        qFatal(tr("Requested key missing in defaults... terminating").toLatin1().constData(),0);
      } else {
        val = var.value<T>();
      }

      // save default value to current local settings
      this->setValue(key, val);
    } else if (defaults==0) {   // terminate if no defaults
      qFatal(tr("No default settings available... terminating").toLatin1().constData(),0);
    } else if (var.isValid()) {
      val = var.value<T>();
    } else {
      qFatal(tr("Unexpected key in Settings::get").toLatin1().constData(),0);
    }

    return val;
  }

  QString getPath(const QString &key)
  {
    return pathReplacement(get<QString>(key));
  }

  static QString pathReplacement(QString path)
  {
    if (!path_map.contains("<BINPATH>"))
      constructPathMap();

    // perform replacement
    QRegExp regex("<(.*)?>");
    while (path.indexOf(regex) != -1) {
      if(!path_map.contains(regex.capturedTexts().first())) {
        qFatal(tr("Path replacement failed, key '%1' not found.")
            .arg(regex.capturedTexts().first()).toLatin1().constData(),0);
      }
      path.replace(regex, path_map[regex.capturedTexts().first()]);
    }
    return path;
  }

  // default settings .ini files

protected:

  // pointer to defaults, no special considerations for derived classes
  QSettings *defaults;

private:
  static void constructPathMap()
  {
    if (!QCoreApplication::startingUp()){
      // WARNING <BINPATH> can only be accessed after the core application has
      // been instantiated in main!
      path_map["<BINPATH>"] = QCoreApplication::applicationDirPath();
    }
    path_map["<HOME>"] = QDir::homePath();
    path_map["<ROOT>"] = QDir::rootPath();
    path_map["<SYSTMP>"] = QDir::tempPath() + "/db-sim";
    path_map["<CONFIG>"] = QStandardPaths::writableLocation(
        QStandardPaths::ConfigLocation) + "/db-sim";
  }
  static QMap<QString, QString> path_map;
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
  AppSettings() : Settings(pathReplacement(app_settings_path)){defaults = defs;}

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
  GUISettings() : Settings(pathReplacement(gui_settings_path)){defaults = defs;}

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
  LatticeSettings() : Settings(pathReplacement(lattice_settings_path)){defaults = defs;}
  LatticeSettings(const QString &fname) : Settings(fname){defaults = defs;}

  // default values
  static QSettings *defs;     // pointer to default settings, initalized before first call
  static QSettings* m_defs(); // constructs the defaults settings
};

} // end settings namespace

#endif
