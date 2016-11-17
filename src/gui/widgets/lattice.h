#ifndef _GUI_LATTICE_H_
#define _GUI_LATTICE_H_

#include "primitives/layer.h"
#include "settings/settings.h"

#include <QString>

namespace gui{


class Lattice : public prim::Layer
{
public:

  // constructor
  Lattice();
  Lattice(QString fname);

  // destructor
  ~Lattice();

private:

  void construct(settings::LatticeSettings &lattice_settings);

  QString lattice_name;

};


} // end gui namespace



#endif
