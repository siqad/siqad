// @file:     electrode_poly_helper.cc
// @author:   Nathan
// @created:  2018.07.17
// @editted:  2018.07.17 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     Widget to create and edit polygonal electrodes

#include "electrode_poly_helper.h"

namespace gui{

ElectrodePolyHelper::ElectrodePolyHelper(QWidget *parent)
  : QWidget(parent)
{
  qDebug() << "EPH created";
}

void ElectrodePolyHelper::addPoint(QPointF point)
{
  points.append(point);
}

void ElectrodePolyHelper::clearPoints()
{
  points.clear();
}

void ElectrodePolyHelper::toolChangeResponse(gui::ToolType tool_type)
{
  if (tool_type != ElectrodePolyTool)
    clearPoints();
}

} // end of gui namespace
