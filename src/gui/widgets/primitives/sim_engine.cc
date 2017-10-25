// @file:     sim_engine.cc
// @author:   Samuel
// @created:  2017.10.03
// @editted:  2017.10.03 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     SimEngine classes

#include "sim_engine.h"

namespace prim{

/*SimEngine::SimEngine(const QString &s_desc_file, QWidget *parent)
  : QObject(parent), eng_desc_file(s_desc_file)
{
  // if(!readSimInfo()) throw error
}*/

// temporary constructor until desc file is implemented
SimEngine::SimEngine(const QString &eng_nm, QWidget *parent)
  : QObject(parent), eng_name(eng_nm)
{}


bool SimEngine::readEngineDecl(QFile *in_f)
{
  // check desc path readability

  // start QXmlStreamReader

  // read simulator properties and desired parameters from simulator XML
  return true;
}


QString SimEngine::runtimeTempDir()
{
  if(runtime_temp_dir.isEmpty()){
    runtime_temp_dir = "./tmp/phys"; // TODO hard coded for now, this should be done by readEngineDecl in the future. The relative path should be relative to the root temp dir
  }
  return runtime_temp_dir;
}


} // end of prim namespace
