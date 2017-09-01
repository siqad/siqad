// @file:     phys_engine.h
// @author:   Samuel
// @created:  2017.08.23
// @editted:  2017.08.23 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Base class for physics engines

#include "problem.h"
#include <string>

namespace phys{

  class PhysicsEngine
  {
  public:

    // constructor
    PhysicsEngine(const std::string &fname);

    // destructor
    ~PhysicsEngine() {};

    // variables
    Problem problem;
  };

}
