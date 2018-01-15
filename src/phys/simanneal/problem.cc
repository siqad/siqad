// @file:     problem.cc
// @author:   Samuel
// @created:  2017.08.23
// @editted:  2017.08.23 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Definition of the problem - dbdot loc, material properties, etc.

#include "problem.h"

using namespace phys;

// Constructor
Problem::Problem(const std::string &fname)
{
  initProblem();
  readProblem(fname);
}

void Problem::initProblem() 
{
  db_tree = std::make_shared<Problem::Aggregate>();
}



// DB & AGG

// aggregate
int Problem::Aggregate::size()
{
  int n_dbs=dbs.size();
  if(!aggs.empty())
    for(auto agg : aggs)
      n_dbs += agg->size();
  return n_dbs;
}



// ITERATOR

Problem::DBIterator::DBIterator(std::shared_ptr<Aggregate> root, bool begin)
{
  if(begin){
    // keep finding deeper aggregates until one that contains dbs is found
    while(root->dbs.empty() && !root->aggs.empty()) {
      push(root);
      root = root->aggs.front();
    }
    push(root);
  }
  else{
    db_iter = root->dbs.cend();
  }
}

Problem::DBIterator& Problem::DBIterator::operator++()
{
  // exhaust the current Aggregate DBs first
  if(db_iter != curr->dbs.cend())
    return ++db_iter != curr->dbs.cend() ? *this : ++(*this);

  // if available, push the next aggregate onto the stack
  if(agg_stack.top().second != curr->aggs.cend()){
    push(*agg_stack.top().second);
    return db_iter != curr->dbs.cend() ? *this : ++(*this);
  }

  // aggregate is complete, pop off stack
  pop();
  return agg_stack.size() == 0 ? *this : ++(*this);
}

void Problem::DBIterator::push(std::shared_ptr<Aggregate> agg)
{
  if(!agg_stack.empty())
    ++agg_stack.top().second;
  agg_stack.push(std::make_pair(agg, agg->aggs.cbegin()));
  db_iter = agg->dbs.cbegin();
  curr = agg;
}

void Problem::DBIterator::pop()
{
  agg_stack.pop();              // pop complete aggregate off stack
  if(agg_stack.size() > 0){
    curr = agg_stack.top().first; // update current to new top
    db_iter = curr->dbs.cend();   // don't reread dbs
  }
}



// FILE HANDLING

// parse problem XML, return true if successful
bool Problem::readProblem(const std::string &fname)
{
  std::cout << "Reading problem file: " << fname << std::endl;

  bpt::ptree tree; // create empty property tree object
  bpt::read_xml(fname, tree, bpt::xml_parser::no_comments); // parse the input file into property tree
  // TODO catch read error exception

  // parse XML

  // read program properties
  // TODO read program node

  // read material properties
  // TODO read material_prop node

  // read simulation parameters
  std::cout << "Read simulation parameters" << std::endl;
  if(!readSimulationParam(tree.get_child("dbdesigner.sim_params")))
    return false;

  // read items
  std::cout << "Read items tree" << std::endl;
  if(!readDesign(tree.get_child("dbdesigner.design"), db_tree))
    return false;


  //return true;
  return false;
}


bool Problem::readProgramProp(const bpt::ptree &program_prop_tree)
{
  for (bpt::ptree::value_type const &v : program_prop_tree) {
    program_props.insert(std::map<std::string, std::string>::value_type(v.first, v.second.data()));
    std::cout << "ProgramProp: Key=" << v.first << ", Value=" << program_props[v.first] << std::endl;
  }
  return true;
}

bool Problem::readMaterialProp(const bpt::ptree &material_prop_tree)
{
  (void)material_prop_tree; // function to be implemented, suppress variable unused warning for now
  return true;
}


bool Problem::readSimulationParam(const bpt::ptree &sim_params_tree)
{
  for (bpt::ptree::value_type const &v : sim_params_tree) {
    sim_params.insert(std::map<std::string, std::string>::value_type(v.first, v.second.data()));
    std::cout << "SimParam: Key=" << v.first << ", Value=" << sim_params[v.first] << std::endl;
  }
  return true;
}


bool Problem::readDesign(const bpt::ptree &subtree, const std::shared_ptr<Aggregate> &agg_parent)
{
  std::cout << "Beginning to read design" << std::endl;
  for (bpt::ptree::value_type const &layer_tree : subtree) {
    std::string layer_type = layer_tree.second.get<std::string>("<xmlattr>.type");
    if (!layer_type.compare("DB")) {
      std::cout << "Encountered node " << layer_tree.first << " with type " << layer_type << ", entering" << std::endl;
      readItemTree(layer_tree.second, agg_parent);
    } else if (!layer_type.compare("Electrode")) {
      // TODO parse electrode code
      std::cout << "TODO write code for parsing electrodes" << std::endl;
    } else {
      std::cout << "Encountered node " << layer_tree.first << " with type " << layer_type << ", no defined action for this layer. Skipping." << std::endl;
    }
  }
  return true;
}


bool Problem::readItemTree(const bpt::ptree &subtree, const std::shared_ptr<Aggregate> &agg_parent)
{
  for (bpt::ptree::value_type const &item_tree : subtree) {
    std::string item_name = item_tree.first;

    if (!item_name.compare("aggregate")) {
      // add aggregate child to tree
      agg_parent->aggs.push_back(std::make_shared<Aggregate>());
      readItemTree(item_tree.second, agg_parent->aggs.back());
    } else if (!item_name.compare("dbdot")) {
      // add DBDot to tree
      readDBDot(item_tree.second, agg_parent);
    } else {
      std::cout << "Encountered unknown item node: " << item_tree.first << std::endl;
    }
  }
  return true;
}


bool Problem::readDBDot(const bpt::ptree &subtree, const std::shared_ptr<Aggregate> &agg_parent)
{
  float x, y, elec;

  // read x and y from XML stream
  elec = subtree.get<float>("elec");
  x = subtree.get<float>("physloc.<xmlattr>.x");
  y = subtree.get<float>("physloc.<xmlattr>.y");

  agg_parent->dbs.push_back(std::make_shared<DBDot>(x,y,elec));

  std::cout << "DBDot created with x=" << agg_parent->dbs.back()->x << ", y=" << agg_parent->dbs.back()->y << ", elec=" << agg_parent->dbs.back()->elec << std::endl;
  
  return true;
}
