// @file:     siqadconn.cc
// @author:   Samuel
// @created:  2017.08.23
// @editted:  2017.08.23 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Convenient functions for interacting with SiQAD

#include "siqadconn.h"
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>


using namespace phys;


//CONSTRUCTOR
SiQADConnector::SiQADConnector(const std::string &eng_name_in,
  const std::string &input_path_in, const std::string &output_path_in)
{
  eng_name = eng_name_in;
  input_path = input_path_in;
  output_path = output_path_in;
  initProblem();
}

void SiQADConnector::setRequiredSimParam(std::string param_name)
{
  req_params.push_back(param_name);
}

std::map<std::string, std::string> SiQADConnector::getProperty(const std::string identifier)
{
  if (identifier == "Metal"){
    return metal_props;
  } else if (identifier == "Program"){
    return program_props;
  } else {
    std::map<std::string, std::string> empty;
    return empty;
  }
}

void SiQADConnector::initCollections()
{
  elec_col = new ElectrodeCollection(item_tree);
  db_col = new DBCollection(item_tree);
}

void SiQADConnector::setExport(std::string key, std::vector< std::pair< std::string, std::string > > &data_in)
{
    if (key == "db_loc"){
      setDBLocData(data_in);
    } else if (key == "db_charge"){
      setDBChargeData(data_in);
    }
}

void SiQADConnector::setExport(std::string key, std::vector< std::vector< std::string > > &data_in)
{
    if (key == "potential"){
      setElecPotentialData(data_in);
    } else if (key == "electrodes"){
      setElectrodeData(data_in);
    } else if (key == "db_pot"){
      setElectrodeData(data_in);
    }
}

void SiQADConnector::setElecPotentialData(std::vector<std::vector<std::string>> &data_in)
{
  setExportElecPotential(true);
  pot_data = data_in;
}

void SiQADConnector::setElectrodeData(std::vector<std::vector<std::string>> &data_in)
{
  setExportElectrode(true);
  elec_data = data_in;
}


void SiQADConnector::setDBPotData(std::vector< std::vector< std::string > > &data_in){
  setExportDBPot(true);
  db_pot_data = data_in;
}

void SiQADConnector::setDBLocData(std::vector< std::pair< std::string, std::string > > &data_in)
{
  setExportDBLoc(true);
  dbl_data = data_in;
}

void SiQADConnector::setDBChargeData(std::vector< std::pair< std::string, std::string > > &data_in)
{
  setExportDBChargeConfig(true);
  db_charge_data = data_in;
}

//What used to be problem
void SiQADConnector::initProblem(void)
{
  item_tree = std::make_shared<Aggregate>();
  expect_electrode = false;
  expect_db = false;
  expect_afm_path = false;
  return_code = 0;
  setExportElecPotential(false);
  setExportDBChargeConfig(false);
  setExportElectrode(false);
  setExportDBLoc(false);
  setExportDBPot(false);
  start_time = std::chrono::system_clock::now();
}

// aggregate
int Aggregate::size()
{
  int n_elecs=elecs.size();
  if(!aggs.empty())
    for(auto agg : aggs)
      n_elecs += agg->size();
  return n_elecs;
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

// FILE HANDLING
// parse problem XML, return true if successful
bool SiQADConnector::readProblem(void)
{
  std::cout << "Reading problem file: " << input_path << std::endl;

  bpt::ptree tree; // create empty property tree object
  bpt::read_xml(input_path, tree, bpt::xml_parser::no_comments); // parse the input file into property tree
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
  if(!readDesign(tree.get_child("dbdesigner.design"), item_tree))
    return false;

  // read layer properties
  std::cout << "Read layer properties" << std::endl;
  if(!readLayerProp(tree.get_child("dbdesigner")))
    return false;

  //return true;
  return false;
}

bool SiQADConnector::readProgramProp(const bpt::ptree &program_prop_tree)
{
  for (bpt::ptree::value_type const &v : program_prop_tree) {
    program_props.insert(std::map<std::string, std::string>::value_type(v.first, v.second.data()));
    std::cout << "ProgramProp: Key=" << v.first << ", Value=" << program_props[v.first] << std::endl;
  }
  return true;
}

bool SiQADConnector::readLayerProp(const bpt::ptree &top_tree)
{
  // if this were structured the same way as readDesign, then only the first layer_prop subtree would be read.
  // TODO: make this more general.
  for (bpt::ptree::value_type const &v : top_tree) {
    if(v.first == "layer_prop"){
      bool active = false;
      for (bpt::ptree::value_type const &layer_prop_tree : v.second) {
        // active will be true if we are in the Metal layer prop.
        if (layer_prop_tree.second.data() == "Metal") {
          active = true;
        } else if (layer_prop_tree.first == "name" && layer_prop_tree.second.data() != "Metal"){
          active = false;
        }
        if (active == true) {
          metal_props.insert(std::map<std::string, std::string>::value_type(layer_prop_tree.first, layer_prop_tree.second.data()));
          std::cout << "MetalProp: Key=" << layer_prop_tree.first << ", Value=" << metal_props[layer_prop_tree.first] << std::endl;
        }
      }
    }
  }
  return true;
}


bool SiQADConnector::readMaterialProp(const bpt::ptree &material_prop_tree)
{
  (void)material_prop_tree; // function to be implemented, suppress variable unused warning for now
  return true;
}

bool SiQADConnector::readSimulationParam(const bpt::ptree &sim_params_tree)
{
  for (bpt::ptree::value_type const &v : sim_params_tree) {
    sim_params.insert(std::map<std::string, std::string>::value_type(v.first, v.second.data()));
    std::cout << "SimParam: Key=" << v.first << ", Value=" << sim_params[v.first] << std::endl;
  }
  return true;
}

bool SiQADConnector::readDesign(const bpt::ptree &subtree, const std::shared_ptr<Aggregate> &agg_parent)
{
  std::cout << "Beginning to read design" << std::endl;
  std::cout << expect_electrode << expect_db << expect_afm_path << std::endl;
  for (bpt::ptree::value_type const &layer_tree : subtree) {
    std::string layer_type = layer_tree.second.get<std::string>("<xmlattr>.type");
    if ((!layer_type.compare("DB")) && (expect_db)) {
      std::cout << "Encountered node " << layer_tree.first << " with type " << layer_type << ", entering" << std::endl;
      readItemTree(layer_tree.second, agg_parent);
    } else if ( (!layer_type.compare("Electrode")) && (expect_electrode) ) {
      std::cout << "Encountered node " << layer_tree.first << " with type " << layer_type << ", entering" << std::endl;
      readItemTree(layer_tree.second, agg_parent);
    } else {
      std::cout << "Encountered node " << layer_tree.first << " with type " << layer_type << ", no defined action for this layer. Skipping." << std::endl;
    }
  }
  return true;
}

bool SiQADConnector::readItemTree(const bpt::ptree &subtree, const std::shared_ptr<Aggregate> &agg_parent)
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
  return true;
}

bool SiQADConnector::readElectrode(const bpt::ptree &subtree, const std::shared_ptr<Aggregate> &agg_parent)
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

  return true;
}

bool SiQADConnector::readDBDot(const bpt::ptree &subtree, const std::shared_ptr<Aggregate> &agg_parent)
{
  float x, y;

  // read x and y from XML stream
  // elec = subtree.get<float>("elec");
  x = subtree.get<float>("physloc.<xmlattr>.x");
  y = subtree.get<float>("physloc.<xmlattr>.y");

  agg_parent->dbs.push_back(std::make_shared<DBDot>(x,y));

  // std::cout << "DBDot created with x=" << agg_parent->dbs.back()->x << ", y=" << agg_parent->dbs.back()->y << ", elec=" << agg_parent->dbs.back()->elec << std::endl;
  std::cout << "DBDot created with x=" << agg_parent->dbs.back()->x << ", y=" << agg_parent->dbs.back()->y << std::endl;

  return true;
}

void SiQADConnector::writeResultsXml()
{
  std::cout << "SiQADConnector::writeResultsXml()" << std::endl;
  // define major XML nodes
  boost::property_tree::ptree tree;
  boost::property_tree::ptree node_root;       // <sim_out>
  boost::property_tree::ptree node_eng_info;   // <eng_info>
  boost::property_tree::ptree node_sim_params; // <sim_params>
  boost::property_tree::ptree node_electrode;  // <electrode>
  boost::property_tree::ptree node_potential_map;  // <potential>
  boost::property_tree::ptree node_db_potentials;  // <db_potential>
  boost::property_tree::ptree node_physloc;    // <physloc>
  boost::property_tree::ptree node_elec_dist;  // <elec_dist>

  std::cout << "Write results to XML..." << std::endl;
  // NOTE in the future, there's probably a range of stuff that can be exported.
  // for now, only export charge config

  // eng_info
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
  node_root.add_child("eng_info", node_eng_info);

  // sim_params
  // TODO

  // DB locations
  if (export_db_loc){
    std::cout << "Exporting DB locations..." << std::endl;
    for (unsigned int i = 0; i < dbl_data.size(); i++){
      bpt::ptree node_dbdot;
      node_dbdot.put("<xmlattr>.x", dbl_data[i].first.c_str());
      node_dbdot.put("<xmlattr>.y", dbl_data[i].second.c_str());
      node_physloc.add_child("dbdot", node_dbdot);
    }
    node_root.add_child("physloc", node_physloc);
  }

  //DB elec distributions
  if(export_db_charge_config){
    std::cout << "Exporting DB electron distrbutions..." << std::endl;
    for (unsigned int i = 0; i < db_charge_data.size(); i++){
      bpt::ptree node_dist;
      node_dist.put("", db_charge_data[i].first);
      node_dist.put("<xmlattr>.energy", db_charge_data[i].second);
      node_elec_dist.add_child("dist", node_dist);
    }
    node_root.add_child("elec_dist", node_elec_dist);
  }
  // electrode
  if (export_electrode){
    std::cout << "Exporting electrode data..." << std::endl;
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
    node_root.add_child("electrode", node_electrode);
  }

  //electric potentials
  if (export_elec_potential){
    std::cout << "Exporting electric potential data..." << std::endl;
    for (unsigned int i = 0; i < pot_data.size(); i++){
      boost::property_tree::ptree node_potential_val;
      node_potential_val.put("<xmlattr>.x", pot_data[i][0].c_str());
      node_potential_val.put("<xmlattr>.y", pot_data[i][1].c_str());
      node_potential_val.put("<xmlattr>.val", pot_data[i][2].c_str());
      node_potential_map.add_child("potential_val", node_potential_val);
    }
    node_root.add_child("potential_map", node_potential_map);
  }

  //potentials at db locations
  if (export_db_pot){
    bpt::ptree node_dbdots;
    std::cout << "Exporting electric potential data at dbs..." << std::endl;
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
    node_root.add_child("dbdots", node_dbdots);
  }
  // if (export_db_loc){
  //   std::cout << "Exporting DB locations..." << std::endl;
  //   for (unsigned int i = 0; i < dbl_data.size(); i++){
  //     bpt::ptree node_dbdot;
  //     node_dbdot.put("<xmlattr>.x", dbl_data[i].first.c_str());
  //     node_dbdot.put("<xmlattr>.y", dbl_data[i].second.c_str());
  //     node_physloc.add_child("dbdot", node_dbdot);
  //   }
  //   node_root.add_child("physloc", node_physloc);
  // }



  tree.add_child("sim_out", node_root);
  // write to file
  boost::property_tree::write_xml(output_path, tree, std::locale(), boost::property_tree::xml_writer_make_settings<std::string>(' ',4));

  std::cout << "Write to XML complete." << std::endl;
}
