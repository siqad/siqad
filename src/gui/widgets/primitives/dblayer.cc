/** @file:     dblayer.h
 *  @author:   Samuel
 *  @created:  2020.05.28
 *  @license:  GNU LGPL v3
 *
 *  @desc:     Implementation of DBLayer
 */

#include "dblayer.h"

using namespace prim;

QList<prim::DBDot*> DBLayer::getDBs()
{
  QList<prim::DBDot*> db_list;
  for (prim::Item *item : getItems()) {
    if (item->item_type == prim::Item::DBDot) {
      db_list.append(static_cast<prim::DBDot*>(item));
    }
  }
  return db_list;
}

QList<prim::DBDot*> DBLayer::getDBsAtLocs(const QList<QPointF> &phys_locs)
{
  QList<prim::DBDot*> dbs;

  for (const QPointF &phys_loc : phys_locs) {
    prim::DBDot *db = lattice->dbAt(lattice->nearestSite(phys_loc, false));
    if (db == nullptr) {
      qFatal("%s", tr("Invalid DB location (%1, %2), aborting DB gathering.")
          .arg(phys_loc.x()).arg(phys_loc.y()).toLatin1().constData());
      return QList<prim::DBDot*>();
    }
    dbs.append(db);
  }
  return dbs;
}
