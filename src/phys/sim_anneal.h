// @file:     sim_anneal.h
// @author:   Samuel
// @created:  2017.08.23
// @editted:  2017.08.23 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Simulated annealing physics engine

#include "phys_engine.h"
#include <vector>
#include <tuple>
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
    void timeStep();
    void printCharges();



    // PHYS CALC
    float systemEnergy();
    float distance(float x1, float y1, float x2, float y2);
    float interElecPotential(float r);



    // ACCEPTANCE FUNCTIONS
    //bool acceptPop(float v_eff, float v_freeze, float temp, bool direction);  TODO del later
    bool acceptPop(int db_ind); 
    bool acceptHop(float v_diff); // acceptance function for hopping
    bool evalProb(float prob); // generate true or false based on given probaility



    // OTHER ACCESSORS
    int getRandDBInd(bool occ);

  private:
    // CONST
    const int div_0 = 1E5; // arbitrary big number that represents divide by 0
    const float har_to_ev = 27.2114; // hartree to eV conversion factor
    const float db_distance_scale = 1E-10; // TODO move this to xml

    // VARIABLES
    int n_dbs=-1; // number of dbs
    float debye_length; // Silicon intrinsic Debye length in m (TODO trial and error to get good magic number)
    std::vector<int> db_charges; // charge in each db, only 0 or 1 are allowed
    std::vector<std::vector<float>> db_r; // distance between all dbs
    float v_0; // global potential and other stuff (magic number)
    float kT, kT_step, v_freeze_step; // temperature, time
    int t=0;
    float v_freeze; // freeze out potential (pushes out population transition probability)
    std::vector<float> v_eff, v_ext;
    std::vector<std::vector<float>> v_ij;
  };
}
