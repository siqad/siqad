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
  ghost_handle = new prim::PolygonHandle(QPointF(0,0));
  showGhost(false);
  qDebug() << "EPH created";
}

ElectrodePolyHelper::~ElectrodePolyHelper()
{
  delete ghost_handle;
}

void ElectrodePolyHelper::addPoint(QPointF point)
{
  points.append(point);
  poly_trail.append(new prim::PolygonHandle(point));
}

void ElectrodePolyHelper::clearPoints()
{
  points.clear();
}

void ElectrodePolyHelper::showGhost(bool show)
{
  ghost_handle->setVisible(show);
  return;
}


void ElectrodePolyHelper::clearTrail()
{
  for (prim::PolygonHandle* handle: poly_trail){
    delete handle;
  }
  poly_trail.clear();
}

void ElectrodePolyHelper::toolChangeResponse(gui::ToolType tool_type)
{
  if (tool_type != ElectrodePolyTool)
    clearPoints();
    showGhost(false);
}

} // end of gui namespace
