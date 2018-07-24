// @file:     electrode_poly_helper.h
// @author:   Nathan
// @created:  2018.07.17
// @editted:  2018.07.17 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     Widget to create and edit polygonal electrodes

#ifndef _GUI_ELECTRODE_POLY_HELPER_H_
#define _GUI_ELECTRODE_POLY_HELPER_H_

// TODO includes
#include "../../global.h"
#include "primitives/electrode_poly.h"
#include "primitives/polygon_handle.h"
#include "primitives/polygon_segment.h"


namespace gui{

  class ElectrodePolyHelper : public QWidget
  {
    Q_OBJECT

  public:

    // constructor
    // TODO change active_afm_layer_index to pointer to layer manager
    ElectrodePolyHelper(QWidget *parent = 0);

    // destructor
    ~ElectrodePolyHelper();

    //Adds apoint to the polygon plan. Returns true if a new segment is
    //available after addition of the new point.
    bool addPoint(QPointF point);
    void addSegment(QPointF start, QPointF end);
    void clearPoints();
    void clearTrail();
    void showGhost(bool show);
    prim::PolygonHandle* ghostHandle(){return ghost_handle;}
    QList<prim::PolygonSegment*> ghostSegment(){return ghost_segment;}
    QList<QPointF> getPoints(){return points;}
    QList<prim::PolygonHandle*> getTrail(){return poly_point_trail;}
    QList<prim::PolygonSegment*> getSegments(){return poly_segment_trail;}
    prim::PolygonHandle* getLastHandle(){return poly_point_trail.last();}
    prim::PolygonSegment* getLastSegment(){return poly_segment_trail.last();}
    // tool change actions
    void toolChangeResponse(gui::ToolType tool_type);

  private:
    // VAR
    QList<prim::PolygonHandle*> poly_point_trail;
    QList<QPointF> points;
    prim::PolygonHandle* ghost_handle;
    QList<prim::PolygonSegment*> ghost_segment;
    // prim::PolygonSegment* ghost_segment;
    QList<prim::PolygonSegment*> poly_segment_trail;
  };

} // end of gui namespace

#endif
