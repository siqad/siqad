// @file:     emitter.cpp
// @author:   Jake
// @created:  2016.11.24
// @editted:  2017.05.01  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Emitter definitions

#include "emitter.h"


prim::Emitter* prim::Emitter::inst = 0;

prim::Emitter *prim::Emitter::instance()
{
  // if no instance has been created, initialize
  if(!inst)
    inst = new prim::Emitter();
  return inst;
}
