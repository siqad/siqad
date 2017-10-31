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

SimJob::SimJob(const QString &nm, SimEngine *eng, QWidget *parent)
  : QObject(parent), job_name(nm), engine(eng)
{
  completed = false;
}


// invoke the simulator binary
bool SimJob::invokeBinary()
{
  // NOTE might be helpful: https://stackoverflow.com/questions/14960472/running-c-binary-from-inside-qt-and-redirecting-the-output-of-the-binary-to-a

  // TODO emit signal to save problem file and wait for it to complete
  // problem_file_path = engine->generateJobDir() + "problem_desc.xml";

  qDebug() << tr("SimJob: prepare physeng binary execution"); // TODO put in other directories
  QFileInfo problem_file_info(problemFile()); // TODO don't hard code path

  // check if file exists
  qDebug() << tr("Check if file exists...");
  if(!(problem_file_info.exists() && problem_file_info.isFile())){
    qDebug() << tr("SimJob: problem file '%1' doesn't exist.").arg(problem_file_info.filePath());
    return false;
  }
  qDebug() << tr("SimJob: File does exist");

  // check if binary path exists
  qDebug() << tr("Check if binary path exists...");
  QFileInfo bin_path_info(engine->binaryPath());
  if(!(bin_path_info.exists() && bin_path_info.isFile())){
    qDebug() << tr("SimJob: engine binary '%1' doesn't exist.").arg(bin_path_info.filePath());
    return false;
  }

  arguments << problem_file_info.canonicalFilePath();
  arguments << resultFile();
  //arguments << problem_file_info.canonicalPath().append("/simanneal_output.xml"); // TODO put in other directories

  start_time = QDateTime::currentDateTime();

  qDebug() << tr("SimJob: Setting up simulation process...");
  sim_process = new QProcess();
  sim_process->setProgram(engine->binaryPath());
  //sim_process->setProgram("src/phys/physeng");
  sim_process->setArguments(arguments);
  sim_process->setProcessChannelMode(QProcess::MergedChannels);
  qDebug() << tr("SimJob: Starting process");
  sim_process->start();

  // TODO check that &arguments contains a valid path of the problem XML

  // TODO check documentation for piping outputs

  // TODO connect signals for error and finish

  // temperary solution: just wait till completion
  qDebug() << tr("SimJob: wait for completion...");
  while(!sim_process->waitForStarted());

  // dump output
  while(sim_process->waitForReadyRead())
    terminal_output.append(QString::fromStdString(sim_process->readAll().toStdString()));
    //qDebug() << sim_process->readAll();

  qDebug() << tr("SimJob: simulation complete. You may check the job's terminal output on the simulation visualization panel.");
  completed = true;

  end_time = QDateTime::currentDateTime(); // TODO instead of determining end time here, should read end time from XML for future

  return true;
}


bool SimJob::readResults()
{
  // TODO check path exists
  QFile result_file(resultFile());
  
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
        while(!(rs.isEndElement() && rs.name() == "physloc")){
          if(!rs.readNextStartElement())
            continue; // skip until a start element is encountered
          if(rs.name() == "dbdot"){
            float x=0,y=0;
            for(QXmlStreamAttribute &attr : rs.attributes()){
              if(attr.name().toString() == QLatin1String("x"))
                x = attr.value().toFloat();
              else if(attr.name().toString() == QLatin1String("y"))
                y = attr.value().toFloat();
            }
            physlocs.append(qMakePair(x,y));
            //qDebug() << tr("SimJob: Physloc identified at x=%1, y=%2").arg(x).arg(y);
          }
        }
        rs.readNext();
      }
      else if(rs.name() == "elec_dist"){
        while(!(rs.isEndElement() && rs.name() == "elec_dist")){
          if(!rs.readNextStartElement())
            continue; // skip until a start element is encountered
          if(rs.name() == "dist"){
            // read dist, convert each character to bool (or raise error if not 0/1) and push_back the list to elec_dists
            elec_dists.append(QList<int>());
            QString dist = rs.readElementText();
            for(QString chg : dist)
              elec_dists.last().append(chg.toInt());
            // print dist for verification TODO remove later
            QString this_dist;
            for(int this_chg : elec_dists.last())
              this_dist.append(QString::number(this_chg));
            //qDebug() << tr("This distribution: %1").arg(this_dist);
          }
        }
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

bool SimJob::processResults()
{
  // TODO check that results have already been read
  // TODO sort results?
  // TODO deduplicate results, keep count of how many times it was duplicated
  return true;
}


QString SimJob::runtimeTempDir()
{
  if(run_job_dir.isEmpty()){
    run_job_dir = QDir(engine->runtimeTempDir()).filePath(name().isEmpty() ? QDateTime::currentDateTime().toString("MM-dd_HHmm") : name());
  }
  QDir job_qdir(run_job_dir);
  if(!job_qdir.exists())
    job_qdir.mkpath(".");
  return run_job_dir;
}


QString SimJob::problemFile()
{
  if(problem_path.isEmpty())
    problem_path = QDir(runtimeTempDir()).filePath("sim_problem.xml");
  return problem_path;
}


QString SimJob::resultFile()
{
  if(result_path.isEmpty())
    result_path = QDir(runtimeTempDir()).filePath("sim_result.xml");
  return result_path;
}


void SimJob::saveTerminalOutput()
{
  // TODO implement
  QString save_term_path = QFileDialog::getSaveFileName(0, tr("Save Terminal Output"),
                            QDir::home().filePath(tr("%1_term_out.txt").arg(name())), tr("TXT files (*.txt)"));
  if(save_term_path.isEmpty())
    return;

  QFile save_f(save_term_path);

  if(!save_f.open(QIODevice::WriteOnly)){
    qWarning() << tr("Error opening file '%1' when saving terminal output").arg(save_f.errorString());
    return;
  }

  QTextStream save_s(&save_f);
  save_s << terminalOutput();

  save_f.close();
}


// PRIVATE

void SimJob::deduplicateDist()
{
  // TODO update dist count after deduplication
}

} // end of prim namespace
