// @file:     sim_job.cc
// @author:   Samuel
// @created:  2017.10.10
// @editted:  2017.10.10 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     SimJob classes


#include <QProcess>
#include <iostream>
#include <algorithm>
#include "sim_job.h"
#include "../../../global.h"

extern QString gui::python_path;

namespace prim{

SimJob::SimJob(const QString &nm, SimEngine *eng, QWidget *parent)
  : QObject(parent), job_name(nm), engine(eng)
{
  completed = false;
}


// append all entries in the provided PropertyMap to the list of simulation parameters
void SimJob::addSimParams(const gui::PropertyMap &sim_params_map)
{
  for (const QString &key : sim_params_map.keys())
    addSimParam(key, sim_params_map[key].value.toString());
}


// invoke the simulator binary. Assumes that the problem file has already been written to
// the path specified by problemFile()
bool SimJob::invokeBinary()
{
  QFileInfo problem_file_info(problemFile());

  // check if problem file exists
  if(!(problem_file_info.exists() && problem_file_info.isFile())){
    qDebug() << tr("SimJob: problem file '%1' doesn't exist.").arg(problem_file_info.filePath());
    return false;
  }

  // check if binary path of simulation engine exists
  QFileInfo bin_path_info(engine->binaryPath());
  if(!(bin_path_info.exists() && bin_path_info.isFile())){
    qDebug() << tr("SimJob: engine binary/script '%1' doesn't exist.").arg(bin_path_info.filePath());
    return false;
  }

  // setup simulation process

  sim_process = new QProcess();
  if (!engine->runtimeInterpreter().isEmpty()) {
    if (engine->runtimeInterpreter() == "python" && !gui::python_path.isEmpty()) {
      // using an interpreter, e.g. Python
      // template: `python /path/to/script.py /path/to/problem/file /path/to/result/file`
      sim_process->setProgram(gui::python_path);
      cml_arguments << engine->binaryPath();
    } else {
      qCritical() << tr("Runtime interpreter %1 not recognized, ceasing binary invocation").arg(engine->runtimeInterpreter());
      return false;
    }
  } else {
    // calling a binary
    // template: `/path/to/binary /path/to/problem/file /path/to/result/file`
    sim_process->setProgram(engine->binaryPath());
  }
  
  cml_arguments << problem_file_info.canonicalFilePath(); // problem file
  cml_arguments << resultFile();                          // result file

  if (!engine->linkedScriptPath().isEmpty()) {
    // script called by binary, if applicable
    cml_arguments << engine->linkedScriptPath();
  }

  start_time = QDateTime::currentDateTime();

  sim_process->setArguments(cml_arguments);
  sim_process->setProcessChannelMode(QProcess::MergedChannels);
  qDebug() << tr("SimJob: Starting process");
  sim_process->start();

  // TODO connect signals for error and finish

  // temperary solution: just wait till completion
  qDebug() << tr("SimJob: Process started, waiting for completion...");
  while(!sim_process->waitForStarted());

  while(sim_process->waitForReadyRead(-1))
    terminal_output.append(QString::fromStdString(sim_process->readAll().toStdString())); // dump output TODO might not have to do it in while

  // post-simulation flag setting
  end_time = QDateTime::currentDateTime();
  completed = true;

  // clean up sim_process pointer
  delete sim_process;
  sim_process = NULL;

  qDebug() << tr("SimJob: simulation complete. You may check the job's terminal output on the simulation visualization panel.");
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
  qDebug() << tr("SimJob: Reading simulation results from %1...").arg(result_file.fileName());

  // TODO flag that indicates what type of data these results contain, might be useful for sim_manager

  // store electron distributions to a map first for deduplication
  QMap<QString, elecDist> elec_dists_map;
  QString engine_name = "";
  while(!rs.atEnd()){
    if(rs.isStartElement()){
      if(rs.name() == "sim_out"){
        rs.readNext();
      }
      else if(rs.name() == "eng_info"){
        while(!(rs.isEndElement() && rs.name() == "eng_info")){
          if(!rs.readNextStartElement())
            continue; // skip until a start element is encountered
          if(rs.name() == "engine"){
            qDebug() << tr("Engine name: %1").arg(rs.readElementText());
            engine_name = rs.readElementText();
          }
          else if(rs.name() == "version"){
            // TODO
          }
        }
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
            // read dist, convert each character to bool and store to a temp map
            float energy = -1;
            int config_count = 1;
            for (QXmlStreamAttribute &attr : rs.attributes()) {
              if (attr.name().toString() == QLatin1String("energy")) {
                energy = attr.value().toFloat();
              } else if (attr.name().toString() == QLatin1String("count")) {
                config_count = attr.value().toInt();
              }
            }

            // convert distribution to array of int
            QString dist = rs.readElementText();

            elecDist read_dist;
            read_dist.energy = energy;
            read_dist.config_count = config_count;

            for (QString charge_str : dist) {
              read_dist.dist.append(charge_str.toInt());
              read_dist.elec_count += read_dist.dist.back();
            }

            elec_dists_map[dist] = read_dist;
            //qDebug() << tr("Distribution: %1, Energy: %2, Count: %3").arg(dist).arg(read_dist.energy).arg(config_count);
          }
        }
        rs.readNext();
      }
      else if (rs.name() == "line_scans") {
        while (!(rs.isEndElement() && rs.name() == "line_scans")) {
          if (!rs.readNextStartElement())
            continue; // skip until a start element is encountered
          if (rs.name() == "afm_path") {
            // TODO add an AFM path entry to the job
            line_scan_paths.append(LineScanPath());
            LineScanPath &curr_path = line_scan_paths.back();
            while (!(rs.isEndElement() && rs.name() == "afm_path")) {
              if (!rs.readNextStartElement())
                continue;
              if (rs.name() == "db") {
                float x=0,y=0;
                // dbs encountered by this path
                for (QXmlStreamAttribute &attr : rs.attributes()) {
                  if (attr.name().toString() == QLatin1String("x"))
                    x = attr.value().toFloat();
                  else if (attr.name().toString() == QLatin1String("y"))
                    y = attr.value().toFloat();
                }
                // TODO add these dbs to the AFM path struct thing
                //line_scan_paths.last().afm_nodes.append(qMakePair(x,y));
                curr_path.afm_nodes.append(qMakePair(x,y));
              } else if (rs.name() == "line_scan") {
                // individual line scan results
                curr_path.results.append(rs.readElementText());
              }
            }
          }
        }
        rs.readNext();
      }
      else if(rs.name() == "potential_map"){
        QVector<float> potentials_vec;
        while(!(rs.isEndElement() && rs.name() == "potential_map")){
          if(!rs.readNextStartElement())
            continue; // skip until a start element is encountered
          if(rs.name() == "potential_val"){
            float x=0,y=0,pot_val;
            for(QXmlStreamAttribute &attr : rs.attributes()){
              if(attr.name().toString() == QLatin1String("x"))
                x = attr.value().toFloat();
              else if(attr.name().toString() == QLatin1String("y"))
                y = attr.value().toFloat();
              else if(attr.name().toString() == QLatin1String("val"))
                pot_val = attr.value().toFloat();
            }
            //build the vector to insert into the QList
            potentials_vec.clear();
            potentials_vec.append(x);
            potentials_vec.append(y);
            potentials_vec.append(pot_val);
            potentials.append(potentials_vec);
            //qDebug() << tr("SimJob: Physloc identified at x=%1, y=%2").arg(x).arg(y);
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

  qDebug() << tr("SimJob: Successfully read simulation result.");
  result_file.close();

  // sort and store the deduplicated electron distributions into the class
  if (elec_dists_map.size() > 0)
    processElecDists(elec_dists_map);

  return true;
}


void SimJob::processElecDists(QMap<QString, elecDist> elec_dists_map)
{
  // sort
  // TODO it is unecessary to start with a QMap for elec_dists now, streamline
  // the code.
  elec_dists.append(elec_dists_map.values());
  std::sort(elec_dists.begin(), elec_dists.end());

  // find average
  int result_count = elec_dists.size();
  int db_count = elec_dists[0].dist.size();
  int dist_count=0;
  for (int db_ind=0; db_ind<db_count; db_ind++) {
    elec_dists_avg.push_back(0);
    for (int result_ind=0; result_ind<result_count; result_ind++) {
      elec_dists_avg[db_ind] += elec_dists[result_ind].dist[db_ind] * 
                                elec_dists[result_ind].config_count;
      if (db_ind == 0) dist_count += elec_dists[result_ind].config_count;
    }
    elec_dists_avg[db_ind] /= dist_count;
  }

  // save available electron counts
  QList<int> elec_counts_occurances;  // accumulation of #config for each #elec_count
  for (elecDist dist : elec_dists) {
    int ind = elec_counts.indexOf(dist.elec_count);
    if (ind >= 0) {
      elec_counts_occurances[ind] += dist.config_count;
    } else {
      elec_counts.append(dist.elec_count);
      elec_counts_occurances.append(dist.config_count);
    }
  }

  // save the electron count with the most number of occurances, the default
  // dist selection will be the lowest energy state with this number of electrons
  preferred_elec_count = elec_counts.at(std::max_element(elec_counts_occurances.begin(), 
        elec_counts_occurances.end()) - elec_counts_occurances.begin());
  std::sort(elec_counts.begin(), elec_counts.end());

  // find the default index of electron distribution to show
  for (int i=0; i<elec_dists.size(); i++) {
    if (elec_dists[i].elec_count == preferred_elec_count) {
      default_elec_dist_ind = i;
      break;
    }
  }

  // disable any filters by default
  applyElecDistsFilter(-1);
}


void SimJob::applyElecDistsFilter(int elec_count)
{
  if (elec_count == -1) {
    elec_dists_filtered = elec_dists;
    return;
  }

  elec_dists_filtered.clear();
  for (elecDist dist : elec_dists)
    if (dist.elec_count == elec_count)
      elec_dists_filtered.append(dist);
}


float SimJob::elecDistAvgDegenOfDB(int dist_ind, int db_ind)
{
  float target_energy = elec_dists_filtered[dist_ind].energy;
  int degen_count = 0;
  float degen_db_accum = 0;
  for (elecDist dist : elec_dists_filtered) {
    if (dist.energy < target_energy)
      continue;
    else if (dist.energy > target_energy)
      break;

    degen_db_accum += dist.dist[db_ind];
    degen_count++;
  }
  return degen_db_accum / degen_count;
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
