// @file:     electrode_poly.cc
// @author:   Nathan
// @created:  2017.10.27
// @editted:  2017.10.27 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     ElectrodePoly class for the functionality of polygonal electrodes

#include <algorithm>
#include "electrode_poly.h"
#include "src/settings/settings.h"


prim::ElectrodePoly::ElectrodePoly(const QPolygonF poly)
  : QPolygonF(poly)
{
  qDebug() << "ElectrodePoly created";
}

void prim::ElectrodePoly::test()
{
  for (QPointF point: *this){
    qDebug() << point;
  }
}
