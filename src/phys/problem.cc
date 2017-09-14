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
  return agg_stack.size() == 1 ? *this : ++(*this);
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
  curr = agg_stack.top().first; // update current to new top
  db_iter = curr->dbs.cend();   // don't reread dbs
}



// FILE HANDLING

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
