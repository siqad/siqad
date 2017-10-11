// @file:     sim_job.cc
// @author:   Samuel
// @created:  2017.10.10
// @editted:  2017.10.10 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     SimJob classes

#include "sim_job.h"

namespace prim{

SimJob::SimJob(SimEngine *eng, QWidget *parent)
  : QObject(parent), engine(eng)
{

}


// invoke the simulator binary
bool SimJob::invokeBinary(const QStringList &arguments)
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
  while(!sim_process->waitForStart());

  // dump output
  while(sim_process->waitForReadyRead())
    qDebug() << sim_process->readAll();

  qDebug() << tr("SimJob: binary has finished running.");

  return true;
}


void SimJob::readResults(QString read_path)
{
  // TODO check path exists
}

} // end of prim namespace
