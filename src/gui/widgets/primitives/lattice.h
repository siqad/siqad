// @file:     lattice.h
// @author:   Jake
// @created:  2016.11.15
// @editted:  2017.05.11  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Specialized Layer class, Lattice, which details the dangling
//            bond locations.


#ifndef _GUI_PR_LATTICE_H_
#define _GUI_PR_LATTICE_H_

#include "layer.h"
#include "latdot.h"

namespace prim{

  class Lattice : public prim::Layer
  {
  public:

    // constructor
    Lattice(const QString &fname = QString());

    // destructor
    ~Lattice() {}

  private:

    int n_cell;         // number of atoms in unit cell

    QPointF a[2];       // lattice vectors
    QList<QPointF> b;   // unit cell site vectors

    qreal Lx;           // x-bound on lattice vectors, in Angstroms
    qreal Ly;           // y-bound on lattice vectors, in Angstroms

    // construct lattice from lattice settings
    void construct();

    // n==0: finds all pairs (m,l) such that m*a[0]+n*a[1] is in the drawing region
    void getLatticeInds(QList<QPoint> &inds, int n=0);

    // find lower and upper bounds on m for a given n
    void findBounds(int &lo, int &hi, int n);

    // construct the lattice dots for the unit cell corresponding to the given
    // lattice vector indices
    void buildUnitCell(const QPoint &ind);
  };

} // end prim namespace



#endif
