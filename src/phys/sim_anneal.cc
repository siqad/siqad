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
  // TODO random initial population and charge distribution

  // TODO initial temperature

  // while not converged
    // for each dbdot
      // make changes to population using current electrostatics potential
      // accept population changes? (if E goes down, accept 100%. Else, certain acceptance function)
    // for each dbdot
      // make changes to e- occupation (hopping, random)
      // accept hopping changes? (if E goes down, accept 100%. Else, certain acceptance function)
  return true;
}

