/** @file:     potential_landscape.cc
 *  @author:   Samuel
 *  @created:  2019.03.17
 *  @license:  GNU LGPL v3
 *
 *  @desc:     Stores DB locations.
 */

#include "potential_landscape.h"

using namespace comp;

PotentialLandscape::PotentialLandscape(QXmlStreamReader *rs, const QString &result_dir_path)
  : JobResult(PotentialLandscapeResult)
{
  auto unrecognizedXMLElement = [](QXmlStreamReader &rs)
  {
    qWarning() << tr("Invalid element encountered on line %1 - %2")
      .arg(rs.lineNumber()).arg(rs.name().toString());
    rs.skipCurrentElement();
  };

  while (rs->readNextStartElement()) {
    if (rs->name().toString() == "potential_val") {
      QVector<float> potentials_vec;
      float x = rs->attributes().value("x").toFloat();
      float y = rs->attributes().value("y").toFloat();
      float pot_val = rs->attributes().value("val").toFloat();
      potentials_vec.append({x, y, pot_val});
      potential_vals.append(potentials_vec);
      rs->skipCurrentElement();
    } else {
      unrecognizedXMLElement(*rs);
    }
  }

  // hacky way to get image/animation paths
  // TODO future proper implementation should have PoisSolver pass paths through
  // SiQADConn
  QDir result_dir(result_dir_path);
  QString static_plot_file_name = "SiAirBoundary000.png";
  QString animation_file_name = "SiAirBoundary.gif";
  QString plot_legend_file_name = "SiAirPlot.png";
  if (result_dir.exists(static_plot_file_name))
    static_plot_path = result_dir.absoluteFilePath(static_plot_file_name);
  if (result_dir.exists(animation_file_name))
    animation_path = result_dir.absoluteFilePath(animation_file_name);
  if (result_dir.exists(plot_legend_file_name))
    plot_legend_path = result_dir.absoluteFilePath(plot_legend_file_name);
}
