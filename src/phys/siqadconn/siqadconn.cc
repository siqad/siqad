// @file:     siqadconn.cc
// @author:   Samuel
// @created:  2017.08.23
// @editted:  2018.06.28 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Convenient functions for interacting with SiQAD

#include "siqadconn.h"
#include <iostream>
#include <stdexcept>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>


using namespace phys;


//CONSTRUCTOR
SiQADConnector::SiQADConnector(const std::string &eng_name,
  const std::string &input_path, const std::string &output_path)
  : eng_name(eng_name), input_path(input_path), output_path(output_path)
{
  // initialize variables
  item_tree = std::make_shared<Aggregate>();
  start_time = std::chrono::system_clock::now();
  elec_col = new ElectrodeCollection(item_tree);
  db_col = new DBCollection(item_tree);

  // read problem from input_path
  readProblem(input_path);
}

void SiQADConnector::setExport(std::string type, std::vector< std::pair< std::string, std::string > > &data_in)
{
  if (type == "db_loc")
    dbl_data = data_in;
  else
    throw std::invalid_argument(std::string("No candidate for export type '") +
        type + std::string("' with class std::vector<std::pair<std::string, std::string>>"));
}

void SiQADConnector::setExport(std::string type, std::vector< std::vector< std::string > > &data_in)
{
  if (type == "potential")
    pot_data = data_in;
  else if (type == "electrodes")
    elec_data = data_in;
  else if (type == "db_pot")
    db_pot_data = data_in;
  else if (type == "db_charge")
    db_charge_data = data_in;
  else
    throw std::invalid_argument(std::string("No candidate for export type '") +
        type + std::string("' with class std::vector<std::vector<std::string>>"));
}



// FILE HANDLING
// parse problem XML, return true if successful
void SiQADConnector::readProblem(const std::string &path)
{
  std::cout << "Reading problem file: " << input_path << std::endl;

  bpt::ptree tree; // create empty property tree object
  bpt::read_xml(path, tree, bpt::xml_parser::no_comments); // parse the input file into property tree

  // parse XML

  // read program properties
  // TODO read program node

  // read simulation parameters
  std::cout << "Read simulation parameters" << std::endl;
  readSimulationParam(tree.get_child("siqad.sim_params"));

  // read layer properties
  std::cout << "Read layer properties" << std::endl;
  readLayers(tree.get_child("siqad.layers"));

  // read items
  std::cout << "Read items tree" << std::endl;
  readDesign(tree.get_child("siqad.design"), item_tree);
}

void SiQADConnector::readProgramProp(const bpt::ptree &program_prop_tree)
{
  for (bpt::ptree::value_type const &v : program_prop_tree) {
    program_props.insert(std::map<std::string, std::string>::value_type(v.first, v.second.data()));
    std::cout << "ProgramProp: Key=" << v.first << ", Value=" << program_props[v.first] << std::endl;
  }
}

void SiQADConnector::readLayers(const bpt::ptree &layer_prop_tree)
{
  // if this were structured the same way as readDesign, then only the first layer_prop subtree would be read.
  // TODO: make this more general.
  for (bpt::ptree::value_type const &v : layer_prop_tree)
    readLayerProp(v.second);
}

void SiQADConnector::readLayerProp(const bpt::ptree &layer_node)
{
  Layer lay;
  lay.name = layer_node.get<std::string>("name");
  lay.type = layer_node.get<std::string>("type");
  lay.zoffset = layer_node.get<float>("zoffset");
  lay.zheight = layer_node.get<float>("zheight");

  layers.push_back(lay);
  std::cout << "Retrieved layer " << lay.name << " of type " << lay.type << std::endl;
}


void SiQADConnector::readSimulationParam(const bpt::ptree &sim_params_tree)
{
  for (bpt::ptree::value_type const &v : sim_params_tree) {
    sim_params.insert(std::map<std::string, std::string>::value_type(v.first, v.second.data()));
    std::cout << "SimParam: Key=" << v.first << ", Value=" << sim_params[v.first] << std::endl;
  }
}

void SiQADConnector::readDesign(const bpt::ptree &subtree, const std::shared_ptr<Aggregate> &agg_parent)
{
  std::cout << "Beginning to read design" << std::endl;
  for (bpt::ptree::value_type const &layer_tree : subtree) {
    std::string layer_type = layer_tree.second.get<std::string>("<xmlattr>.type");
    if ((!layer_type.compare("DB"))) {
      std::cout << "Encountered node " << layer_tree.first << " with type " << layer_type << ", entering" << std::endl;
      readItemTree(layer_tree.second, agg_parent);
    } else if ( (!layer_type.compare("Electrode"))) {
      std::cout << "Encountered node " << layer_tree.first << " with type " << layer_type << ", entering" << std::endl;
      readItemTree(layer_tree.second, agg_parent);
    } else {
      std::cout << "Encountered node " << layer_tree.first << " with type " << layer_type << ", no defined action for this layer. Skipping." << std::endl;
    }
  }
}

void SiQADConnector::readItemTree(const bpt::ptree &subtree, const std::shared_ptr<Aggregate> &agg_parent)
{
  for (bpt::ptree::value_type const &item_tree : subtree) {
    std::string item_name = item_tree.first;
    std::cout << "item_name: " << item_name << std::endl;
    if (!item_name.compare("aggregate")) {
      // add aggregate child to tree
      agg_parent->aggs.push_back(std::make_shared<Aggregate>());
      readItemTree(item_tree.second, agg_parent->aggs.back());
    } else if (!item_name.compare("dbdot")) {
      // add DBDot to tree
      readDBDot(item_tree.second, agg_parent);
    } else if (!item_name.compare("electrode")) {
      // add Electrode to tree
      readElectrode(item_tree.second, agg_parent);
    } else {
      std::cout << "Encountered unknown item node: " << item_tree.first << std::endl;
    }
  }
}

void SiQADConnector::readElectrode(const bpt::ptree &subtree, const std::shared_ptr<Aggregate> &agg_parent)
{
  double x1, x2, y1, y2, pixel_per_angstrom, potential, phase;
  int layer_id, electrode_type;
  // read values from XML stream
  layer_id = subtree.get<int>("layer_id");
  potential = subtree.get<double>("property_map.potential.val");
  phase = subtree.get<double>("property_map.phase.val");
  std::string electrode_type_s = subtree.get<std::string>("property_map.type.val");
  if (!electrode_type_s.compare("fixed")){
    electrode_type = 0;
  } else if (!electrode_type_s.compare("clocked")) {
    electrode_type = 1;
  }
  pixel_per_angstrom = subtree.get<double>("pixel_per_angstrom");
  x1 = subtree.get<double>("dim.<xmlattr>.x1");
  x2 = subtree.get<double>("dim.<xmlattr>.x2");
  y1 = subtree.get<double>("dim.<xmlattr>.y1");
  y2 = subtree.get<double>("dim.<xmlattr>.y2");
  agg_parent->elecs.push_back(std::make_shared<Electrode>(layer_id,x1,x2,y1,y2,potential,phase,electrode_type,pixel_per_angstrom));

  std::cout << "Electrode created with x1=" << agg_parent->elecs.back()->x1 << ", y1=" << agg_parent->elecs.back()->y1 <<
    ", x2=" << agg_parent->elecs.back()->x2 << ", y2=" << agg_parent->elecs.back()->y2 <<
    ", potential=" << agg_parent->elecs.back()->potential << std::endl;
}

void SiQADConnector::readDBDot(const bpt::ptree &subtree, const std::shared_ptr<Aggregate> &agg_parent)
{
  float x, y;
  int n, m, l;

  // read x and y physical locations
  x = subtree.get<float>("physloc.<xmlattr>.x");
  y = subtree.get<float>("physloc.<xmlattr>.y");

  // read n, m and l lattice coordinates
  n = subtree.get<int>("latcoord.<xmlattr>.n");
  m = subtree.get<int>("latcoord.<xmlattr>.m");
  l = subtree.get<int>("latcoord.<xmlattr>.l");

  agg_parent->dbs.push_back(std::make_shared<DBDot>(x, y, n, m, l));

  std::cout << "DBDot created with x=" << agg_parent->dbs.back()->x
            << ", y=" << agg_parent->dbs.back()->y
            << ", n=" << agg_parent->dbs.back()->n
            << ", m=" << agg_parent->dbs.back()->m
            << ", l=" << agg_parent->dbs.back()->l
            << std::endl;
}

void SiQADConnector::writeResultsXml()
{
  std::cout << "SiQADConnector::writeResultsXml()" << std::endl;

  boost::property_tree::ptree node_root;

  std::cout << "Write results to XML..." << std::endl;

  // eng_info
  node_root.add_child("eng_info", engInfoPropertyTree());

  // sim_params
  node_root.add_child("sim_params", simParamsPropertyTree());

  // DB locations
  if (!dbl_data.empty())
    node_root.add_child("physloc", dbLocPropertyTree());

  // DB electron distributions
  if(!db_charge_data.empty())
    node_root.add_child("elec_dist", dbChargePropertyTree());

  // electrode
  if (!elec_data.empty())
    node_root.add_child("electrode", electrodePropertyTree());

  //electric potentials
  if (!pot_data.empty())
    node_root.add_child("potential_map", potentialPropertyTree());

  //potentials at db locations
  if (!db_pot_data.empty())
    node_root.add_child("dbdots", dbPotentialPropertyTree());

  // write full tree to file
  boost::property_tree::ptree tree;
  tree.add_child("sim_out", node_root);
  boost::property_tree::write_xml(output_path, tree, std::locale(), boost::property_tree::xml_writer_make_settings<std::string>(' ',4));

  std::cout << "Write to XML complete." << std::endl;
}

bpt::ptree SiQADConnector::engInfoPropertyTree()
{
  boost::property_tree::ptree node_eng_info;
  node_eng_info.put("engine", eng_name);
  node_eng_info.put("version", "TBD"); // TODO real version
  node_eng_info.put("return_code", std::to_string(return_code).c_str());

  //get timing information
  end_time = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end_time-start_time;
  std::time_t end = std::chrono::system_clock::to_time_t(end_time);
  char* end_c_str = std::ctime(&end);
  *std::remove(end_c_str, end_c_str+strlen(end_c_str), '\n') = '\0'; // removes _all_ new lines from the cstr
  node_eng_info.put("timestamp", end_c_str);
  node_eng_info.put("time_elapsed_s", std::to_string(elapsed_seconds.count()).c_str());

  return node_eng_info;
}

bpt::ptree SiQADConnector::simParamsPropertyTree()
{
  bpt::ptree node_sim_params;
  for (std::pair<std::string, std::string> param : sim_params)
    node_sim_params.put(param.first, param.second);
  return node_sim_params;
}

bpt::ptree SiQADConnector::dbLocPropertyTree()
{
  bpt::ptree node_physloc;
  for (unsigned int i = 0; i < dbl_data.size(); i++){
    bpt::ptree node_dbdot;
    node_dbdot.put("<xmlattr>.x", dbl_data[i].first.c_str());
    node_dbdot.put("<xmlattr>.y", dbl_data[i].second.c_str());
    node_physloc.add_child("dbdot", node_dbdot);
  }
  return node_physloc;
}

bpt::ptree SiQADConnector::dbChargePropertyTree()
{
  bpt::ptree node_elec_dist;
  for (unsigned int i = 0; i < db_charge_data.size(); i++){
    bpt::ptree node_dist;
    node_dist.put("", db_charge_data[i][0]);
    node_dist.put("<xmlattr>.energy", db_charge_data[i][1]);
    node_dist.put("<xmlattr>.count", db_charge_data[i][2]);
    node_elec_dist.add_child("dist", node_dist);
  }
  return node_elec_dist;
}

bpt::ptree SiQADConnector::electrodePropertyTree()
{
  bpt::ptree node_electrode;
  for (unsigned int i = 0; i < elec_data.size(); i++){
    boost::property_tree::ptree node_dim;
    node_dim.put("<xmlattr>.x1", elec_data[i][0].c_str());
    node_dim.put("<xmlattr>.y1", elec_data[i][1].c_str());
    node_dim.put("<xmlattr>.x2", elec_data[i][2].c_str());
    node_dim.put("<xmlattr>.y2", elec_data[i][3].c_str());
    node_electrode.add_child("dim", node_dim);
    boost::property_tree::ptree node_pot;
    node_pot.put("", elec_data[i][4].c_str());
    node_electrode.add_child("potential", node_pot);
  }
  return node_electrode;
}

bpt::ptree SiQADConnector::potentialPropertyTree()
{
  bpt::ptree node_potential_map;
  for (unsigned int i = 0; i < pot_data.size(); i++){
    boost::property_tree::ptree node_potential_val;
    node_potential_val.put("<xmlattr>.x", pot_data[i][0].c_str());
    node_potential_val.put("<xmlattr>.y", pot_data[i][1].c_str());
    node_potential_val.put("<xmlattr>.val", pot_data[i][2].c_str());
    node_potential_map.add_child("potential_val", node_potential_val);
  }
  return node_potential_map;
}

bpt::ptree SiQADConnector::dbPotentialPropertyTree()
{
  bpt::ptree node_dbdots;
  for (unsigned int i = 0; i < db_pot_data.size(); i++){
    bpt::ptree node_dbdot;
    bpt::ptree node_physloc;
    node_physloc.put("<xmlattr>.x", db_pot_data[i][0].c_str());
    node_physloc.put("<xmlattr>.y", db_pot_data[i][1].c_str());
    node_dbdot.add_child("physloc", node_physloc);
    boost::property_tree::ptree node_db_pot;
    node_db_pot.put("", db_pot_data[i][2].c_str());
    node_dbdot.add_child("potential", node_db_pot);
    node_dbdots.add_child("dbdot", node_dbdot);
  }
  return node_dbdots;
}

//DB ITERATOR

DBIterator::DBIterator(std::shared_ptr<Aggregate> root, bool begin)
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

DBIterator& DBIterator::operator++()
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

void DBIterator::push(std::shared_ptr<Aggregate> agg)
{
  if(!agg_stack.empty())
    ++agg_stack.top().second;
  agg_stack.push(std::make_pair(agg, agg->aggs.cbegin()));
  db_iter = agg->dbs.cbegin();
  curr = agg;
}

void DBIterator::pop()
{
  agg_stack.pop();              // pop complete aggregate off stack
  if(agg_stack.size() > 0){
    curr = agg_stack.top().first; // update current to new top
    db_iter = curr->dbs.cend();   // don't reread dbs
  }
}


// ELEC ITERATOR
ElecIterator::ElecIterator(std::shared_ptr<Aggregate> root, bool begin)
{
  if(begin){
    // keep finding deeper aggregates until one that contains dbs is found
    while(root->elecs.empty() && !root->aggs.empty()) {
      push(root);
      root = root->aggs.front();
    }
    push(root);
  }
  else{
    elec_iter = root->elecs.cend();
  }
}

ElecIterator& ElecIterator::operator++()
{
  // exhaust the current Aggregate DBs first
  if(elec_iter != curr->elecs.cend())
    return ++elec_iter != curr->elecs.cend() ? *this : ++(*this);

  // if available, push the next aggregate onto the stack
  if(agg_stack.top().second != curr->aggs.cend()){
    push(*agg_stack.top().second);
    return elec_iter != curr->elecs.cend() ? *this : ++(*this);
  }

  // aggregate is complete, pop off stack
  pop();
  return agg_stack.size() == 0 ? *this : ++(*this);
}

void ElecIterator::push(std::shared_ptr<Aggregate> agg)
{
  if(!agg_stack.empty())
    ++agg_stack.top().second;
  agg_stack.push(std::make_pair(agg, agg->aggs.cbegin()));
  elec_iter = agg->elecs.cbegin();
  curr = agg;
}

void ElecIterator::pop()
{
  agg_stack.pop();              // pop complete aggregate off stack
  if(agg_stack.size() > 0){
    curr = agg_stack.top().first; // update current to new top
    elec_iter = curr->elecs.cend();   // don't reread dbs
  }
}


// AGGREGATE
int Aggregate::size()
{
  int n_elecs=elecs.size();
  if(!aggs.empty())
    for(auto agg : aggs)
      n_elecs += agg->size();
  return n_elecs;
}
