/** @file:     sim_job.h
 *  @author:   Samuel
 *  @created:  2017.10.10
 *  @editted:  2017.10.10 - Samuel
 *  @license:  GNU LGPL v3
 *
 *  @desc:     SimJob object that describes a simulation job and stores the results from that job
 */

#ifndef _COMP_SIM_JOB_H_
#define _COMP_SIM_JOB_H_

#include <QtWidgets>
#include <QtCore>
#include "plugin_engine.h"
#include "job_results/job_result_types.h"
#include "settings/settings.h" // TODO probably need this later
#include <tuple> //std::tuple for 3+ article data structure, std::get for accessing the tuples
#include <QDir>
#include <libs/zipper/zipper/unzipper.h>
#include <libs/zipper/zipper/zipper.h>

namespace comp{

  class SimJob;

  //! A single job step in a job.
  class JobStep : public QObject
  {
    Q_OBJECT

  public:

    enum JobStepState{NotInvoked, Running, FinishedWithError, FinishedNormally};
    Q_ENUM(JobStepState);

    //! Constructor.
    JobStep(PluginEngine *t_engine, QStringList t_command_format, 
            gui::PropertyMap t_job_prop_map);
    
    //! XML import constructor.
    JobStep(QXmlStreamReader *rs, QDir job_root_dir);

    //! Destructor.
    ~JobStep();

    //! Write the manifest of this job step to the provided XML Stream.
    void writeManifest(QXmlStreamWriter *ws);

    //! Prepare job step by setting the step placement within the job and 
    //! various paths.
    //! For the optional paths, if no input is given they would be automatically
    //! determined using t_placement and t_job_tmp_dir_path.
    void prepareJobStep(const int &t_placement,
                        const QString &t_job_tmp_dir_path,
                        const QString &t_js_tmp_dir_path=QString(),
                        const QString &t_problem_path=QString(),
                        const QString &t_result_path=QString());

    //! Invoke the job step binary and return whether the process set-up 
    //! procedure was successful. Cannot be invoked if confirmJobStepsPlacement()
    //! has not been called or was unsuccessful in the parent sim job.
    //! Returns whether the binary has been invoked successfully.
    bool invokeBinary();

    //! Process the job finish signal.
    void processJobStepCompletion(int t_exit_code, QProcess::ExitStatus t_exit_status);

    //! Read job step results.
    bool readResults(bool attempt_import_logs=false);

    //! Write the terminal outputs to files.
    void exportTerminalOutputs(QString std_out_path, QString std_err_path);

    //! Kill job step
    void terminateJobStep();

    // ACCESSORS

    //! Return the placement.
    int jobStepPlacement() {return placement;}

    //! Return the engine pointer.
    PluginEngine *pluginEngine() {return engine;}

    //! Return the simulation parameters.
    QMap<QString, QString> jobParameters() {return job_params;}

    //! Return the problem file path.
    QString problemPath() {return problem_path;}

    //! Return the result file path.
    QString resultPath() {return result_path;}

    //! Return the start time.
    QDateTime startTime() {return start_time;}

    //! Return the end time.
    QDateTime endTime() {return end_time;}

    //! Return the terminal output from the specified channel.
    QString terminalOutput(QProcess::ProcessChannel channel)
    {
      switch (channel) {
        case QProcess::StandardOutput:
          return std_out;
        case QProcess::StandardError:
          return std_err;
        default:
          return std_out;
      }
    }

    //! Return the job results.
    QMap <comp::JobResult::ResultType, comp::JobResult*> jobResults() {return job_results;}

    //! Return the job step tmp directory path.
    QString jobStepTempDirPath() const {return js_tmp_dir_path;}

  signals:

    //! Emit job step completion status.
    void sig_jobStepFinishState(int placement, bool successful);

  private:

    //! Perform keyword replacement on the command and returns whether 
    //! the replacement took place.
    //! TODO implement some sort of "path role" which determines which types of
    //! replacements can be done to a certain path.
    bool commandKeywordReplacement();

    // variables from GUI/initial setup
    PluginEngine *engine;
    QStringList command_format;
    QMap<QString, QString> job_params;

    // pre-invocation variables
    int placement=-1;                       // execution order of this step within the job
    JobStepState job_step_state=NotInvoked; // job step run state
    QStringList command;                    // the invocation command
    QProcess *process=nullptr;              // the program process
    QString job_tmp_dir_path;               // temp directory shared among steps
    QString js_tmp_dir_path;                // temp directory dedicated to this job step
    QString problem_path;                   // problem file path
    QString result_path;                    // result file path

    // post-invocation, runtime-related variables
    QDateTime start_time;                   // start time of this job step
    QDateTime end_time;                     // end time of this job step
    QString std_out;                        // stdout from process
    QString std_err;                        // stderr from process
    int exit_code=-1;                       // exit code of the process, -1 if haven't invoked nor finished
    QProcess::ExitStatus exit_status;       // exit status of the process (normal or crashed)

    // post-invocation, results-related variables
    bool results_read=false;                // indicates whether results have been read
    QMap<comp::JobResult::ResultType, comp::JobResult*> job_results;  // store job results
  };



  //! A job is a collection of job steps and manages the progress of the 
  //! job.
  class SimJob : public QObject
  {
    Q_OBJECT

  public:

    struct GuiControlElems {
      GuiControlElems(SimJob *job) : job(job)
      {
        pb_terminate = new QPushButton("Terminate");
        pb_job_terminal = new QPushButton("Log");
        pb_sim_visualize = new QPushButton("Visualize Results");
        pb_export_results = new QPushButton("Export Results");

        connect(pb_job_terminal, &QPushButton::clicked,
                [job](){job->terminalOutputDialog()->show();});
        connect(pb_terminate, &QPushButton::clicked,
                [job](){
                  QMessageBox msg;
                  msg.setText("Are you sure that you would like to terminate the job?\nNote: multi-threaded plugins may leave behind orphaned children processes depending on implementation.");
                  msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                  msg.setDefaultButton(QMessageBox::No);
                  if (msg.exec() == QMessageBox::Yes) {
                    job->terminateJob();
                  }
                });
        connect(pb_sim_visualize, &QPushButton::clicked,
                [job](){emit job->sig_requestJobVisualization(job);});
        connect(pb_export_results, &QPushButton::clicked,
                [job](){job->exportJob();});
      }

      SimJob *job=nullptr;
      QPushButton *pb_terminate=nullptr;
      QPushButton *pb_job_terminal=nullptr;
      QPushButton *pb_sim_visualize=nullptr;
      QPushButton *pb_export_results=nullptr;
    };

    enum JobState{NotInvoked, Running, FinishedWithError, FinishedNormally};
    Q_ENUM(JobState);

    enum JobInfoStandardItemField{JobNameField, JobStartTimeField, 
      JobEndTimeField, JobStepCountField, JobFinishStateField, JobTempPathField};
    Q_ENUM(JobInfoStandardItemField);

    //! Constructor.
    SimJob(const QString &nm, QWidget *parent=nullptr);

    //! XML import constructor.
    //! If dcmp is true, then assume that fpath is an archive; if dcmp is false,
    //! then assume that fpath is the manifest XML path.
    //! For name_override, use string @IMPORTED_NAME@ to denote the original
    //! name in the manifest. For example, you can provide:
    //! "IMPORTED_@IMPORTED_NAME@"
    SimJob(const QString &fpath, bool dcmp, QString name_override=QString(),
        QWidget *parent=nullptr);

    //! Destructor.
    ~SimJob();


    // JOB SETUP

    //! Append a job step.
    void addJobStep(JobStep *js) {job_steps.append(js);}

    //! Return the job step at the specified index.
    JobStep *getJobStep(int i) {return job_steps.at(i);}

    //! Return a pointer to the list of all job steps.
    QList<JobStep*> jobSteps() {return job_steps;}

    //! Write the manifest of this job step to the default file location.
    void writeManifest(QString fpath="");

    //! Write the job manifest.
    void writeManifest(QXmlStreamWriter *ws);

    // OTHER SETTINGS

    //! Set the inclusion area.
    void setInclusionArea(gui::DesignInclusionArea a) {inclusion_area = a;}

    //! Return the inclusion area.
    gui::DesignInclusionArea inclusionArea() {return inclusion_area;}


    // JOB EXECUTION

    //! Confirm the job steps order placement, must be done before execution 
    //! begins (beginJob() does this if it hasn't already been done elsewhere).
    void confirmJobStepsPlacement();

    //! Prepare the job and contained job steps for invocation.
    void prepareJob();

    //! Begin execution sequence - the first job step would be invoked, 
    //! appropriate signals connected and at the end of each job step the next 
    //! one would be invoked. Returns whether the job has begun execution.
    bool beginJob();

    //! Continue the job if previous step was successful, stop it otherwise.
    void continueJob(int prev_step_ind, bool prev_step_successful);

    //! Terminal the running job step process and prevent remaining job steps 
    //! from executing.
    void terminateJob();

    //! Job finish actions.
    void jobFinishActions(JobState);

    //! Return a list of QStandardItems containing generic information relevant 
    //! to this job.
    QList<QStandardItem*> jobInfoStandardItemRow(QList<JobInfoStandardItemField> fields=QList<JobInfoStandardItemField>());

    //! Static method to show a dialog to choose a job to import.
    static SimJob *importSimJob(QString fname=QString())
    {
      if (fname.isNull()) {
        qDebug() << "No path was provided for import, prompt to choose from dialog.";
        fname = QFileDialog::getOpenFileName(nullptr, tr("Open File"));
        if (fname.isNull()) {
          qDebug() << "No path was provided in the dialog. Import halted.";
          return nullptr;
        }
      }
      return new SimJob(fname, true, "IMP_@IMPORTED_NAME@", nullptr);
    }


    // ACCESSORS

    //! Default job name.
    static QString defaultJobName() 
    {
      return "SIM_" + QDateTime::currentDateTime().toString("yyMMdd_HHmmss");
    }

    //! Name of this job.
    QString name() const {return job_name;}

    //! Set the name of this job.
    //! TODO add GUI element for renaming jobs.
    void setName(const QString &t_name) {job_name = t_name;}

    //! Runtime temporary directory (all job steps share the same dir).
    QString runtimeTempPath();

    //! Return the overall start time of the job (start time of the first step).
    QDateTime startTime() const {return job_steps.first()->startTime();}

    //! Return the overall end time of the job (end time of the last step).
    QDateTime endTime() const {return job_steps.last()->endTime();}

    //! Return the current job state.
    JobState jobState() const {return job_state;}

    //! Return GUI control elements.
    GuiControlElems guiControlElems() const {return gui_ctrl_elems;}

    //! Return a QMap of result types mapped to job steps that have that type
    //! of result.
    QMultiMap<comp::JobResult::ResultType, JobStep*> resultTypeStepMap() {return result_type_step_map;}

    //! Show a dialog containing the job's terminal output.
    QWidget *terminalOutputDialog(QWidget *parent=nullptr, Qt::WindowFlags w_flags=Qt::Dialog);

    //! Export the finished SimJob into an archive and return whether the export 
    //! was successful.
    bool exportJob(QString outpath=QString());


  signals:

    //! Export problem files.
    void sig_exportJobStepProblem(JobStep *job_step, gui::DesignInclusionArea inclusion_area);

    //! Emit the job finish state.
    void sig_jobFinishState(SimJob *job, JobState finish_state);

    //! Request the job results to be shown.
    void sig_requestJobVisualization(SimJob *job);


  private:

    // variables
    JobState job_state;                 // the state of the job
    QList<JobStep*> job_steps;          // list of steps in this simulation job, each step invokes one simulation
    QMultiMap<comp::JobResult::ResultType, JobStep*> result_type_step_map;  // all result types contained in job steps
    bool placement_confirmed=false;     // the job steps execution order has been confirmed, must be true before execution begins
    gui::DesignInclusionArea inclusion_area=gui::IncludeEntireDesign;       // the inclusion area for this job
    QString job_name;                   // job name for identification
    QString job_tmp_dir_path;           // job directory for storing runtime data
    QDateTime start_time, end_time;     // start and end times of the job
    JobStep *curr_step=nullptr;
    GuiControlElems gui_ctrl_elems;     // store GUI control elements
    bool imported=false;

    // read xml
    QStringList ignored_xml_elements; // XML elements to ignore when reading results
  };

} // end of comp namespace

#endif
