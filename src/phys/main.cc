// @file:     main.cc
// @author:   Samuel
// @created:  2017.08.28
// @editted:  2017.08.28 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Main function for physics engine

#include "sim_anneal.h"
#include <iostream>
#include <string>

using namespace phys;

// temporary main function for testing the xml parsing functionality
int main()
{
  std::cout << "Physeng invoked" << std::endl;

  std::string fname("export_to_simanneal.xml");
  SimAnneal sim_anneal(fname);

  sim_anneal.runSim();
  //Problem *problem = new Problem();

  //problem->readProblem(std::string("problem_sample.xml"));
}
