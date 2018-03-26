// @file:     main.cc
// @author:   Samuel
// @created:  2017.08.28
// @editted:  2017.08.28 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Main function for physics engine

#include "afm_marcus.h"
#include <iostream>
#include <string>

using namespace phys;

// temporary main function for testing the xml parsing functionality
int main(int argc, char *argv[])
{
  std::cout << "AFMMarcus invoked" << std::endl << std::endl;
  std::string if_name, of_name, script_path, temp_path;

  std::cout << "*** Argument Parsing ***" << std::endl;

  // for now, only support two arguments: input and output files
  // TODO flags: -i input_path -o output_path
  // maybe make a struct to contain program options in case of more input options
  if_name = (argc < 2) ? "cooldbdesign.xml" : argv[1];
  of_name = (argc < 3) ? "cooloutput.xml" : argv[2];
  script_path = (argc < 4) ? "python/db-sim-connector.py" : argv[3]; // TODO default to blank before porting back to generic
  temp_path = (argc < 5) ? "./tmp" : argv[4];

  std::cout << "In File: " << if_name << std::endl;
  std::cout << "Out File: " << of_name << std::endl;
  std::cout << "Script path: " << script_path << std::endl;
  std::cout << "Temp path: " << temp_path << std::endl;

  std::cout << std::endl << "*** Constructing Problem ***" << std::endl;
  AFMMarcus afm_marcus(if_name, of_name, script_path, temp_path);

  std::cout << std::endl << "*** Run Simulation ***" << std::endl;
  if(!afm_marcus.runSim()) {
    std::cout << "Simulation failed, aborting" << std::endl;
    return 1;
  }

  std::cout << std::endl << "*** Write Result to Output ***" << std::endl;
  afm_marcus.writeResultsXml();
}
