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
int main(int argc, char *argv[])
{
  std::cout << "Physeng invoked" << std::endl;
  std::string if_name, of_name;

  // for now, only support one argument which is the input file
  // TODO -i input_path -o output_path
  // maybe make a struct to contain program options in case of more input options
  if(argc == 1){
    if_name = std::string("cooldbdesign.xml");
    of_name = std::string("cooloutput.xml");
  }
  else if(argc == 2){
    if_name = argv[1];
    of_name = std::string("cooloutput.xml");
  }
  else if(argc == 3){
    if_name = argv[1];
    of_name = argv[2];
  }
  else{
    std::cout << "More arguments than expected are encountered, aborting" << std::endl;
    return 0;
  }


  std::cout << "In File: " << if_name << std::endl;
  std::cout << "Out File: " << of_name << std::endl;

  SimAnneal sim_anneal(if_name, of_name);

  sim_anneal.runSim();

  sim_anneal.writeResultsXML();
  //Problem *problem = new Problem();

  //problem->readProblem(std::string("problem_sample.xml"));
}
