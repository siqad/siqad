// @file:     emitter.cc
// @author:   Jake
// @created:  2016.11.24
// @editted:  2017.05.09  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Emitter definitions


#include "emitter.h"

prim::Emitter *prim::Emitter::inst = 0;

prim::Emitter *prim::Emitter::instance()
{
  // if no instance has been created, initialize
  if(!inst)
    inst = new prim::Emitter();
  return inst;
}

void prim::Emitter::clear()
{
  // if instance not set to NULL, free memory
  if(inst)
    delete inst;
  inst=0;
}

void prim::Emitter::selectClicked(prim::Item *item)
{
  emit sig_selectClicked(item);
}

void prim::Emitter::addItemToScene(prim::Item *item)
{
  emit sig_addItemToScene(item);
}

void prim::Emitter::removeItemFromScene(prim::Item *item)
{
  emit sig_removeItemFromScene(item);
}

void prim::Emitter::moveDBToLatticeCoord(Item *item, int n, int m, int l)
{
  emit sig_moveDBToLatticeCoord(item, n, m, l);
}

void prim::Emitter::physLoc2LatticeCoord(QPointF physloc, int &n, int &m, int &l)
{
  emit sig_physLoc2LatticeCoord(physloc, n, m, l);
}

void prim::Emitter::editTextLabel(Item *item, const QString &new_text)
{
  emit sig_editTextLabel(item, new_text);
}
