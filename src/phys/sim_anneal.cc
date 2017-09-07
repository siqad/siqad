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

  std::cout << std::endl << "Iteration complete without seg fault!" << std::endl;


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
}
