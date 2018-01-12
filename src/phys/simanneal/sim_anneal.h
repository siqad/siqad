// @file:     sim_anneal.h
// @author:   Samuel
// @created:  2017.08.23
// @editted:  2017.08.23 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Simulated annealing physics engine

#include "phys_engine.h"
#include <vector>
#include <deque>
#include <tuple>
#include <memory>
#include <cmath>

#include <boost/random.hpp>

namespace phys {

  class SimAnneal: public PhysicsEngine
  {
  public:
    // constructor
    SimAnneal(const std::string& i_path, const std::string& o_path);

    // destructor
    ~SimAnneal() {};

    // run simulation
    bool runSim();


  private:
    // simulation components
    void initVars();
    void precalc();
    void simAnneal();     // perform simulated annealing
    void dbHop(int from_ind, int to_ind);
    void timeStep();      // advance time-step
    void printCharges();

    // physical calculations
    float systemEnergy();
    float distance(float x1, float y1, float x2, float y2);
    float interElecPotential(float r);

    // acceptance functions
    bool acceptPop(int db_ind); 
    bool acceptHop(float v_diff); // acceptance function for hopping
    bool evalProb(float prob); // generate true or false based on given probaility

    // other accessors
    int getRandDBInd(int charge);
    int chargedDBCount(int charge);


    boost::random::mt19937 rng;

    // VARIABLES
    const int div_0 = 1E5; // arbitrary big number that represents divide by 0
    const float har_to_ev = 27.2114; // hartree to eV conversion factor
    const float db_distance_scale = 1E-10; // TODO move this to xml

    // TODO variables could be grouped into structs
    // TODO reduce class-level variables

    int n_dbs=-1; // number of dbs
    float debye_length; // Silicon intrinsic Debye length in m (TODO trial and error to get good magic number)
    //std::vector<int> db_charges; // charge in each db, only 0 or 1 are allowed
    std::vector<std::vector<float>> db_r; // distance between all dbs
    //std::vector<std::pair<float,float>> db_locs; // location of free dbs
    std::vector<std::tuple<float,float,float>> fixed_charges; // location of fixed charges
    float v_0; // global potential and other stuff (magic number)
    float kT, kT_step, v_freeze_step; // temperature, time
    int t=0, t_max, t_preanneal;
    float v_freeze; // freeze out potential (pushes out population transition probability)
    std::vector<float> v_eff, v_ext, v_drive;
    std::vector<std::vector<float>> v_ij;
    float unfav_hop_scale; // acceptance prob of positive energy hopes, lower for less probable: exp(-v_diff/unfav_hop_scale)
    // TODO result queue: which type?
    int result_queue_size;
    //std::deque<std::vector<int>> db_charges;
    std::vector<int> curr_charges;
  };
}
