#ifndef _GUI_LATTICE_H_
#define _GUI_LATTICE_H_

#include "primitives/layer.h"
#include "primitives/dbdot.h"
#include "primitives/emitter.h"
#include "src/settings/settings.h"

#include <QString>
#include <QPointF>
#include <QList>

namespace gui{


class Lattice : public prim::Layer
{
public:

  // constructor
  Lattice();
  Lattice(const QString &fname);

  // destructor
  ~Lattice();


private:

  void construct(settings::LatticeSettings &lattice_settings);

  // n == 0: finds all pairs (m,l) s.t m*a1+l*a2 is in the drawing region.
  void getLatticeInds(QList<QPoint> *inds, int n=0);

  // find lower and upper bounds on m for a given n
  void findBounds(int *low, int *high, int n);

  // construct the QGraphicsItems for the unit cell corresponding to the given
  // lattice vector indices
  void buildUnitCell(QPoint ind);


  QPointF a[2];     // lattice vectors

  int n_cell;       // number of atoms in unit cell
  QList<QPointF> b; // unit cell site vectors

  qreal Lx; // x-bound on lattice vectors, in Angstroms
  qreal Ly; // y-bound on lattice vectors, in Angstroms

};


} // end gui namespace



#endif
