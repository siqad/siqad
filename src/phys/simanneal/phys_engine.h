// @file:     phys_engine.h
// @author:   Samuel
// @created:  2017.08.23
// @editted:  2017.08.23 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Base class for physics engines

#ifndef _SIM_ANNEAL_PHYS_PHYS_ENGINE_H_
#define _SIM_ANNEAL_PHYS_PHYS_ENGINE_H_

#include "problem.h"

#include <string>
#include <vector>
#include <boost/circular_buffer.hpp>

namespace phys{

  namespace bpt = boost::property_tree;

  class PhysicsEngine
  {
  public:

    // constructor
    PhysicsEngine(const std::string &eng_nm, const std::string &i_path, const std::string &o_path);

    // destructor
    ~PhysicsEngine() {};

    // export results
    void writeResultsXml();

    // variables
    Problem problem;

    std::vector<std::pair<float,float>> db_locs; // location of free dbs
    boost::circular_buffer<std::vector<int>> db_charges;

  private:
    std::string eng_name;
    std::string out_path;

  };

}

#endif
