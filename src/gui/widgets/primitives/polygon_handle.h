// @file:     polygon_handle.h
// @author:   Nathan
// @created:  2018.07.17
// @editted:  2018.07.17 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     Handles for polygons, similar to ResizeHandle

#ifndef _PRIM_POLYGON_HANDLE_H_
#define _PRIM_POLYGON_HANDLE_H_

#include "item.h"

namespace prim{

  class PolygonHandle : public Item
  {
  public:
    PolygonHandle(QPointF point, QGraphicsItem *parent=0);
    QPointF getPoint(){return center;}
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem*, QWidget*) override;

  private:
    void constructStatics();
    void initPolygonHandle(QPointF point);

    QRectF scene_rect;
    QPointF center;
    qreal handle_dim;
  };


}

#endif
