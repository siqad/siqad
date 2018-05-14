// @file:     convex.h
// @author:   Jake
// @created:  2017.07.14
// @editted:  2017.07.14  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Classes for convex hulls


#ifndef _HULL_CONVEX_H_
#define _HULL_CONVEX_H_

#include "hull.h"

namespace hull{

  // tight box around all points
  QPolygonF boundingBox(const QVector<QPointF> &points);

  // optimal convex hulls with increasing performance
  QPolygonF jarvisAlg(const QVector<QPointF> &points);
  QPolygonF grahamScan(const QVector<QPointF> &points);
  //QPolygonF chanAlg(const QVector<QPointF> &points);

  // hull is the bounding box of the union of all points
  class BBHull : public Hull{
  public:

    // constructors
    BBHull(const QVector<QPointF> points): Hull(points) {}
    BBHull(const QList<prim::Item*> &items): Hull(items) {}
    BBHull(const QVector<prim::Item*> &items): Hull(items) {}

    // find the hull
    virtual void solve() Q_DECL_OVERRIDE;

  private:
  };


  // tightest possible convex hull
  class ConvexHull : public Hull{
  public:

    // constructors
    ConvexHull(const QVector<QPointF> points): Hull(points) {}
    ConvexHull(const QList<prim::Item*> &items): Hull(items) {}
    ConvexHull(const QVector<prim::Item*> &items): Hull(items) {}

    // find the hull
    virtual void solve() Q_DECL_OVERRIDE;

  private:
  };

} // end hull namespace

#endif
