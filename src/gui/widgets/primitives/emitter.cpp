#include "emitter.h"


prim::Emitter* prim::Emitter::inst = 0;

prim::Emitter *prim::Emitter::instance()
{
  // if no instance has been created, initialize
  if(!inst)
    inst = new prim::Emitter();
  return inst;
}
