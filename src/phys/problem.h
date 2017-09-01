// @file:     problem.h
// @author:   Samuel
// @created:  2017.08.23
// @editted:  2017.08.23 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Definition of the problem - dbdot loc, material properties, etc.

#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "rapidxml-1.13/rapidxml.hpp"

namespace phys{

  class Problem
  {
  public:
    // Constructor
    Problem(const std::string &fname);
    Problem() {initProblem();};
    void initProblem();

    // Destructor
    ~Problem() {};

    // dangling bond
    struct DBDot {
      float x,y;  // physical location in angstroms
      float charge;
      DBDot(float in_x, float in_y) : x(in_x), y(in_y), charge(0) {};
    };

    // aggregate
    struct Aggregate {
      std::vector<std::unique_ptr<Aggregate>> aggs;
      std::vector<std::unique_ptr<DBDot>> dbdots;
    };

    // electrode
    struct Electrode {
      float x,y;      // physical location in angstroms (top left corner)
      float dx,dy;    // width and height in angstroms
      float z;        // vertical distance from surface
      float voltage;  // voltage that the electrode is set to
    };

    // File Handling

    bool readProblem(const std::string &fname);
    bool readMaterialProp(rapidxml::xml_node<> *node);
    bool readSimulationParam(rapidxml::xml_node<> *node);
    bool readItemTree(rapidxml::xml_node<> *node, const std::unique_ptr<Aggregate>& agg_parent);
    bool readDBDot(rapidxml::xml_node<> *node, const std::unique_ptr<Aggregate>& agg_parent);

    static bool writeResult();

    // Variables
    std::unique_ptr<Problem::Aggregate> db_tree;
  };
}
