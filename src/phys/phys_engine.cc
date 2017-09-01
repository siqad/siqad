// @file:     phys_engine.cc
// @author:   Samuel
// @created:  2017.08.23
// @editted:  2017.08.23 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Base class for physics engines

#include "phys_engine.h"

using namespace phys;

// CONSTRUCTOR
PhysicsEngine::PhysicsEngine(const std::string &fname)
{
  problem.readProblem(fname);
}

