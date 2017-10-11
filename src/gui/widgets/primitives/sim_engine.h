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
    SimEngine(const QString &e_desc_path, QWidget *parent);

    // destructor
    ~SimEngine() {};

    // function for reading simulator definition from xml
    bool readEngineDecl(QFile *in_f);

    // generate a directory for the next job to use
    QString generateJobDir();


    // ACCESSORS

    // simulator info, for showing up in manager
    // available parameters and associated type, for user alteration

    void setBinaryPath(const QString &b_path) {bin_path = b_path;}
    QString getBinaryPath() {return bin_path;}


  private:
    // variables like binary location, temp file location, etc.
    QString eng_desc_path;      // description file of this engine
    QString bin_path;           // binary path of this engine
    QString eng_problem_root;   // root directory for all problems files for this engine

    // TODO some stack/dictionary/etc with simulator info, for showing up in manager
    // TODO something that stores default parameters, associated types (so the appropriate fields are used), for user alteration
  };

} // end of prim namespace

#endif
