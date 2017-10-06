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
PhysicsEngine::PhysicsEngine(const std::string &ifnm, const std::string &ofnm)
{
  problem.readProblem(ifnm);
  of_name = ofnm;
}

void PhysicsEngine::writeResultsXML()
{
  // NOTE in the future, there's probably a range of stuff that can be exported.
  // for now, only export charge config
  std::ofstream of(of_name);
  // TODO test that the output file path works

  xml_document<> doc;
  // https://stackoverflow.com/questions/25991961/how-to-fill-xml-document-using-rapidxml-c
}
