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



    // ENV
    void tempStep();
    void vFreezeStep();



    // PHYS CALC
    float systemEnergy();



    // ACCEPTANCE FUNCTIONS
    //bool acceptPop(float v_eff, float v_freeze, float temp, bool direction);  TODO del later
    bool acceptPop(int db_ind, bool direction); 
    bool acceptHop(float v_diff); // acceptance function for hopping
    bool evalProb(float prob); // generate true or false based on given probaility



    // OTHER ACCESSORS
    int getRandDBInd(bool occ);

  private:
    std::vector<int> db_charges; // charge in each db, only 0 or 1 are allowed
    std::vector<std::vector<float>> db_r; // distance between all dbs
    
    // VARIABLES
    float v_0; // global potential and other stuff (magic number)
    float kT, t, t_step; // temperature, time, time step for each time progression
    float v_freeze; // freeze out potential (pushes out population transition probability)
    std::vector<float> v_eff, v_electrode;
    std::vector<std::vector<float>> v_ij;
  };
}
