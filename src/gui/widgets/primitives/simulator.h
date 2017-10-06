// @file:     simulator.h
// @author:   Samuel
// @created:  2017.10.03
// @editted:  2017.10.03 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Simulator object that sim_manager interacts with

#ifndef _PRIM_SIM_H_
#define _PRIM_SIM_H_

#include <QtWidgets>
#include <QtCore>
#include "src/settings/settings.h" // TODO probably need this later

namespace prim{

  class Simulator : public QObject
  {
    Q_OBJECT
  public:
    // constructor
    Simulator(const QString &s_desc_path, QWidget *parent);

    // destructor
    ~Simulator() {};

    // function for reading simulator definition from xml
    void readSimInfo(); // TODO might just put this in constructor

    // invoke simulator binary, TODO in the future maybe return stream of output?
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

    // some stack/dictionary/etc with simulator info, for showing up in manager
    // some stack/dictionary/etc with available parameters and associated type, for user alteration
  };

} // end of prim namespace

#endif
