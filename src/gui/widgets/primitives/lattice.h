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
#include <QHash>

namespace prim{

  struct LatticeCoord {
    //! Construct a lattice coordinate with n, m and l coordinates.
    LatticeCoord(int n, int m, int l) : n(n), m(m), l(l) {};
    //! Construct empty lattice coordinates
    LatticeCoord() {};
    int n;
    int m;
    int l;  // invalid if l < 0

    bool operator==(const LatticeCoord &other) const {
      if (other.n == n && other.m == m && other.l == l)
        return true;
      return false;
    }

    //! Lattice coordinate addition, returned lattice coordinate might not be
    //! valid!
    LatticeCoord operator+(const LatticeCoord other) const {
      return LatticeCoord(n+other.n, m+other.m, l+other.l);
    }

    //! Lattice coordinate subtraction, returned lattice coordinate might not be
    //! valid!
    LatticeCoord operator-(const LatticeCoord other) const {
      return LatticeCoord(n-other.n, m-other.m, l-other.l);
    }
  };

  class Lattice : public prim::Layer
  {
  public:

    //! constructor
    Lattice(const QString &fname = QString(), int lay_id=-1);

    //! destructor
    ~Lattice() {}

    //! Save the lattice layer to XML stream.
    void saveLayer(QXmlStreamWriter *) const override;

    //! Return specified lattice vector after graphical scaling
    QPoint sceneLatticeVector(int dim) const {return a_scene[dim];}

    //! Return specified site vector after graphical scaling
    QPoint sceneSiteVector(int ind) const {return b_scene[ind];}

    //! Identify the nearest lattice site to the given scene position.
    LatticeCoord nearestSite(const QPointF &scene_pos) const;

    //! Identify the nearest lattice site to the given scene position, returns
    //! it in lattice coordinates and updates the reference QPointF site_pos.
    LatticeCoord nearestSite(const QPointF &scene_pos, QPointF &nearest_site_pos) const;

    //! Return all sites enclosed in given lattice coordinates. WARNING this won't
    //! work with rotated lattices!
    QList<prim::LatticeCoord> enclosedSites(const prim::LatticeCoord &coord1,
        const prim::LatticeCoord &coord2) const;

    //! Convert lattice coordinates to QPointF. Does not check for validity!
    QPointF latticeCoord2ScenePos(const prim::LatticeCoord &l_coord) const;

    //! Return whether a given scene_pos collides with the given lattice position
    bool collidesWithLatticeSite(const QPointF &scene_pos, const LatticeCoord &l_coord) const;

    //! Set lattice dot location to be occupied
    void setOccupied(const prim::LatticeCoord &l_coord, prim::Item *dbdot) {
      occ_latdots.insert(l_coord, dbdot);
    }

    //! Set lattice dot location to be unoccupied
    void setUnoccupied(const prim::LatticeCoord &l_coord) {
      occ_latdots.remove(l_coord);
    }

    //! Return whether lattice dot location is occupied.
    bool isOccupied(const prim::LatticeCoord &l_coord) {
      return occ_latdots.contains(l_coord);
    }

    //! Return whether the given lattice coordinate is a valid coordinate.
    bool isValid(const prim::LatticeCoord &l_coord) const {
      return (l_coord.l >= 0 && l_coord.l < b.length());
    }

    //! Return the DBDot pointer at the specified lattice coord, or 0 if none.
    prim::Item *dbAt(const prim::LatticeCoord &l_coord) {
      return occ_latdots.contains(l_coord) ? occ_latdots.value(l_coord) : 0;
    }

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
    qreal m_big;        // maximum distance in search for nearest site

    QHash<prim::LatticeCoord, prim::Item*> occ_latdots; // set of occupied lattice dots

    // constants

    static qreal rtn_acc;     // termination precision for rationalize
    static int rtn_iters;     // max iterations of rationalize

    // construct lattice from lattice settings
    void construct();

    // Find a rational approximation of a given float
    QPair<int,int> rationalize(qreal x, int k=0);
  };

  inline uint qHash(const prim::LatticeCoord &l_coord, uint seed=0)
  {
    return ::qHash(l_coord.n, seed) + ::qHash(l_coord.m, seed) + ::qHash(l_coord.l, seed);
  }

/*inline bool operator==(const prim::LatticeCoord coord1, const prim::LatticeCoord coord2)
{
  if (coord1.n == coord2.n && coord1.m == coord2.m && coord1.l == coord2.l)
    return true;
  return false;
}*/
} // end prim namespace


#endif
