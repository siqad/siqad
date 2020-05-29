/** @file:      dblayer.h
 *  @author:    Samuel
 *  @created:   2020.05.28
 *  @license:   GNU LGPL v3
 *
 *  @desc:      Layer for holding DBs. Mostly just prim::Layer with some extra 
 *              DB-specific features.
 */

#ifndef _GUI_PR_DB_LAYER_H_
#define _GUI_PR_DB_LAYER_H_

#include "layer.h"
#include "items.h"

namespace prim{

  //! DB object layer class
  class DBLayer : public Layer
  {
    Q_OBJECT

  public:

    //! Constructor with specified parameters.
    DBLayer(prim::Lattice *lattice, const QString &nm = QString(), 
        const LayerRole &role=LayerRole::Design, float z_offset=0, 
        float z_height=0, int lay_id=-1, QObject *parent=nullptr)
      : Layer(nm, DB, role, z_offset, z_height, lay_id, parent),
        lattice(lattice) {};

    //! XML Stream constructor.
    DBLayer(QXmlStreamReader *rs, int lay_id=-1, bool override_role=false, 
        const LayerRole &role_override=Design)
      : Layer(rs, lay_id)
    {
      if (override_role)
        layer_role = role_override;
    }

    //! Get lattice pointer.
    prim::Lattice *getLattice() {return lattice;}

    //! Return a list of all DBs held by this layer.
    QList<prim::DBDot*> getDBs();

    //! Return a list of DBDot pointers at the provided physical locations. If
    //! any of the locations is not a valid DB site, a fatal error is reported.
    QList<prim::DBDot*> getDBsAtLocs(const QList<QPointF> &phys_locs);

  private:

    prim::Lattice *lattice;


  };
}

#endif
