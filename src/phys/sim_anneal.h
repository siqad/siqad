// @file:     sim_anneal.h
// @author:   Samuel
// @created:  2017.08.23
// @editted:  2017.08.23 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Simulated annealing physics engine

#include "phys_engine.h"
#include <vector>
#include <memory>

namespace phys {

  class SimAnneal: public PhysicsEngine
  {
  public:
    SimAnneal(const std::string& fname);
    ~SimAnneal() {};

    bool runSim();
  };
}
