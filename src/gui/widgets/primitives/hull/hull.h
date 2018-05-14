// @file:     hull.h
// @author:   Jake
// @created:  2017.07.12
// @editted:  2017.07.14  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Base class for Hull finding algorithms

#ifndef _HULL_HULL_H_
#define _HULL_HULL_H_

#include <QtCore>
#include "../item.h"

namespace hull{

  // base class for Hull's
  class Hull{
  public:

    // constructors
    Hull(const QVector<QPointF> points);
    Hull(const QList<prim::Item*> &items);
    Hull(const QVector<prim::Item*> &items);

    ~Hull() {}

    // get a reference to the Hull's bounding polygon.
    QPolygonF &getPolygon() {return poly;}

    // find the hull and prepare poly
    virtual void solve() = 0;

  protected:

    QVector<QPointF> points;
    QPolygonF poly;  // ordered list of boundary points

    // remove all co-linear points
    void reducePolygon();

  private:
  };

}; // end hull namespace

#endif
