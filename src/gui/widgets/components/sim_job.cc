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

JobStep::~JobStep()
{
  if (process != nullptr)
    delete process;
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

bool JobStep::readResults()
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
      job_results.insert(comp::JobResult::ElectronConfigsResult,
                         new comp::ElectronConfigSet(&rs));
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
      && job_results.keys().contains(comp::JobResult::ElectronConfigsResult)) {
    comp::ElectronConfigSet *ecs = static_cast<comp::ElectronConfigSet*>(job_results.value(comp::JobResult::ElectronConfigsResult));
    ecs->setDBPhysicalLocations(
        static_cast<comp::DBLocations*>(job_results.value(comp::JobResult::DBLocationsResult))->locations());
  }

  // TODO remove line scans support from SiQADConn

  if(rs.hasError()){
    qCritical() << tr("Failed to read results, XML error - ") << rs.errorString().data();
    return false;
  }

  qDebug() << tr("Successfully read job step result.");
  result_file.close();

  results_read = true;
  return true;
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
  replace_map["@PYTHON@"] = gui::python_path; // TODO needs further splitting for comma separated calls
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
  : QObject(parent), job_state(NotInvoked), job_name(nm)
{}

SimJob::~SimJob()
{
  for (JobStep *job_step : job_steps) {
    delete job_step;
  }
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
}

bool SimJob::beginJob()
{
  if (!placement_confirmed)
    prepareJob();

  qDebug() << "Beginning job step invocation.";
  job_state = Running;
  return job_steps.at(0)->invokeBinary();
}

void SimJob::continueJob(int prev_step_ind, bool prev_step_successful)
{
  if (!prev_step_successful) {
    qDebug() << tr("Last job step finished unsuccessfully, ceasing job.");
    job_state = FinishedWithError;
    emit sig_jobFinishState(this, job_state);
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
  } else {
    // wrap up job if no more steps
    job_state = FinishedNormally;
    emit sig_jobFinishState(this, job_state);
  }
}

void SimJob::terminateJob()
{

}

void SimJob::killJob()
{

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
  QString phys_tmp_rt_path = settings::AppSettings::instance()->getPath("phys/runtime_tmp_root_path");
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
