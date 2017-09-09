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
#include <cmath>
#include <random>

namespace phys {

  class SimAnneal: public PhysicsEngine
  {
  public:
    // constructor
    SimAnneal(const std::string& fname);

    // destructor
    ~SimAnneal() {};

    // run simulation
    bool runSim();



    // ACCEPTANCE FUNCTIONS

    // acceptance function for population change. 
      // v_offset shifts the probability function horizontally for freezing out the problem
      // dir = 1 for 0->1, dir = 0 for 1->0
      // temp = temperature
    bool acceptPop(float v_eff, float v_offset, float temp, bool dir); 

    // acceptance function for hopping
    bool acceptHop(float v_del);

    // generate true or false based on given probaility
    bool evalProb(float prob);

  private:
    std::vector<int> db_charges; // charge in each db, only 0 or 1 are allowed
    std::vector<std::vector<float>> db_r; // distance between all dbs
  };
}
