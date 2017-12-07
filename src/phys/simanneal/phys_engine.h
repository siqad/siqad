// @file:     phys_engine.h
// @author:   Samuel
// @created:  2017.08.23
// @editted:  2017.08.23 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Base class for physics engines

#include "problem.h"
#include "rapidxml-1.13/rapidxml.hpp"
#include "rapidxml-1.13/rapidxml_print.hpp"
#include <boost/circular_buffer.hpp>
//#include "rapidxml-1.13/rapidxml_utils.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <deque>

namespace phys{

  class PhysicsEngine
  {
  public:

    // constructor
    PhysicsEngine(const std::string &eng_nm, const std::string &i_path, const std::string &o_path);

    // destructor
    ~PhysicsEngine() {};

    // export results
    void writeResultsXML();

    // variables
    Problem problem;

    std::vector<std::pair<float,float>> db_loc; // location of free dbs
    //std::deque<std::vector<int>> db_charges;
    boost::circular_buffer<std::vector<int>> db_charges;

  private:
    std::string eng_name;
    std::string out_path;

  };

}
