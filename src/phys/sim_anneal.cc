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
  std::vector<int> db_charges; // charge in each db, only 0 or 1 are allowed
  std::vector<std::vector<float>> db_r; // distance between all dbs
  float temp, v_eff, v_offset; // other system variables

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



  // TODO random initial population and charge distribution

  // TODO initial temperature

  // while not converged
    // for each dbdot
      // make changes to population using current electrostatics potential
      // accept population changes? (acceptance function)
    // for each dbdot
      // make changes to e- occupation (hopping, random)
      // accept hopping changes? (if E goes down, accept 100%. Else, certain acceptance function)
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

bool SimAnneal::acceptPop(float v_eff, float v_offset, float temp, bool dir)
{
  float k = 8.61733E-5; // Boltzmann constant in eV/K, might have to switch to other units later
  float v = dir ? v_eff-v_offset : v_offset-v_eff;
  float prob;
  
  prob = 1. / ( 1 + exp( v/(k*temp) ) );

  return evalProb(prob);
}

// acceptance function for hopping
bool SimAnneal::acceptHop(float v_del)
{
  if (v_del < 0)
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
