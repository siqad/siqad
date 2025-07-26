// @file:     hull.cc
// @author:   Jake
// @created:  2017.07.12
// @editted:  2017.07.14  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Base class for Hull finding algorithms

#include "hull.h"


// BASE CLASS :: Hull

hull::Hull::Hull(const QVector<QPointF> points)
 : points(points)
{}

hull::Hull::Hull(const QVector<prim::Item *> &items)
{
  for(QGraphicsItem *item : items){
    QPolygonF poly = item->shape().toFillPolygon();
    poly.translate(item->scenePos());
    bool first=true;
    for(const QPointF &pf : poly){
      if(!first)
        points.append(pf);
      else
        first = false;
    }
  }
}

void hull::Hull::reducePolygon()
{}
