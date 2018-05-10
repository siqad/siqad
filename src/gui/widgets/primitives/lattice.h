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

  struct LatticeCoord {
    LatticeCoord(int n, int m, int l) : n(n), m(m), l(l) {};
    LatticeCoord() {};
    int n;
    int m;
    int l;

    bool operator==(const LatticeCoord &other) {
      if (other.n == n && other.m == m && other.l == l)
        return true;
      return false;
    }
  };

  class Lattice : public prim::Layer
  {
  public:

    //! constructor
    Lattice(const QString &fname = QString(), int lay_id=-1);

    //! destructor
    ~Lattice() {}

    //! Return specified lattice vector after graphical scaling
    QPoint sceneLatticeVector(int dim) {return a_scene[dim];}

    //! Return specified site vector after graphical scaling
    QPoint sceneSiteVector(int ind) {return b_scene[ind];}

    //! identify the nearest lattice site to the given scene position
    LatticeCoord nearestSite(const QPointF &scene_pos);

    //! identify the bounding rect of an approximately rectangular supercell
    QRectF tileApprox();

    //! Return a tileable image that represents the lattice.
    QImage tileableLatticeImage(QColor bkg_col);

  private:

    int layer_id;       // index of this layer in design panel's stack

    int n_cell;         // number of atoms in unit cell

    QPointF a[2];       // lattice vectors
    QPoint a_scene[2];  // lattice vector after graphical scaling
    QList<QPointF> b;   // unit cell site vectors
    QList<QPoint> b_scene;  // unit cell site vectors after graphical scaling

    qreal Lx;           // x-bound on lattice vectors, in Angstroms
    qreal Ly;           // y-bound on lattice vectors, in Angstroms

    qreal coth;         // cotangent of angle between lattice vectors
    qreal a2[2];        // square magnitudes of lattice vectors

    // constants

    static qreal rtn_acc; // termination precision for rationalize
    static int rtn_iters;    // max iterations of rationalize

    // construct lattice from lattice settings
    void construct();

    // Find a rational approximation of a given float
    QPair<int,int> rationalize(qreal x, int k=0);
  };

} // end prim namespace



#endif
