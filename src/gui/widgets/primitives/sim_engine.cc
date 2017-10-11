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
  : QObject(parent), eng_desc_path(s_desc_path)
{
  // if(!readSimInfo()) throw error
}


bool SimEngine::readEngineDecl(QFile *in_f)
{
  // check desc path readability

  // start QXmlStreamReader

  // read simulator properties and desired parameters from simulator XML
  return true;
}




} // end of prim namespace
