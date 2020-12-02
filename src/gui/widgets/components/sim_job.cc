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

using namespace comp;

// JobStep implementation
JobStep::JobStep(PluginEngine *t_engine, QStringList t_command_format,
                 gui::PropertyMap t_job_prop_map)
  : engine(t_engine), command_format(t_command_format)
{
  for (const QString &key : t_job_prop_map.keys()) {
    job_params.insert(key, t_job_prop_map.value(key).value.toString());
  }
}

JobStep::JobStep(QXmlStreamReader *rs, QDir job_root_dir)
{
  job_tmp_dir_path = job_root_dir.absolutePath();
  while (rs->readNextStartElement()) {
    if (rs->name() == "placement") {
      placement = rs->readElementText().toInt();
    } else if (rs->name() == "state") {
      auto&& meta_enum = QMetaEnum::fromType<JobStepState>();
      job_step_state = static_cast<JobStepState>(meta_enum.keyToValue(
            rs->readElementText().toLocal8Bit()));
    } else if (rs->name() == "command") {
      // TODO implement
      rs->skipCurrentElement();
    } else if (rs->name() == "step_dir") {
      js_tmp_dir_path = job_root_dir.absoluteFilePath(rs->readElementText());
    } else if (rs->name() == "problem_path") {
      problem_path = job_root_dir.absoluteFilePath(rs->readElementText());
    } else if (rs->name() == "result_path") {
      result_path = job_root_dir.absoluteFilePath(rs->readElementText());
    } else {
      qWarning() << tr("Unknown XML element encountered when importing JobStep:"
         " %1").arg(rs->name());
      rs->skipCurrentElement();
    }
  }
  qDebug() << tr("JobStep info: problem path %1, result_path %2").arg(problem_path).arg(result_path);
}

JobStep::~JobStep()
{
  if (process != nullptr)
    delete process;
}

void JobStep::writeManifest(QXmlStreamWriter *ws)
{
  ws->writeStartElement("job_step");
  ws->writeTextElement("placement", QString::number(placement));
  ws->writeTextElement("state", QVariant::fromValue(job_step_state).toString());

  ws->writeStartElement("command");
  for (QString line : command)
    ws->writeTextElement("line", line);
  ws->writeEndElement();

  QDir job_root_dir = QDir(job_tmp_dir_path);
  ws->writeComment("Paths below are relative to SimJob manifest");
  ws->writeTextElement("step_dir", job_root_dir.relativeFilePath(js_tmp_dir_path));
  ws->writeTextElement("problem_path", job_root_dir.relativeFilePath(problem_path));
  ws->writeTextElement("result_path", job_root_dir.relativeFilePath(result_path));
  ws->writeEndElement();
}

void JobStep::prepareJobStep(const int &t_placement, 
                             const QString &t_job_tmp_dir_path,
                             const QString &t_js_tmp_dir_path, 
                             const QString &t_problem_path, 
                             const QString &t_result_path)
{
  placement = t_placement;

  // set the job temp directory path, assumes that it has already been created
  job_tmp_dir_path = t_job_tmp_dir_path;

  // set and create the job step temp directory
  QDir job_tmp_dir(job_tmp_dir_path);
  js_tmp_dir_path = !t_js_tmp_dir_path.isEmpty() ? t_js_tmp_dir_path
    : job_tmp_dir.absoluteFilePath(tr("step_%1").arg(placement));
  QDir(js_tmp_dir_path).mkpath(".");

  // set the problem and result file paths
  QDir js_tmp_dir(js_tmp_dir_path);
  problem_path = !t_problem_path.isEmpty()  ? t_problem_path
    : js_tmp_dir.absoluteFilePath(tr("sim_problem_%1.xml").arg(placement));
  result_path = !t_result_path.isEmpty()    ? t_result_path
    : js_tmp_dir.absoluteFilePath(tr("sim_result_%1.xml").arg(placement));

  // other pre-invocation settings
  if (command_format.isEmpty()) {
    // default command format
    command_format = QStringList({"@BINPATH@", "@PROBLEMPATH@", "@RESULTPATH@"});
  }
  commandKeywordReplacement();
}

bool JobStep::invokeBinary()
{
  if (placement == -1) {
    qWarning() << "Job step execution order placement not initialized, stopping \
      invocation.";
    return false;
  }

  // check if problem file exists
  if (!QFileInfo(problem_path).exists()) {
    qDebug() << tr("SimJob: problem file '%1' doesn't exist.").arg(problem_path);
    return false;
  }

  // check if binary path of simulation engine exists
  if (!QFileInfo(engine->binaryPath()).exists()) {
    qDebug() << tr("SimJob: engine binary/script '%1' doesn't exist.").arg(engine->binaryPath());
    return false;
  }

  job_step_state = Running;

  qDebug() << tr("Job step %1 about to execute command: %2")
    .arg(placement).arg(command.join(" "));

  // set up process
  process = new QProcess();
  process->setProcessChannelMode(QProcess::MergedChannels); // TODO doesn't seem to be working now, check
  process->setProgram(command.takeFirst());
  process->setArguments(command);

  start_time = QDateTime::currentDateTime();

  qDebug() << tr("Starting step step process %1").arg(placement);
  process->start();

  // block SiQAD process until the plugin process has started
  qDebug() << tr("Waiting for process start success signal...");
  if (!process->waitForStarted()) {
    qCritical() << tr("Failed to start plugin process.");
    return false;
  } else {
    qDebug() << "Job step process started successfully.";
  }

  // connect signals for error and finish
  connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
          this, &JobStep::processJobStepCompletion);

  // read out standard output and error messages
  connect(process, &QProcess::readyReadStandardOutput,
          [this]()
          {
            std_out.append(QString::fromUtf8(process->readAllStandardOutput()));
          });
  connect(process, &QProcess::readyReadStandardError,
          [this]()
          {
            std_err.append(QString::fromUtf8(process->readAllStandardError()));
          });

  return true;
}

bool JobStep::readResults(bool attempt_import_logs)
{
  if (results_read) {
    qDebug() << "Results have already been read.";
    return true;
  }

  QFile result_file(result_path);

  if(!result_file.open(QFile::ReadOnly | QFile::Text)){
    qDebug() << tr("Error when opening job step result file to read: %1").arg(result_file.errorString());
    return false;
  }

  QXmlStreamReader rs(&result_file);
  qDebug() << tr("Reading simulation results from %1...").arg(result_file.fileName());

  // TODO store the following variables to the class itself
  QString engine_name = "";
  QString version = "";
  QMap<QString, QString> sim_params;

  // enter root XML node
  rs.readNextStartElement();

  auto unrecognizedXMLElement = [](QXmlStreamReader &rs) mutable
  {
    qWarning() << tr("Invalid element encountered on line %1 - %2")
      .arg(rs.lineNumber()).arg(rs.name().toString());
    rs.skipCurrentElement();
  };

  while (rs.readNextStartElement()) {
    if (rs.name() == "eng_info") {
      while (rs.readNextStartElement()) {
        if (rs.name() == "engine") {
          engine_name = rs.readElementText();
        } else if (rs.name() == "version") {
          version = rs.readElementText();
        } else if (rs.name() == "return_code") {
          // TODO remove this from SiQADConn
          rs.skipCurrentElement();
        } else if (rs.name() == "timestamp") {
          // TODO implement
          rs.skipCurrentElement();
        } else if (rs.name() == "time_elapsed_s") {
          // TODO implement
          rs.skipCurrentElement();
        } else {
          unrecognizedXMLElement(rs);
        }
      }
    } else if (rs.name() == "sim_params") {
      // params already stored in job_params, don't need to read again.
      rs.skipCurrentElement();
    } else if (rs.name() == "physloc") {
      job_results.insert(comp::JobResult::DBLocationsResult,
                         new comp::DBLocations(&rs));
    } else if (rs.name() == "elec_dist") {
      job_results.insert(comp::JobResult::ChargeConfigsResult,
                         new comp::ChargeConfigSet(&rs));
    } else if (rs.name() == "potential_map") {
      job_results.insert(comp::JobResult::PotentialLandscapeResult,
                         new comp::PotentialLandscape(&rs, QFileInfo(resultPath()).absolutePath()));
    } else if (rs.name() == "sqcommands") {
      job_results.insert(comp::JobResult::SQCommandsResult,
                        new comp::SQCommands(&rs));
    } else {
      unrecognizedXMLElement(rs);
    }
  }

  // TODO remove the following workaround after SiQADConn has been updated to
  // put DB physical locations inside electron config set
  if (job_results.keys().contains(comp::JobResult::DBLocationsResult)
      && job_results.keys().contains(comp::JobResult::ChargeConfigsResult)) {
    comp::ChargeConfigSet *ecs = static_cast<comp::ChargeConfigSet*>(job_results.value(comp::JobResult::ChargeConfigsResult));
    ecs->setDBPhysicalLocations(
        static_cast<comp::DBLocations*>(job_results.value(comp::JobResult::DBLocationsResult))->locations());
  }

  // TODO remove line scans support from SiQADConn

  if(rs.hasError()){
    qCritical() << tr("Failed to read results, XML error - ") << rs.errorString().data();
    return false;
  }

  // try to read std out and std error from log files if indicated (normally 
  // these are acquired from the QProcess, so only applicable when importing 
  // a job from manifest.)
  auto importLogFromFilePath = [](QString &s, const QString &fpath)
  {
    QFile file(fpath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
      qWarning() << tr("File cannot be opened for reading: %1").arg(fpath);
      return;
    }
    s = QString(file.readAll());
  };
  if (attempt_import_logs) {
    QDir js_tmp_dir(js_tmp_dir_path);
    importLogFromFilePath(std_out, js_tmp_dir.absoluteFilePath("runtime_stdout.log"));
    importLogFromFilePath(std_err, js_tmp_dir.absoluteFilePath("runtime_stderr.log"));
  }

  qDebug() << tr("Successfully read job step result.");
  result_file.close();

  results_read = true;
  return true;
}

void JobStep::exportTerminalOutputs(QString std_out_path, QString std_err_path)
{
  // store std out and std error into file log
  auto writeToFilePath = [](const QString &s, const QString &fpath)
  {
    QFile file(fpath);
    if (!file.open(QFile::WriteOnly)) {
      qWarning() << tr("Failed to open file to write: %1").arg(fpath);
      return;
    }
    file.write(s.toLocal8Bit());
    file.close();
  };
  writeToFilePath(std_out, std_out_path);
  writeToFilePath(std_err, std_err_path);
}

void JobStep::terminateJobStep()
{
#ifdef _WIN32
  process->kill();
#else
  process->terminate();
#endif
}

void JobStep::processJobStepCompletion(int t_exit_code, QProcess::ExitStatus t_exit_status)
{
  exit_code = t_exit_code, exit_status = t_exit_status;
  QString str_exit_status = (exit_status == QProcess::NormalExit) ? "Normal Exit" : "Crashed";
  qDebug() << tr("Job step %1 finished with exit code %2 and status %3.")
    .arg(placement).arg(exit_code).arg(str_exit_status);
  end_time = QDateTime::currentDateTime();

  bool successful = (exit_code == 0) && (exit_status == QProcess::NormalExit);
  if (successful)
    readResults();

  // inform the parent of the success state.
  emit sig_jobStepFinishState(placement, successful);
}

bool JobStep::commandKeywordReplacement()
{
  // keywords are not properly initialized if prepareJobStep hasn't been called
  if (placement == -1) {
    qWarning() << "Trying to perform command keyword replacement before job \
      step has been properly prepared.";
    return false;
  }

  QMap<QString, QString> replace_map;
  //replace_map["@PYTHON@"] = gui::python_path; // TODO needs further splitting for comma separated calls
  replace_map["@PYTHON@"] = engine->pythonBin();
  replace_map["@BINPATH@"] = engine->binaryPath();
  replace_map["@PHYSENGPATH@"] = QFileInfo(engine->descriptionFilePath()).absolutePath();
  replace_map["@PROBLEMPATH@"] = problem_path;
  replace_map["@RESULTPATH@"] = result_path;
  replace_map["@JOBTMP@"] = job_tmp_dir_path;
  replace_map["@STEPTMP@"] = js_tmp_dir_path;

  QRegExp regex("@(.*)?@");
  regex.setMinimal(true);

  command = command_format; // start from the command format
  for (int i=0; i<command.length(); i++) {
    while (command[i].indexOf(regex) != -1) {
      QString found_replace = regex.capturedTexts().first();
      if (!replace_map.contains(found_replace)) {
        qFatal(tr("Path replacement failed, key '%1' not found.")
            .arg(found_replace).toLatin1().constData(),0);
      }
      command[i].replace(command[i].indexOf(regex), found_replace.length(), replace_map[found_replace]);
    }
  }
  return true;
}


// SimJob implementation

SimJob::SimJob(const QString &nm, QWidget *parent)
  : QObject(parent), job_state(NotInvoked), job_name(nm), gui_ctrl_elems(this)
{}

SimJob::SimJob(const QString &fpath, bool dcmp, QString name_override, 
    QWidget *parent)
  : QObject(parent), job_state(FinishedNormally), gui_ctrl_elems(this),
    imported(true)
{
  QString manifest_path;
  gui_ctrl_elems.pb_terminate->setDisabled(true);

  auto find_manifest = [](QDir xdir)
  {
    QDirIterator it(xdir, QDirIterator::Subdirectories);
    while (it.hasNext()) {
      it.next();
      if (it.fileInfo().fileName() == "manifest.xml") {
        return it.fileInfo().absoluteFilePath();
      }
    }
    return QString();
  };

  // lambda function for importing job steps
  auto importJobSteps = [this](QXmlStreamReader &rs, QFile &file)
  {
    while (rs.readNextStartElement()) {
      if (rs.name() == "job_step") {
        job_steps.append(new JobStep(&rs, QFileInfo(file).dir()));
      } else {
        qWarning() << tr("Unknown XML tag encountered when importing job steps:"
           " %1").arg(rs.name());
        rs.skipCurrentElement();
      }
    }
  };

  // decompress the archive if dcmp flag is true
  if (dcmp) {
    qDebug() << "Decompressing SimJob archive...";
    QString tmpd = settings::AppSettings::instance()->getPath("plugs/runtime_tmp_root_path");
    QDir xdir(QDir(tmpd).absoluteFilePath("IM_" + QDateTime::currentDateTime().toString("yyMMdd_HHmmss")));
    zipper::Unzipper unzipper(fpath.toStdString());
    unzipper.extract(xdir.absolutePath().toStdString());
    qDebug() << "Searching for SimJob manifest...";
    manifest_path = find_manifest(xdir);
    if (manifest_path.isEmpty()) {
      QMessageBox msg;
      msg.setText("manifest.xml not found in the provided archive. Import halted.");
      msg.exec();
      job_state = FinishedWithError;
      gui_ctrl_elems.pb_terminate->setText("Import Error");
      return;
    }
  } else {
    manifest_path = fpath;
  }

  // get file and XML stream
  QFile file(manifest_path);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    qWarning() << tr("Error when opening file to read: %1").arg(file.errorString());
    return;
  }
  QXmlStreamReader rs(&file);

  // read manifest from stream
  qDebug() << "Reading SimJob manifest";
  QString name_read = "";
  job_name = "IMP_UNTITLED";
  rs.readNextStartElement();  // enter root element
  if (rs.name() != "simjob") {
    QString msgstr = tr("Manifest file at %1 does not have the appropriate "
        "root node. Import halted.").arg(manifest_path);
    qWarning() << msgstr;
    QMessageBox msg;
    msg.setText(msgstr);
    msg.show();
    return;
  }
  while (rs.readNextStartElement()) {
    if (rs.name() == "name") {
      name_read = rs.readElementText();
    } else if (rs.name() == "state") {
      auto&& meta_enum = QMetaEnum::fromType<JobState>();
      job_state = static_cast<JobState>(meta_enum.keyToValue(rs.readElementText().toLocal8Bit()));
    } else if (rs.name() == "time_start") {
      // TODO implement
      rs.skipCurrentElement();
    } else if (rs.name() == "time_end") {
      // TODO implement
      rs.skipCurrentElement();
    } else if (rs.name() == "job_steps") {
      importJobSteps(rs, file);
    } else {
      qWarning() << tr("Unknown XML tag encountered when importing SimJob: %1")
        .arg(rs.name());
      rs.skipCurrentElement();
    }
  }
  if (!name_override.isEmpty()) {
    if (!name_read.isEmpty()) {
      QRegExp regex("@IMPORTED_NAME@");
      name_override.replace(QRegExp("@IMPORTED_NAME@"), name_read);
    }
    job_name = name_override;
  }

  // import results
  qDebug() << "Reading JobStep results";
  for (JobStep *js : job_steps) {
    js->readResults(true);
    for (comp::JobResult::ResultType type : js->jobResults().keys()) {
      result_type_step_map.insert(type, js);
    }
  }
  gui_ctrl_elems.pb_terminate->setText("Imported");

  // clean up
  file.close();
}

SimJob::~SimJob()
{
  for (JobStep *job_step : job_steps) {
    delete job_step;
  }
}

void SimJob::writeManifest(QString fpath)
{
  qDebug() << "Writing/updating job manifest...";
  if (fpath.isEmpty()) {
    fpath = job_tmp_dir_path+"/manifest.xml";
  }
  QFile file(fpath);
  if(!file.open(QIODevice::WriteOnly)){
    qDebug() << tr("Save: Error when opening manifest file to export: %1").arg(file.errorString());
  }
  QXmlStreamWriter ws(&file);
  ws.setAutoFormatting(true);
  ws.writeStartDocument();
  writeManifest(&ws);
  file.close();
  qDebug() << tr("Manifest written to %1").arg(job_tmp_dir_path+"/manifest.xml");
}

void SimJob::writeManifest(QXmlStreamWriter *ws)
{
  // root element
  ws->writeStartElement("simjob");
  ws->writeTextElement("name", job_name);
  ws->writeTextElement("state", QVariant::fromValue(job_state).toString());
  if (start_time.isValid()) {
    ws->writeTextElement("time_start", QVariant::fromValue(start_time).toString());
  }
  if (end_time.isValid()) {
    ws->writeTextElement("time_end", QVariant::fromValue(end_time).toString());
  }

  // all job steps
  ws->writeStartElement("job_steps");
  for (JobStep *js : job_steps) {
    js->writeManifest(ws);
  }
  ws->writeEndElement();

  // close root elem
  ws->writeEndElement();
}

void SimJob::confirmJobStepsPlacement()
{
  if (placement_confirmed)
    return;

  for (int i=0; i<job_steps.length(); i++) {
    job_steps.at(i)->prepareJobStep(i, runtimeTempPath());
  }
}

void SimJob::prepareJob()
{
  if (!placement_confirmed) {
    qDebug() << "Confirming job steps placement...";
    confirmJobStepsPlacement();
  }

  // export problem files for all job steps
  qDebug() << "Exporting job step problem files...";
  for (JobStep *job_step : job_steps) {
    emit sig_exportJobStepProblem(job_step, inclusion_area);
  }

  // connect necessary signals
  for (JobStep *job_step : job_steps) {
    connect(job_step, &comp::JobStep::sig_jobStepFinishState,
            this, &SimJob::continueJob);
  }

  // write job manifest
  writeManifest();
}

bool SimJob::beginJob()
{
  if (!placement_confirmed)
    prepareJob();

  qDebug() << "Beginning job step invocation.";
  job_state = Running;
  curr_step = job_steps.at(0);
  return job_steps.at(0)->invokeBinary();
}

void SimJob::continueJob(int prev_step_ind, bool prev_step_successful)
{
  if (!prev_step_successful) {
    qDebug() << tr("Last job step finished unsuccessfully, ceasing job.");
    jobFinishActions(FinishedWithError);
    return;
  }

  qDebug() << tr("Received step completion notice from job step %1.").arg(prev_step_ind);

  qDebug() << tr("Prev step engine name %1").arg(job_steps[0]->pluginEngine()->name());
  qDebug() << tr("job_steps.length=%1").arg(job_steps.length());

  for (comp::JobResult::ResultType type : job_steps.at(prev_step_ind)->jobResults().keys())
    result_type_step_map.insert(type, job_steps.at(prev_step_ind));

  int i = prev_step_ind + 1;
  if (i < job_steps.length()) {
    // invoke next step if any
    job_steps.at(i)->invokeBinary();
    curr_step = job_steps.at(i);
  } else {
    // wrap up job if no more steps
    curr_step = nullptr;
    jobFinishActions(FinishedNormally);
  }
  writeManifest();
}

void SimJob::terminateJob()
{
  if (curr_step != nullptr)
    curr_step->terminateJobStep();
}

void SimJob::jobFinishActions(JobState t_job_state)
{
  job_state = t_job_state;
  switch(job_state)
  {
    case FinishedWithError:
      gui_ctrl_elems.pb_terminate->setText("Error");
      break;
    case FinishedNormally:
    {
      gui_ctrl_elems.pb_terminate->setText("Finished");
      gui_ctrl_elems.pb_export_results->setEnabled(true);
      break;
    }
    default:
      break;
  }
  gui_ctrl_elems.pb_terminate->setDisabled(true);
  emit sig_jobFinishState(this, job_state);
}

QList<QStandardItem*> SimJob::jobInfoStandardItemRow(QList<JobInfoStandardItemField> fields)
{
  QList<QStandardItem*> info_si_row;
  QMetaEnum info_si_enum = QMetaEnum::fromType<JobInfoStandardItemField>();
  if (fields.isEmpty()) {
    // if the input list is empty, assume that everything is requested.
    for (int i=0; i<info_si_enum.keyCount(); i++) {
      fields.append(static_cast<JobInfoStandardItemField>(info_si_enum.value(i)));
    }
  }
  for (JobInfoStandardItemField field : fields) {
    switch (field) {
      case JobNameField:
        info_si_row.append(new QStandardItem(name()));
        break;
      case JobStartTimeField:
        info_si_row.append(new QStandardItem(startTime().toString("yyyy-MM-dd HH:mm:ss")));
        break;
      case JobEndTimeField:
        info_si_row.append(new QStandardItem(endTime().toString("yyyy-MM-dd HH:mm:ss")));
        break;
      case JobStepCountField:
        info_si_row.append(new QStandardItem(QString::number(job_steps.length())));
        break;
      case JobFinishStateField:
        info_si_row.append(new QStandardItem(QMetaEnum::fromType<JobState>().valueToKey(job_state)));
        break;
      case JobTempPathField:
        info_si_row.append(new QStandardItem(runtimeTempPath()));
        break;
      default:
        break;
    }
  }

  return info_si_row;
}

QString SimJob::runtimeTempPath()
{
  QString phys_tmp_rt_path = settings::AppSettings::instance()->getPath("plugs/runtime_tmp_root_path");
  if(job_tmp_dir_path.isEmpty()){
    QString sub_dir = name().isEmpty() ? QDateTime::currentDateTime().toString("MM-dd_HHmm") : name();
    job_tmp_dir_path = QDir(phys_tmp_rt_path).filePath(sub_dir);
  }
  QDir job_tmp_dir(job_tmp_dir_path);
  job_tmp_dir.mkpath(".");
  return job_tmp_dir_path;
}

QWidget *SimJob::terminalOutputDialog(QWidget *parent, Qt::WindowFlags w_flags)
{
  // main terminal output widget
  QWidget *w_job_term_out = new QWidget(parent, w_flags);

  QListWidget *lw_job_steps = new QListWidget();
  QStackedWidget *sw_job_term_outs = new QStackedWidget();

  lw_job_steps->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
  sw_job_term_outs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  for (JobStep *js : job_steps) {
    lw_job_steps->addItem(tr("Step %1").arg(js->jobStepPlacement()));

    QComboBox *cb_channel = new QComboBox();
    QPlainTextEdit *te_js_term_out = new QPlainTextEdit;

    QString str_stdout = "Standard Output";
    QString str_stderr = "Standard Error";
    cb_channel->addItem(str_stdout);
    cb_channel->addItem(str_stderr);

    auto showChannelText = [js, te_js_term_out, str_stdout, str_stderr](const QString &str_channel)
    {
      te_js_term_out->clear();
      if (str_channel == str_stdout) {
        te_js_term_out->setPlainText(js->terminalOutput(QProcess::StandardOutput));
      } else if (str_channel == str_stderr) {
        te_js_term_out->setPlainText(js->terminalOutput(QProcess::StandardError));
      } else {
        qWarning() << tr("Channel %1 not recognized").arg(str_channel);
      }
    };

    connect(cb_channel, &QComboBox::currentTextChanged, showChannelText);
    showChannelText(cb_channel->currentText());

    QVBoxLayout *vl_js_term_out = new QVBoxLayout();
    vl_js_term_out->addWidget(cb_channel);
    vl_js_term_out->addWidget(te_js_term_out);

    QWidget *w_js_term_out = new QWidget();
    w_js_term_out->setLayout(vl_js_term_out);
    sw_job_term_outs->addWidget(w_js_term_out);
  }

  connect(lw_job_steps, &QListWidget::currentRowChanged,
          sw_job_term_outs, &QStackedWidget::setCurrentIndex);

  // pop-up widget
  QHBoxLayout *hl_job_term_out = new QHBoxLayout();
  hl_job_term_out->addWidget(lw_job_steps);
  hl_job_term_out->addWidget(sw_job_term_outs);

  w_job_term_out->setWindowTitle(tr("%1 Terminal Output").arg(name()));
  w_job_term_out->setLayout(hl_job_term_out);
  return w_job_term_out;
}

bool SimJob::exportJob(QString out_path)
{
  if (imported) {
    QMessageBox msg;
    msg.setText("The job you are attempting to export was an imported job. "
        "Imported jobs cannot be exported again.");
    msg.exec();
    return false;
  }
  if (job_state != FinishedNormally) {
    QMessageBox msg;
    msg.setText("The SimJob that you are attempting to export is not one that "
        "completed successfully. Export halted.");
    msg.exec();
    return false;
  }
  if (out_path.isNull()) {
    out_path = QFileDialog::getSaveFileName(nullptr, 
        tr("Export SimJob"), name() + ".sqjx.zip");
    if (out_path.isEmpty()) {
      qWarning() << "No output path was chosen. Halting export.";
      return false;
    }
  }

  // tell job steps to write their terminal outputs to file
  for (JobStep *js : job_steps) {
    QDir js_tmp_dir(js->jobStepTempDirPath());
    js->exportTerminalOutputs(js_tmp_dir.absoluteFilePath("runtime_stdout.log"),
        js_tmp_dir.absoluteFilePath("runtime_stderr.log"));
  }

  // throw everything to archive
  zipper::Zipper zipper(out_path.toStdString());
  zipper.add(job_tmp_dir_path.toStdString());
  zipper.close();

  qDebug() << tr("SimJob exported successfully to %1").arg(out_path);

  return true;
}
