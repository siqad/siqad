// @file:     simulator.cc
// @author:   Samuel
// @created:  2017.10.03
// @editted:  2017.10.03 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     simulator classes

#include "simulator.h"

namespace prim{

Simulator::Simulator(const QString &s_desc_path, QWidget *parent)
  : QObject(parent), sim_desc_path(s_desc_path)
{
}



// TODO might just put this in constructor
void Simulator::readSimInfo()
{}

// invoke the simulator binary
bool Simulator::invokeBinary(const QStringList &arguments)
{
  // TODO check whether bin_path is valid

  // TODO check that &arguments contains a valid path of the problem XML

  sim_process = new QProcess();
  sim_process->start(bin_path, arguments);
  
  // TODO check documentation for piping outputs

  // TODO connect signals for error and finish

  return true;
}



} // end of gui namespace
