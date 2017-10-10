// @file:     sim_engine.cc
// @author:   Samuel
// @created:  2017.10.03
// @editted:  2017.10.03 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     SimEngine classes

#include "sim_engine.h"

namespace prim{

SimEngine::SimEngine(const QString &s_desc_path, QWidget *parent)
  : QObject(parent), sim_desc_path(s_desc_path)
{
  // if(!readSimInfo()) throw error
}


bool SimEngine::readSimEngineDecl(QFile *in_f)
{
  // check desc path readability

  // start QXmlStreamReader

  // read simulator properties and desired parameters from simulator XML
  return true;
}

// invoke the simulator binary
bool SimEngine::invokeBinary(const QStringList &arguments)
{
  // TODO check whether bin_path is valid

  // TODO check that &arguments contains a valid path of the problem XML

  sim_process = new QProcess();
  sim_process->start(bin_path, arguments);
  
  // TODO check documentation for piping outputs

  // TODO connect signals for error and finish

  return true;
}



} // end of prim namespace
