// @file:     problem.h
// @author:   Samuel
// @created:  2017.08.23
// @editted:  2017.08.23 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Definition of the problem - dbdot loc, material properties, etc.

#ifndef _SIM_ANNEAL_PHYS_PROBLEM_H_
#define _SIM_ANNEAL_PHYS_PROBLEM_H_

#include <vector>
#include <stack>
#include <memory>
#include <string>
#include <map>
#include <iostream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace phys{

  namespace bpt = boost::property_tree;

  class Problem
  {
  public:
    // Constructor
    Problem(const std::string &fname);
    Problem() {initProblem();};
    void initProblem();

    // Destructor
    ~Problem() {};

    // File Handling
    bool readProblem(const std::string &fname);

    // Accessors
    bool parameterExists(const std::string &key) {return sim_params.find(key) != sim_params.end();}
    std::string getParameter(const std::string &key) {return sim_params.find(key) != sim_params.end() ? sim_params.at(key) : "";}


    // STRUCTS

    // dangling bond
    struct DBDot {
      float x,y;  // physical location in angstroms
      float elec;
      DBDot(float in_x, float in_y, float in_e) : x(in_x), y(in_y), elec(in_e) {};
    };

    // aggregate
    class Aggregate
    {
    public:
      std::vector<std::shared_ptr<Aggregate>> aggs;
      std::vector<std::shared_ptr<DBDot>> dbs;

      // Properties
      int size(); // returns the number of contained DBs, including those in children aggs
    };

    // electrode TODO Nathan change at will
    struct Electrode {
      float x,y;      // physical location in angstroms (top left corner)
      float dx,dy;    // width and height in angstroms
      float z;        // vertical distance from surface
      float voltage;  // voltage that the electrode is set to
    };



    // ITERATOR
    typedef std::vector<std::shared_ptr<DBDot>>::const_iterator DBIter;
    typedef std::vector<std::shared_ptr<Aggregate>>::const_iterator AggIter;

    // a constant iterator that iterates through all dangling bonds in the problem
    class DBIterator
    {
    public:
      explicit DBIterator(std::shared_ptr<Aggregate> root, bool begin=true);

      //~Iterator() {delete agg_stack;};

      DBIterator& operator++(); // recursive part here
      bool operator==(const DBIterator &other) {return other.db_iter == db_iter;}
      bool operator!=(const DBIterator &other) {return other.db_iter != db_iter;}
      std::shared_ptr<DBDot> operator*() const {return *db_iter;}

    private:

      DBIter db_iter;                   // points to the current DB
      std::shared_ptr<Aggregate> curr;  // current working Aggregate
      std::stack<std::pair<std::shared_ptr<Aggregate>, AggIter>> agg_stack;

      // add a new aggregate pair to the stack
      void push(std::shared_ptr<Aggregate> agg);

      // pop the aggregate stack
      void pop();
    };

    DBIterator begin() {return DBIterator(db_tree);}
    DBIterator end() {return DBIterator(db_tree, false);}

  private:
    bool readProgramProp(const bpt::ptree &);
    bool readMaterialProp(const bpt::ptree &);
    bool readSimulationParam(const bpt::ptree &sim_params_tree);
    bool readDesign(const bpt::ptree &subtree, const std::shared_ptr<Aggregate> &agg_parent);
    bool readItemTree(const bpt::ptree &subtree, const std::shared_ptr<Aggregate> &agg_parent);
    bool readDBDot(const bpt::ptree &subtree, const std::shared_ptr<Aggregate> &agg_parent);

    // Variables
    std::shared_ptr<Aggregate> db_tree;
    std::map<std::string, std::string> program_props;
    // std::map<std::string, std::string> material_props; TODO probably need a different structure for this
    std::map<std::string, std::string> sim_params;
  };
}

#endif
