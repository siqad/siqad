/** @file:     lattice.h
 *  @author:   Jake
 *  @created:  2016.11.15
 *  @editted:  2017.06.07  - Jake
 *  @license:  GNU LGPL v3
 *
 *  @desc:     Specialized Layer class, Lattice, which details the dangling
 *             bond locations.
 */

#ifndef _GUI_PR_LATTICE_H_
#define _GUI_PR_LATTICE_H_

#include "layer.h"
#include "latdot.h"

namespace prim{

  class Lattice : public prim::Layer
  {
  public:

    //! constructor
    Lattice(const QString &fname = QString(), int lay_id=-1);

    //! destructor
    ~Lattice() {}

    // identify the nearest lattice site to the given scene position
    QPointF nearestSite(const QPointF &scene_pos);

    // identify the bounding rect of an approximately rectangular supercell
    QRectF tileApprox();

  private:

    int layer_id;       // index of this layer in design panel's stack

    int n_cell;         // number of atoms in unit cell

    QPointF a[2];       // lattice vectors
    QList<QPointF> b;   // unit cell site vectors

    qreal Lx;           // x-bound on lattice vectors, in Angstroms
    qreal Ly;           // y-bound on lattice vectors, in Angstroms

    qreal coth;         // cotangent of angle between lattice vectors
    qreal a2[2];        // square magnitudes of lattice vectors

    // constants

    static qreal rtn_acc; // termination precision for rationalize
    static int rtn_iters;    // max iterations of rationalize

    // construct lattice from lattice settings
    void construct();

    // n==0: finds all pairs (m,l) such that m*a[0]+n*a[1] is in the drawing region
    void getLatticeInds(QList<QPoint> &inds, int n=0);

    // find lower and upper bounds on m for a given n
    void findBounds(int &lo, int &hi, int n);

    // construct the lattice dots for the unit cell corresponding to the given
    // lattice vector indices
    void buildUnitCell(const QPoint &ind);

    // Find a rational approximation of a given float
    QPair<int,int> rationalize(qreal x, int k=0);
  };

} // end prim namespace



#endif
