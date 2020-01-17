// @file:     convex.cc
// @author:   Jake
// @created:  2017.07.14
// @editted:  2017.07.14  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Classes for convex hulls

#include <QtAlgorithms>
#include "convex.h"

// UTILITY FUNCTIONS FOR CONVEX HULL ALGORITHMS

enum Orient{COL, CW, CCW}; // {colinear, clockwise, counter-clockwise}

// orientation of ordered triplet
Orient orient(const QPointF &p, const QPointF &q, const QPointF &r)
{
  qreal val = (q.y()-p.y())*(r.x()-q.x()) - (q.x()-p.x())*(r.y()-q.y());
  return val==0 ? COL : ( val > 0 ? CW : CCW);
}

// squared distance between two points
qreal sqDist(const QPointF &a, const QPointF &b)
{
  qreal dx = b.x() - a.x(), dy = b.y() - a.y();
  return dx*dx+dy*dy;
}

// comparison of polar angles, struct to allow pivot argument
struct PolarCmp{
  PolarCmp(const QPointF &pivot): pivot(pivot) {}
  bool operator()(const QPointF &a, const QPointF &b){
    Orient ornt = orient(pivot, a, b);
    if(ornt==COL)
      return sqDist(pivot, a) < sqDist(pivot, b);
    return ornt==CCW;
  }
  QPointF pivot;
};

QPointF nextToTop(QStack<QPointF> &S)
{
  QPointF top = S.top();
  S.pop();
  QPointF next = S.top();
  S.push(top);
  return next;
}


// CONVEX HULL ALGORITHMS

void echoPoint(const QPointF &p){
  qDebug() << QString("point: %1 :: %2").arg(p.x()).arg(p.y());
}

QPolygonF hull::boundingBox(const QVector<QPointF> &points)
{
  QPolygonF poly;
  qreal xmin, xmax, ymin, ymax;

  if(!points.isEmpty()){
    xmin = xmax = points.first().x();
    ymin = ymax = points.first().y();

    for(const QPointF &pf : points){
      if(pf.x()<xmin) xmin = pf.x();
      if(pf.x()>xmax) xmax = pf.x();
      if(pf.y()<ymin) ymin = pf.y();
      if(pf.y()>ymax) ymax = pf.y();
    }

    poly << QPointF(xmin, ymin) << QPointF(xmax, ymin) << QPointF(xmax, ymax)
         << QPointF(xmin, ymax);
  }

  return poly;
}

QPolygonF hull::jarvisAlg(const QVector<QPointF> &points)
{
  QPolygonF poly;
  QVector<bool> used;
  used.resize(points.size());

  // need to have more than 3 points to have a hull
  if(points.size()<3) return poly;

  // initialized used and find left-most point
  int first=0;
  for(int i=1; i<points.size(); i++){
    used[i] = false;
    if(points[i].x() < points[first].x())
      first = i;
  }


  int p = first, q;
  do{
    // add current point to hull
    poly << points[p];

    // find most clockwise point:
    // if orient(p,i,q) is c.c.w, then i is more clockwise than q
    q = (p+1)%points.size();
    for(int i=0; i<points.size(); i++){
      if(used[i])
        continue;
      else if(orient(points[p], points[i], points[q])==CCW)
        q = i;
    }

    // update current point
    p = q;

  }while(p != first);

  return poly;
}

QPolygonF hull::grahamScan(const QVector<QPointF> &points)
{
  QPolygonF poly;

  if(points.size() < 3) return poly;

  int N = points.size();

  // find pivot : smallest y component, smallest x if tie.
  int pivot=0;
  for(int i=1; i<N; i++){
    if(points[i].y() < points[pivot].y() ||
      (points[i].y() == points[pivot].y() && points[i].x() < points[pivot].x()))
      pivot = i;
  }

  // going to edit the points vector in place so copy
  QVector<QPointF> points_c = points;

  // swap pivot into points[0]
  if(pivot)
    qSwap(points_c[0], points_c[pivot]);

  // sort points by polar angle in ccw order about pivot
  //qSort(points_c.begin()+1, points_c.end(), PolarCmp(points_c[0]));
  std::sort(points_c.begin()+1, points_c.end(), PolarCmp(points_c[0]));

  // keep only the furthest points for each angle from pivot
  int m = 1; // number of elements in modified points list
  for(int i=1; i<points_c.count(); i++){// QPolygonF hull::chanAlg(const QVector<QPointF> &)
// {
//   QPolygonF poly;
//   return poly;
// }
    while(i<N-1 && orient(points_c[0], points_c[i], points_c[i+1]) == COL)
      i++;
    points_c[m++] = points_c[i];
  }

  if(m<3) return poly;

  QStack<QPointF> S;
  // first few points
  for(int i=0; i<3; i++)
    S.push(points_c[i]);

  // remaining points
  for(int i=3; i<m; i++){
    while(orient(nextToTop(S), S.top(), points_c[i]) != CCW)
      S.pop();
    S.push(points_c[i]);
  }

  return poly << S;
}


// DERIVED CLASS :: BBHull

void hull::BBHull::solve()
{
  poly = hull::boundingBox(points);
}


// DERIVED CLASS :: ConvexHull

void hull::ConvexHull::solve()
{
  // use best implemented convex hull algorithm
  poly = hull::grahamScan(points);
}
