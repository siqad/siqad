// @file:     phys_engine.cc
// @author:   Samuel
// @created:  2017.08.23
// @editted:  2017.08.23 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Base class for physics engines

#include "phys_engine.h"

#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace phys;

// CONSTRUCTOR
PhysicsEngine::PhysicsEngine(const std::string &eng_nm, const std::string &i_path, const std::string &o_path, const std::string &script_path, const std::string &temp_path)
  : eng_name(eng_nm), script_path(script_path), in_path(i_path), out_path(o_path), temp_path(temp_path)
{
  problem.readProblem(i_path);
  readDBLocFromProblem();
}

void PhysicsEngine::readDBLocFromProblem()
{
  // grab all physical locations (in original distance unit)
  std::cout << "Grab all physical locations..." << std::endl;
  n_dbs = 0;
  for (auto db : problem) {
    if (db->elec != 1) {
      db_locs.push_back(std::make_pair(db->x, db->y));
      n_dbs++;
      std::cout << "DB loc: x=" << db_locs.back().first
          << ", y=" << db_locs.back().second << std::endl;
    } else {
      fixed_charges.push_back(std::make_tuple(db->x, db->y, db->elec));
    }
  }
  std::cout << "Free dbs, n_dbs=" << n_dbs << std::endl << std::endl;
}

void PhysicsEngine::writeResultsXml()
{
  std::cout << "Write results to XML..." << std::endl;
  // NOTE in the future, there's probably a range of stuff that can be exported.
  // for now, only export charge config

  // define major XML nodes
  bpt::ptree tree;
  bpt::ptree node_root;                             // <sim_out>
  bpt::ptree node_eng_info = xmlNodeEngInfo();      // <eng_info>
  bpt::ptree node_sim_params = xmlNodeSimParams();  // <sim_params>
  bpt::ptree node_physloc = xmlNodePhysloc();       // <physloc>
  bpt::ptree node_elec_dist = xmlNodeElecDist();    // <elec_dist>
  bpt::ptree node_line_scans = xmlNodeLineScans();  // <line_scans>

  // add nodes to appropriate parent
  node_root.add_child("eng_info", node_eng_info);
  node_root.add_child("sim_params", node_sim_params);
  node_root.add_child("physloc", node_physloc);
  node_root.add_child("elec_dist", node_elec_dist);
  node_root.add_child("line_scans", node_line_scans);
  tree.add_child("sim_out", node_root);

  // write to file
  bpt::write_xml(out_path, tree, std::locale(), bpt::xml_writer_make_settings<std::string>(' ',4));

  std::cout << "Write to XML complete." << std::endl;
}

bpt::ptree PhysicsEngine::xmlNodeEngInfo()
{
  bpt::ptree node_eng_info;
  node_eng_info.put("engine", eng_name.c_str());
  node_eng_info.put("version", "TBD"); // TODO real version
  return node_eng_info;
}

bpt::ptree PhysicsEngine::xmlNodeSimParams()
{
  bpt::ptree node_sim_params;
  //for (std::pair<std::string,std::string> param : problem.sim_params)
  //  node_sim_params.add_child(param.first, param.second);
  for (std::string param_key : problem.getParameterKeys())
    node_sim_params.put(param_key, problem.getParameter(param_key));
  return node_sim_params;
}

bpt::ptree PhysicsEngine::xmlNodePhysloc()
{
  bpt::ptree node_physloc;
  for (auto dbl : db_locs) {
    bpt::ptree node_dbdot;
    node_dbdot.put("<xmlattr>.x", float2StringPrecision(dbl.first, 2));
    node_dbdot.put("<xmlattr>.y", float2StringPrecision(dbl.second, 2));
    node_physloc.add_child("dbdot", node_dbdot);
  }
  return node_physloc;
}

bpt::ptree PhysicsEngine::xmlNodeElecDist()
{
  bpt::ptree node_elec_dist;
  for (auto db_charge : db_charges) {
    bpt::ptree node_dist;
    std::string dbc_link;
    for(auto chg : db_charge)
      dbc_link.append(std::to_string(chg));
    node_dist.put("", dbc_link);
    node_elec_dist.add_child("dist", node_dist);
  }
  return node_elec_dist;
}

bpt::ptree PhysicsEngine::xmlNodeLineScans()
{
  bpt::ptree node_line_scan_paths;
  for (LineScanPath line_scan_path : line_scan_paths) {
    bpt::ptree node_afm_path;
    bpt::ptree node_dbs_encountered;
    bpt::ptree node_scan_results;

    for (std::pair<float,float> db_loc : line_scan_path.db_locs_enc) {
      bpt::ptree node_db;
      node_db.put("<xmlattr>.x", float2StringPrecision(db_loc.first, 2));
      node_db.put("<xmlattr>.y", float2StringPrecision(db_loc.second, 2));
      node_dbs_encountered.add_child("db", node_db);
    }

    for (std::string scan_result : line_scan_path.results) {
      //bpt::ptree node_scan_result;
      //node_scan_result.put(scan_result);
      node_scan_results.add("line_scan", scan_result);
    }

    node_afm_path.add_child("dbs_encountered", node_dbs_encountered);
    node_afm_path.add_child("scan_results", node_scan_results);
    node_line_scan_paths.add_child("afm_path", node_afm_path);
  }
  return node_line_scan_paths;
}

std::string PhysicsEngine::formattedTime(const std::string &time_format) const
{
  const boost::posix_time::ptime curr_time = boost::posix_time::second_clock::local_time();
  boost::posix_time::time_facet* facet = new boost::posix_time::time_facet();
  facet->format(time_format.c_str());
  std::stringstream ss;
  ss.imbue(std::locale(std::locale::classic(), facet));
  ss << curr_time;
  return ss.str();
}


std::string PhysicsEngine::float2StringPrecision(float number, int decimal_places)
{
  std::stringstream ss;
  ss << std::fixed << std::setprecision(decimal_places) << number;
  return ss.str();
}
