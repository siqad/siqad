// @file:     sim_anneal.cc
// @author:   Samuel
// @created:  2017.08.23
// @editted:  2017.08.23 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Simulated annealing physics engine

#include "sim_anneal.h"

using namespace phys;

SimAnneal::SimAnneal(const std::string& fname)
  : PhysicsEngine(fname)
{

}

bool SimAnneal::runSim()
{
  int i=0,j=0;
  int n_dbs = problem.db_tree->size();

  // INIT VARS

  // fill in r (db to db distance) matrix
  for(Problem::DBIterator db_iter1 = problem.begin(); db_iter1 != problem.end(); ++db_iter1) {
    for(Problem::DBIterator db_iter2 = problem.begin(); db_iter2 != problem.end(); ++db_iter2) {
      if (i>j)
        db_r[i][j] = db_r[j][i];
      else if (i==j) 
        db_r[i][j] = 0;
      else
        db_r[i][j] = pow((**db_iter1).x - (**db_iter2).x, 2.0) + pow((**db_iter1).y - (**db_iter2).y, 2.0);
      j++;
    }
    i++; 
    j=0;
  }

  // TODO fill in v_ij, note how to deal with v_ii

  // TODO fill in v_electrode (0 for now)


  // other problem variables
  float temp = 200, v_freeze = 0, v_eff; // TODO placeholder for temp, change later
  int from_db, to_db;
  bool converged = false;

  while(!converged) {
    // pop
    for(int i=0; i<n_dbs; i++) {
      // make changes to population using current electrostatics potential
      v_eff = v_0 + v_electrode[i];
      for(int j=0; j<n_dbs; j++)
        v_eff += v_ij[i][j] * db_charges[j];

      // make population change?
      db_charges[i] = acceptPop(v_eff, v_freeze, temp, !db_charges[i]) ? 1 : 0;
    }

    // hop
    bool finished_hop = false;
    int successful_hops;
    float prev_E, new_E;
    // TODO might have to store the previous hop(s) that has been performed to reduce redundant calculations
    while(!finished_hop) {
      prev_E = systemEnergy(); // calculate original energy

      from_db = getRandDBInd(true); // from an occupied DB
      to_db = getRandDBInd(false); // to an unoccupied DB

      // perform the hop
      db_charges[from_db] = 0;
      db_charges[to_db] = 1;

      // calculate new system energy and see whether the hop is accepted
      new_E = systemEnergy();
      if(!acceptHop(new_E-prev_E)) {
        // reverse hop if the energy delta is unaccepted
        db_charges[from_db] = 1;
        db_charges[to_db] = 0;
      }
      else
        successful_hops++; // TODO might not be useful at the end, delete this line if so

      // TODO determine finished_hop
      // TODO haven't figured out how many hops to perform in total
    }

    // temp & v_freeze time step
    tempStep();
    vOffsetStep();

    // TODO pre-annealing (keep time-step at 0 for a while)

    // TODO determine convergence
  }

  return true;

  /* Iterator test code

  std::shared_ptr<Problem::Aggregate> db_treee = problem.db_tree;
  for(std::shared_ptr<Problem::DBDot> dot_pt : db_treee->dbs){
    std::cout << "dot_pt: " << dot_pt << std::endl;
  }
  std::cout << "&*dbs.cend (dot end): " << &*db_treee->dbs.cend() << std::endl;
  //std::cout << "problem.end(): " << &*problem.end() << std::endl;
  //std::shared_ptr<Problem::DBDot> dbdott = db_treee->dbs.();
  //std::cout << "db_tree last elem: " << problem.db_tree->dbs.back()->x << "," << problem.db_tree->dbs.back()->y << std::endl;
  // print the location of all dbs to test the iterator
  std::cout << std::endl << std::endl;
  for(Problem::DBIterator db_iter = problem.begin(); db_iter != problem.end(); ++db_iter) {
    std::cout << "\n**\nfor loop" << std::endl;
    std::shared_ptr<Problem::DBDot> dbdot_ptr = *db_iter;
    std::cout << "dbdot pointer: " << dbdot_ptr << std::endl;
    std::cout << "DBDot loc: " << (**db_iter).x << ", " << (**db_iter).y << std::endl;
    std::cout << std::endl;
  }

  std::cout << std::endl << "Iteration complete without seg fault!" << std::endl;*/
}



// ENV
void SimAnneal::tempStep()
{

}

void SimAnneal::vDelStep()
{

}



// PHYS CALC

float SimAnneal::systemEnergy()
{
  
}



// ACCEPTANCE FUNCTIONS

bool SimAnneal::acceptPop(int db_ind, bool direction)
{
  //float k = 8.61733E-5; // Boltzmann constant in eV/K, might have to switch to other units later
  float v = direction ? v_eff[db_ind]-v_freeze : v_freeze-v_eff[db_ind];
  float prob;
  
  prob = 1. / ( 1 + exp( v/kT ) );

  return evalProb(prob);
}

// acceptance function for hopping
bool SimAnneal::acceptHop(float v_diff)
{
  if (v_diff < 0)
    return true;

  // some acceptance function, acceptance probability falls off exponentially
  float prob = 1; // TODO placeholder
  return evalProb(prob);
}

bool SimAnneal::evalProb(float prob)
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(0,1);

  return dis(gen) >= prob;
}




// ACCESSORS

int SimAnneal::getRandDBInd(bool occ)
{
  std::vector<int> dbs;

  // store the indices of dbs that have the desired occupation
  for (unsigned int i=0; i<db_charges.size(); i++)
    if (db_charges[i] == occ)
      dbs.push_back(i);

  // pick one from them
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<int> dis(0,dbs.size());

  return dbs[dis(gen)];
}

float SimAnneal::systemEnergy()
{

}

