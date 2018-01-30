// @file:     sim_engine.h
// @author:   Samuel
// @created:  2017.10.03
// @editted:  2017.10.03 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     SimEngine object that SimManager interacts with

#ifndef _PRIM_SIM_ENG_H_
#define _PRIM_SIM_ENG_H_

#include <QtWidgets>
#include <QtCore>
#include "src/settings/settings.h" // TODO probably need this later

namespace prim{

  class SimEngine : public QObject
  {
    Q_OBJECT
  public:
    // constructor
    SimEngine(const QString &eng_nm, const QString &eng_root, QWidget *parent=0);

    // destructor
    ~SimEngine() {};

    // read simulator definition from xml
    bool readEngineDecl(QFile *in_f);

    // read sim param dialog from provided *.ui file (xml)
    bool constructSimParamDialog();


    // ACCESSORS
    QString name() {return eng_name;}
    void setName(const QString &nm) {eng_name = nm;}
    QString version() {return eng_version;}
    void setVersion(const QString &ver) {eng_version = ver;}
    QString binaryPath() {return bin_path;}
    void setBinaryPath(const QString &path) {bin_path = path;}
    QString runtimeTempDir();

    // gui
    QWidget *simParamDialog() {return sim_param_dialog;}

    // simulator info, for showing up in manager
    // available parameters and associated type, for user alteration


  private:
    // variables like binary location, temp file location, etc.
    QString eng_desc_file;      // description file of this engine
    QString eng_name;           // name of this engine
    QString eng_root;           // root directory of this engine containing description and more
    QString eng_version;
    QString bin_path;           // binary path of this engine
    QString runtime_temp_dir;   // root directory for all problems files for this engine

    // GUI
    QWidget *sim_param_dialog=0;// gui for setting sim params for this engine, loaded from *.ui


    // TODO some stack/dictionary/etc with simulator info, for showing up in manager
    // TODO something that stores default parameters, associated types (so the appropriate fields are used), for user alteration
  };

} // end of prim namespace

#endif
