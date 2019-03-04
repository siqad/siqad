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
#define DEFAULT_OVERRIDE true   // use user settings for release compilation
//#define DEFAULT_OVERRIDE false   // TODO change back to false after implementation
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
    return get(key).value<T>();
  }

  //! Return the value of the specified key in QVariant type.
  QVariant get(const QString &key)
  {
    QVariant var = DEFAULT_OVERRIDE ? defaults->value(key) : this->value(key);

    // if key not found, get value from defaults
    if (!var.isValid() && defaults != 0) {
      var = defaults->value(key);

      // if key not in defaults, prompt and abort
      if (!var.isValid()) {   // terminate
        qDebug() << tr("Searching for key: %1").arg(key);
        qFatal(tr("Requested key missing in defaults... terminating").toLatin1().constData(),0);
      } else {
        return var;
      }

      // save default value to current local settings
      //this->setValue(key, val);
    } else if (defaults==0) {   // terminate if no defaults
      qFatal(tr("No default settings available... terminating").toLatin1().constData(),0);
    } else if (var.isValid()) {
      return var;
    } else {
      qFatal(tr("Unexpected key in Settings::get").toLatin1().constData(),0);
    }

    return var;
  }

  // Returns a QString with each of the included replaceable paths replaced by
  // a writable path (whatever QStandardPaths::writableLocation provides)
  QString getPath(const QString &key)
  {
    return pathReplacement(get<QString>(key));
  }

  // Returns a QStringList of paths with the preset path replacements.
  QStringList getPaths(const QString &key)
  {
    QStringList paths = get<QStringList>(key);
    QStringList::iterator i;
    for (i = paths.begin(); i != paths.end(); ++i) {
      *i = pathReplacement(*i);
    }
    return paths;
  }

  // returns a QStringList with each of the included replaceable paths replaced
  // by all possible paths given by QStandardPaths::standardLocations
  /*
  QStringList getAllPossiblePaths(const QString &key)
  {
    QStringList val_split = get<QString>(key).split(';', QString::SkipEmptyParts);
    QStringList paths_return;

    // perform replacement
    QRegExp regex("<(.*)?>");
    regex.setMinimal(true);

    for (QString val : val_split) {
      //qDebug() << tr("val: %1").arg(val);
      // assumes that there is only one replacement per splitted value
      if (val.indexOf(regex) != -1) {
        QString found_tag = regex.capturedTexts().first();
        for (QString loc : standardLocations(found_tag)) {
          QString new_val = val;
          new_val.replace(val.indexOf(regex), found_tag.length(), loc);
          paths_return.append(new_val);
          qDebug() << tr("Standard path added: %1").arg(new_val);
        }
      } else {
        // no tag to replace, add without modification
        paths_return.append(val);
      }
    }

    return paths_return;
  }
  */

  static QString pathReplacement(QString path)
  {
    if (!path_map.contains("BINPATH"))
      constructPathMap();

    // perform replacement
    QRegExp regex("<(.*)?>");
    regex.setMinimal(true);
    while (path.indexOf(regex) != -1) {
      QString found_path = regex.capturedTexts().first();
      if (!path_map.contains(found_path)) {
        qFatal(tr("Path replacement failed, key '%1' not found.")
            .arg(found_path).toLatin1().constData(),0);
      }
      path.replace(path.indexOf(regex), found_path.length(), path_map[found_path]);
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
      path_map["<APPLOCALDATA>"] = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    }
    path_map["<HOME>"] = QDir::homePath();
    path_map["<ROOT>"] = QDir::rootPath();
    path_map["<SYSTMP>"] = QDir::tempPath() + "/siqad";
    path_map["<CONFIG>"] = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/siqad";
  }

  static QStringList standardLocations(const QString &type)
  {
    // NOTE very messy implementation, improve in the future
    if (type == "<HOME>") {
      return {QDir::homePath()};
    } else if (type == "<ROOT>") {
      return {QDir::rootPath()};
    } else if (type == "<SYSTMP>") {
      return {QDir::tempPath() + "/siqad"};
    } else if (type == "<CONFIG>") {
      QStringList locs =  QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
      QStringList locs_return;
      for (QString loc : locs)
        locs_return.append(loc + "/siqad");
      return locs_return;
    } else if (type == "<BINPATH>") {
      if (!QCoreApplication::startingUp())
        return {QCoreApplication::applicationDirPath()};
      else
        qFatal("Trying to access <BINPATH> before QApplication has been launched");
    } else if (type == "<APPLOCALDATA>") {
      if (!QCoreApplication::startingUp())
        return QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
      else
        qFatal("Trying to access <APPLOCALDATA> before QApplication has been launched");
    } else {
      qFatal("Provided standard location type not known");
    }

    return {};
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
