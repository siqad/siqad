// @file:     polygon_handle.cc
// @author:   Nathan
// @created:  2018.07.17
// @editted:  2018.07.17 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     Handles for polygons, similar to ResizeHandle

#include "polygon_handle.h"

namespace prim{

PolygonHandle::PolygonHandle()
  : prim::Item(prim::Item::ElectrodePoly)
{
  qDebug() << "POLYGON HANDLE";
}


}
