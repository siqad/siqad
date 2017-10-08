// @file:     phys_engine.cc
// @author:   Samuel
// @created:  2017.08.23
// @editted:  2017.08.23 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Base class for physics engines

#include "phys_engine.h"

using namespace phys;

// CONSTRUCTOR
PhysicsEngine::PhysicsEngine(const std::string &eng_nm, const std::string &i_path, const std::string &o_path)
{
  eng_name = eng_nm;
  problem.readProblem(i_path);
  out_path = o_path;
}

void PhysicsEngine::writeResultsXML()
{
  std::cout << "Write results to XML..." << std::endl;
  // NOTE in the future, there's probably a range of stuff that can be exported.
  // for now, only export charge config
  std::ofstream of(out_path);
  // TODO test that the output file path works

  // xml declaration
  rapidxml::xml_document<> doc;
  rapidxml::xml_node<>* decl = doc.allocate_node(rapidxml::node_declaration);
  decl->append_attribute(doc.allocate_attribute("version", "1.0"));
  decl->append_attribute(doc.allocate_attribute("encoding", "UTF-8"));
  doc.append_node(decl);

  // major nodes
  //rapidxml::xml_node<>* nd_root, nd_eng_info, nd_sim_param, nd_physloc, nd_elec_dist;
  rapidxml::xml_node<>* nd_root = doc.allocate_node(rapidxml::node_element, "sim_out");
  rapidxml::xml_node<>* nd_eng_info = doc.allocate_node(rapidxml::node_element, "eng_info");
  rapidxml::xml_node<>* nd_sim_param = doc.allocate_node(rapidxml::node_element, "sim_param");
  rapidxml::xml_node<>* nd_physloc = doc.allocate_node(rapidxml::node_element, "physloc");
  rapidxml::xml_node<>* nd_elec_dist = doc.allocate_node(rapidxml::node_element, "elec_dist");
  doc.append_node(nd_root);
  nd_root->append_node(nd_eng_info);
  nd_root->append_node(nd_sim_param);
  nd_root->append_node(nd_physloc);
  nd_root->append_node(nd_elec_dist);

  // eng_info
  //rapidxml::xml_node<>* nd_eng_name, nd_version;
  rapidxml::xml_node<>* nd_engine_name = doc.allocate_node(rapidxml::node_element, "engine", eng_name.c_str());
  rapidxml::xml_node<>* nd_version = doc.allocate_node(rapidxml::node_element, "version", "TBD");
  nd_eng_info->append_node(nd_engine_name);
  nd_eng_info->append_node(nd_version);

  // sim_param
  // TODO implement a struct in phys::Problem storing all user sim params, write all that
  
  // physloc
  for(auto dbl : db_loc) {
    rapidxml::xml_node<>* nd_dbl = doc.allocate_node(rapidxml::node_element, "dbdot");
    char *dbl_x = doc.allocate_string(std::to_string(dbl.first).c_str());
    char *dbl_y = doc.allocate_string(std::to_string(dbl.second).c_str());
    nd_dbl->append_attribute(doc.allocate_attribute("x",dbl_x));
    nd_dbl->append_attribute(doc.allocate_attribute("y",dbl_y));
    nd_physloc->append_node(nd_dbl);
  }

  // elec_dist
  for(auto db_charge : db_charges) {
    std::string dbc_link;
    for(auto chg : db_charge)
      dbc_link.append(std::to_string(chg));
    char *dbc_char = doc.allocate_string(dbc_link.c_str());
    rapidxml::xml_node<>* nd_dbc = doc.allocate_node(rapidxml::node_element, "dist", dbc_char);
    nd_elec_dist->append_node(nd_dbc);
  }
  
    std::deque<std::vector<int>> db_charges;

  // write to file
  of << doc;
  of.close();
  doc.clear();

  std::cout << "Output written to: " << out_path << std::endl;
}
