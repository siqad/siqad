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

bool ElectrodePolyHelper::addPoint(QPointF point)
{
  points.append(point);
  poly_point_trail.append(new prim::PolygonHandle(point));
  if (points.size() > 1) {
    addSegment(points[points.size()-2], points[points.size()-1]);
    return true;
  }
  return false;
}

void ElectrodePolyHelper::addSegment(QPointF start, QPointF end)
{
  poly_segment_trail.append(new prim::PolygonSegment(start, end));
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
  for (prim::PolygonHandle* handle: poly_point_trail){
    delete handle;
  }
  poly_point_trail.clear();
  for (prim::PolygonSegment* segment: poly_segment_trail){
    delete segment;
  }
  poly_segment_trail.clear();
}

void ElectrodePolyHelper::toolChangeResponse(gui::ToolType tool_type)
{
  if (tool_type != ElectrodePolyTool) {
    clearPoints();
    clearTrail();
    showGhost(false);
  }

}

} // end of gui namespace
