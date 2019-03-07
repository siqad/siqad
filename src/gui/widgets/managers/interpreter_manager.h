// @file:     interpreter_manager.h
// @author:   Samuel
// @created:  2019.03.06
// @license:  GNU LGPL v3
//
// @desc:     Manage simulation engine script interpreters (e.g. Python).

#ifndef _GUI_INTERP_MANAGER_H_
#define _GUI_INTERP_MANAGER_H_

#include <QtWidgets>
#include <QMultiMap>

#include "settings/settings.h"

namespace gui{

  //! Interpreter struct holds all relevant information related to that 
  //! interpreter.
  struct Interpreter {
    //! The command or file path and arguments to invoke this interpreter. The 
    //! 0th entry must be the command/file path, the rest of the arguments 
    //! should each be separated to an individual entry.
    QStringList command;

    //! (For Python) List of virtualenv paths related to this interpreter 
    //! previously used by SiQAD. Doesn't necessarily still exist.
    QStringList virtualenv_paths;
  };

  class InterpreterManager : public QWidget
  {
    Q_OBJECT

  public:

    //! Constructor.
    InterpreterManager(QWidget *parent = 0);

    //! Destructor.
    ~InterpreterManager();

    //! Return a QStringList of supported languages.
    QStringList supportedInterpreters() {return interps.keys();}

    //! Return a QStringList of interpreter commands/paths which correspond to 
    //! the given language name.
    QStringList getInterpreters(const QString &lang) {return interps.values(lang);}

    //! Return the most preferred interpreter command/path corresponding to the 
    //! given language name. If the user has not set a preferred interpreter then 
    //! the application's default is returned.
    QString getPreferredInterpreter(const QString &lang);

  private:

    QMultiMap<QString, QString> interps;

  };


} // end of gui namespace

#endif
