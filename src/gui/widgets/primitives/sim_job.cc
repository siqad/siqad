// @file:     sim_job.cc
// @author:   Samuel
// @created:  2017.10.10
// @editted:  2017.10.10 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     SimJob classes


#include <QProcess>
#include <iostream>
#include "sim_job.h"

namespace prim{

SimJob::SimJob(SimEngine *eng, QWidget *parent)
  : QObject(parent), engine(eng)
{

}


// invoke the simulator binary
bool SimJob::invokeBinary()
{
  // NOTE might be helpful: https://stackoverflow.com/questions/14960472/running-c-binary-from-inside-qt-and-redirecting-the-output-of-the-binary-to-a

  // TODO emit signal to save problem file and wait for it to complete
  // problem_file_path = engine->generateJobDir() + "problem_desc.xml";

  qDebug() << tr("SimJob: prepare physeng binary execution"); // TODO put in other directories
  QFileInfo problem_file_info("./src/phys/problem_desc_datetime.xml"); // TODO don't hard code path
  arguments << problem_file_info.canonicalFilePath();
  arguments << problem_file_info.canonicalPath().append("/simanneal_output.xml"); // TODO put in other directories

  // TODO check whether bin_path is valid

  sim_process->setProgram(engine->getBinaryPath());
  sim_process->setArguments(arguments);
  sim_process->setProcessChannelMode(QProcess::MergedChannels);
  sim_process->start();

  // TODO check that &arguments contains a valid path of the problem XML

  // TODO check documentation for piping outputs

  // TODO connect signals for error and finish

  // temperary solution: just wait till completion
  while(!sim_process->waitForStarted());

  // dump output
  while(sim_process->waitForReadyRead())
    qDebug() << sim_process->readAll();

  qDebug() << tr("SimJob: binary has finished running.");

  return true;
}


bool SimJob::readResults(QString read_path)
{
  // TODO check path exists
  QFile result_file(read_path);
  
  if(!result_file.open(QFile::ReadOnly | QFile::Text)){
    qDebug() << tr("SimJob: Error when opening result file to read: %1").arg(result_file.errorString());
    return false;
  }

  QXmlStreamReader rs(&result_file);
  qDebug() << tr("Beginning load from %1").arg(result_file.fileName());

  // TODO flag that indicates what type of data these results contain, might be useful for sim_manager

  while(!rs.atEnd()){
    if(rs.isStartElement()){
      if(rs.name() == "eng_info"){
        // TODO
        rs.readNext();
      }
      else if(rs.name() == "sim_param"){
        // TODO
        rs.readNext();
      }
      else if(rs.name() == "physloc"){
        // TODO
        // make pair with dbdot x & y attributes and push_back to physlocs
        rs.readNext();
      }
      else if(rs.name() == "elec_dist"){
        // TODO
        // read dist, convert each character to bool (or raise error if not 0/1) and push_back the list to elec_dists
        rs.readNext();
      }
      // TODO make a QStringList of ignored stuff
      else{
        if(!ignored_xml_elements.contains(rs.name().toString()))
          qDebug() << tr("SimJob: invalid element encountered on line %1 - %2").arg(rs.lineNumber()).arg(rs.name().toString());
        rs.readNext();
      }
    }
    else
      rs.readNext();
  }

  if(rs.hasError()){
    qCritical() << tr("SimJob: Failed to read results, XML error - ") << rs.errorString().data();
    return false;
  }

  qDebug() << tr("Load complete");
  result_file.close();

  return true;
}

} // end of prim namespace
