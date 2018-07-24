// @file:     polygon_handle.h
// @author:   Nathan
// @created:  2018.07.17
// @editted:  2018.07.17 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     Handles for polygons, similar to ResizeHandle

#ifndef _PRIM_POLYGON_SEGMENT_H_
#define _PRIM_POLYGON_SEGMENT_H_

#include "item.h"

namespace prim{

  class PolygonSegment : public Item
  {
  public:
    PolygonSegment(QPointF start_in, QPointF end_in, QGraphicsItem *parent=0);
    void setPoints(QPointF start_in, QPointF end_in);
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem*, QWidget*) override;

  private:
    void constructStatics();
    void initPolygonSegment(QPointF start_in, QPointF end_in);
    QPointF start;
    QPointF end;
    static qreal line_width;
  };


}

#endif
