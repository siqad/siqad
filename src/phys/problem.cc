// @file:     problem.cc
// @author:   Samuel
// @created:  2017.08.23
// @editted:  2017.08.23 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Definition of the problem - dbdot loc, material properties, etc.

#include "problem.h"

using namespace phys;

// Constructors
Problem::Problem(const std::string &fname) 
{
  initProblem();
  readProblem(fname);
}

void Problem::initProblem() {
  db_tree = std::make_shared<Problem::Aggregate>();
}



// Iterator
Problem::DBIterator& Problem::DBIterator::operator++(){
  std::shared_ptr<Aggregate> curr = agg_stack.top().first;

  std::cout << "entered DBIterator++" << std::endl;

  std::cout << "*db_iter (before): " << *db_iter << std::endl;
  std::cout << "addr of curr->dbs.cend(): " << &*(curr->dbs.cend()) << std::endl;

  // exhaust current DBs first
  if(db_iter != curr->dbs.cend()) {
    std::cout << "looking at DBs" << std::endl;
    ++db_iter;
    std::cout << "*db_iter (after): " << *db_iter << std::endl;
    return db_iter != curr->dbs.cend() ? *this : ++(*this);
  }
  // look at children aggregates
  else if(agg_stack.top().second != curr->aggs.cend()) {
    // add next aggregate to stack
    std::cout << "looking at children aggregates" << std::endl;
    curr = *agg_stack.top().second;
    db_iter = curr->dbs.cbegin();
    ++agg_stack.top().second; // AggIter should point to the next agg
    agg_stack.push(make_pair(curr, curr->aggs.cbegin()));
    return ++(*this);
  }
  // children aggregates and DBs exhausted, pop out this element
  else {
    std::cout << "agg and dbdots exhausted" << std::endl;
    agg_stack.pop();
    assert(!agg_stack.empty());
    curr = agg_stack.top().first;
    db_iter = curr->dbs.cend(); // prevent re-reading of parent dbs
    std::cout << "addr of curr->dbs.cend(): " << &*(curr->dbs.cend()) << ". addr of db_iter: " << &*db_iter << std::endl;
    return agg_stack.size() == 1 ? *this : ++(*this); // if there's only one elem in stack, iteration complete
  }
}




// File handling

// parse problem XML, return true if successful
bool Problem::readProblem(const std::string &fname)
{
  std::cout << "Reading problem file: " << fname << std::endl;

  std::ifstream in_file(fname);
  rapidxml::xml_document<> xmldoc;
  rapidxml::xml_node<> *root_node;

  if(!in_file){
    std::cout << "Failed to open file " << fname << std::endl;
    return false;
  }


  // read file to buffer and close
  std::stringstream buffer;
  buffer << in_file.rdbuf();
  in_file.close();
  std::string in_content(buffer.str());

  //std::cout << "Buffer content: " << in_content << std::endl;

  // parse XML
  std::cout << "Parse XML" << std::endl;
  xmldoc.parse<0>(&in_content[0]);
  root_node = xmldoc.first_node(); // get root node of input file
  std::cout << "Root node name: " << root_node->name() << std::endl;

  // material parameters
  std::cout << "Read material properties" << std::endl;
  readMaterialProp(root_node->first_node("material_prop"));

  // simulation parameters
  std::cout << "Read simulation parameters" << std::endl;
  readSimulationParam(root_node->first_node("sim_param"));

  // item tree
  std::cout << "Read DB tree" << std::endl;
  readItemTree(root_node->first_node("db_tree"), db_tree);
  std::cout << "db_tree pointer " << db_tree.get() << std::endl;

  return true;
}


bool Problem::readMaterialProp(rapidxml::xml_node<> *node)
{
  for(rapidxml::xml_node<> *material_node = node->first_node(); material_node; material_node = material_node->next_sibling()){
    std::cout << "Material property: " << material_node->name() << std::endl;
    // TODO material vector, containing structs of material properties
  }
  return true;
}


bool Problem::readSimulationParam(rapidxml::xml_node<> *node)
{
  for(rapidxml::xml_node<> *sim_node = node->first_node(); sim_node; sim_node = sim_node->next_sibling()){
    // TODO just add everything into sim_param dictionary
  }
  return true;
}


bool Problem::readItemTree(rapidxml::xml_node<> *node, const std::shared_ptr<Aggregate>& agg_parent)
{
  for(rapidxml::xml_node<> *item_node = node->first_node(); item_node; item_node = item_node->next_sibling()){
    std::string item_name = item_node->name();

    if(!item_name.compare("aggregate")){
      // add aggregate child to tree
      agg_parent->aggs.push_back(std::make_shared<Aggregate>());
      readItemTree(item_node, agg_parent->aggs.back());
    }
    else if(!item_name.compare("dbdot")) {
      // add DBDot to tree
      readDBDot(item_node, agg_parent);
    }
    else
      std::cout << "Encountered unknown item_node: " << item_node->name() << std::endl;
  }
  return true;
}


bool Problem::readDBDot(rapidxml::xml_node<> *node, const std::shared_ptr<Aggregate>& agg_parent)
{
  float x,y;

  // read x and y from XML stream
  x = std::stof(node->first_attribute("x")->value());
  y = std::stof(node->first_attribute("y")->value());

  agg_parent->dbs.push_back(std::make_shared<DBDot>(x,y));

  std::cout << "DBDot created with x=" << agg_parent->dbs.back()->x << ", y=" << agg_parent->dbs.back()->y << std::endl;

  return true;
}

// write result to XML, return true if successful
bool Problem::writeResult()
{
  return true;
}

