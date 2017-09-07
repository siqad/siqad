// @file:     problem.h
// @author:   Samuel
// @created:  2017.08.23
// @editted:  2017.08.23 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Definition of the problem - dbdot loc, material properties, etc.

#include <vector>
#include <stack>
#include <memory>
//#include <iterator>
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
      std::vector<std::shared_ptr<Aggregate>> aggs;
      std::vector<std::shared_ptr<DBDot>> dbs;
    };

    // electrode
    struct Electrode {
      float x,y;      // physical location in angstroms (top left corner)
      float dx,dy;    // width and height in angstroms
      float z;        // vertical distance from surface
      float voltage;  // voltage that the electrode is set to
    };

    // Iterator
    typedef std::vector<std::shared_ptr<DBDot>>::const_iterator DBIter;
    typedef std::vector<std::shared_ptr<Aggregate>>::const_iterator AggIter;

    // a constant iterator that iterates through all dangling bonds in the problem
    class DBIterator{
    public:
      explicit DBIterator(std::shared_ptr<Aggregate> root, bool begin=true){
        // some sort of check for valid Aggregate root
        if(begin){
          db_iter = root->dbs.cbegin();
          agg_stack.push(std::make_pair(root, root->aggs.cbegin()));
        }
        else{
          // right-populate agg_stack
          /*std::shared_ptr<Aggregate> agg = root;
          do{
            agg_stack.push(std::make_pair(agg, agg->aggs.cend()));
            agg = agg->aggs.empty() ? 0 : agg->aggs.back();
          }while(agg);
          db_iter = agg_stack.top().first->dbs.cend();*/
          db_iter = root->dbs.cend();
        }
      }

      //~Iterator() {delete agg_stack;};

      DBIterator& operator++(); // recursive part here
      bool operator==(const DBIterator &other) {return other.db_iter == db_iter;}
      bool operator!=(const DBIterator &other) {return other.db_iter != db_iter;}
      std::shared_ptr<DBDot> operator*() const {return *db_iter;}

    private:
      DBIter db_iter;
      std::stack<std::pair<std::shared_ptr<Aggregate>, AggIter>> agg_stack;
    };

    DBIterator begin() {return DBIterator(db_tree);}
    DBIterator end() {return DBIterator(db_tree, false);}

    // File Handling

    bool readProblem(const std::string &fname);
    bool readMaterialProp(rapidxml::xml_node<> *node);
    bool readSimulationParam(rapidxml::xml_node<> *node);
    bool readItemTree(rapidxml::xml_node<> *node, const std::shared_ptr<Aggregate>& agg_parent);
    bool readDBDot(rapidxml::xml_node<> *node, const std::shared_ptr<Aggregate>& agg_parent);

    static bool writeResult();

    // Variables
    std::shared_ptr<Aggregate> db_tree;
  };
}
