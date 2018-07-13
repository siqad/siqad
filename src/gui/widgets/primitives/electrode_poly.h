/** @file:     electrode_poly.h
 *  @author:   Nathan
 *  @created:  2017.10.27
 *  @editted:  2018.01.17 - Nathan
 *  @license:  GNU LGPL v3
 *
 *  @brief:     Function prototypes for the ElectrodePoly object.
 */

#include <QtWidgets>
#include "resizablerect.h"

namespace prim{

  class Layer;

  class ElectrodePoly: public QPolygonF
  {
  public:
    ElectrodePoly(const QPolygonF);
  };

} //end prim namespace
