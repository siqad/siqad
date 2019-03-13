/** @file:     db_locations.cc
 *  @author:   Samuel
 *  @created:  2019.03.17
 *  @license:  GNU LGPL v3
 *
 *  @desc:     Stores DB locations.
 */

#include "db_locations.h"

using namespace comp;

DBLocations::DBLocations(QXmlStreamReader *rs)
  : JobResult(DBLocationsResult)
{
  auto unrecognizedXMLElement = [](QXmlStreamReader &rs)
  {
    qWarning() << tr("Invalid element encountered on line %1 - %2")
      .arg(rs.lineNumber()).arg(rs.name().toString());
    rs.skipCurrentElement();
  };

  while (rs->readNextStartElement()) {
    if (rs->name() == "dbdot") {
      float x = rs->attributes().value("x").toFloat();
      float y = rs->attributes().value("y").toFloat();
      rs->skipCurrentElement(); // TODO check whether this leads to unintentional skips
      db_locs.append(QPointF(x,y));
    } else {
      unrecognizedXMLElement(*rs);
    }
  }
}
