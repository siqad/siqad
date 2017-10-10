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
    SimEngine(const QString &s_desc_path, QWidget *parent);

    // destructor
    ~SimEngine() {};

    // function for reading simulator definition from xml
    bool readSimEngineDecl(QFile *in_f);

    // invoke simulator binary, TODO might want to return the QProcess instead of bool
    // TODO move this to sim_job
    bool invokeBinary(const QStringList &arguments);
    // NOTE actually this might be it: https://stackoverflow.com/questions/14960472/running-c-binary-from-inside-qt-and-redirecting-the-output-of-the-binary-to-a

    

    // ACCESSORS

    // simulator info, for showing up in manager
    // available parameters and associated type, for user alteration

    void setBinaryPath(const QString &b_path) {bin_path = b_path;}


  private:
    // variables like binary location, temp file location, etc.
    QString sim_desc_path;
    QString bin_path;
    QProcess *sim_process;

    // TODO some stack/dictionary/etc with simulator info, for showing up in manager
    // TODO something that stores default parameters, associated types (so the appropriate fields are used), for user alteration
  };

} // end of prim namespace

#endif
