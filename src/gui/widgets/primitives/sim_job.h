/** @file:     sim_job.h
 *  @author:   Samuel
 *  @created:  2017.10.10
 *  @editted:  2017.10.10 - Samuel
 *  @license:  GNU LGPL v3
 *
 *  @desc:     SimJob object that describes a simulation job and stores the results from that job
 */

#ifndef _PRIM_SIM_JOB_H_
#define _PRIM_SIM_JOB_H_

#include <QtWidgets>
#include <QtCore>
#include "sim_engine.h"
#include "src/settings/settings.h" // TODO probably need this later
#include <tuple> //std::tuple for 3+ article data structure, std::get for accessing the tuples

namespace prim{

  //! A single run of a simulation engine. Takes care of setting up simulation
  //! parameters, calling simulation engine binaries, and reading simulation
  //! results.
  class SimJob : public QObject
  {
    Q_OBJECT
  public:
    //! constructor
    SimJob(const QString &nm, SimEngine *eng=0, QWidget *parent=0);

    //! destructor
    ~SimJob() {};

    // JOB SETUP

    // TODO emit signal from SimJob to save job, instead of letting design panel call
    // invoke binary.
    // bool saveJob(const QString &);


    // load job from XML (for jobs that keep running even if parent terminates)
    // TODO sim_manager probably needs to check folders for unfinished simulations
    bool loadJob(const QString &) {return false;}

    // simulation parameters
    //! Returns a list key-value pairs representing simulation parameters.
    QList<QPair<QString, QString>> simParams() const {return sim_params;}
    //! Appends two strings to the list of simulation parametres as a key-value pair
    void addSimParam(const QString &field, const QString &value) {sim_params.append(qMakePair(field, value));}
    //! Appends a key-value pair to the list of simulation parameters
    void addSimParams(const QList<QPair<QString, QString>> &add_params) {sim_params.append(add_params);}
    QList<QPair<QString, QString>> loadSimParamsFromDialog();
    //! Loads simulation parameters from the SimManager dialog.
    void loadSimParamsFromEngineDialog();


    // JOB EXECUTION

    //! call sim engine binary
    bool invokeBinary();


    // JOB RESULT

    bool readResults();     //!< read result XML

    // result storage and access
    //! elec_dists struct for showing where electrons are
    struct elecDist{
      QList<int> dist;
      float energy;     // energy of this distribution
      int count=0;        // how many times does this distribution occur

      bool operator < (const elecDist &other) const {
        return (energy < other.energy);
      }
    };

    void processElecDists(QMap<QString, elecDist> elec_dists_map);
    float elecDistAvgDegenOfDB(int dist_ind, int db_ind);


    struct LineScanPath {
      QList<QPair<float,float>> afm_nodes;    //!< afm nodes in angstrom
      QList<QPair<float,float>> db_locs_enc;  //!< db locs in angstrom
      QList<QString> results;
    };

    // ACCESSORS

    //! job name
    QString name() {return job_name;}

    //! engine setter
    void setEngine(SimEngine *eng) {engine = eng;}
    //prim::SimEngine *engine() {return engine;}
    //! getter for engine name
    QString engineName() {return engine ? engine->name() : "Undefined";}

    QString runtimeTempDir();     //!< runtime job directory
    QString problemFile();        //!< runtime problem file
    QString resultFile();         //!< runtime result file

    QDateTime startTime() {return start_time;}
    QDateTime endTime() {return end_time;}

    bool isComplete() {return completed;} //!< indicate whether the job has been completed

    QString terminalOutput() {return terminal_output;}
    void saveTerminalOutput();

    // variables TODO put them back to private later, with proper accessors
    QList<QVector<float>> potentials; //!< potentials[result_ind][0] is x, ...[1] is y, ...[2] is potential value
    QList<QPair<float,float>> physlocs;   //!< physlocs[dot_ind].first or .second

    // electron distribution
    QList<elecDist> elec_dists;           //! electron distributions
    QList<float> elec_dists_avg;            //! the average charge across all dots

    QList<LineScanPath> line_scan_paths;  //!< line scan path props and results
  private:

    void deduplicateDist();           // deduplicate charge distribution results

    // variables
    QString job_name;         // job name for identification
    SimEngine *engine;        // the engine used by this job
    QString run_job_dir;      // job directory for storing runtime data
    QString problem_path;     // path to problem file
    QString result_path;      // path to result file
    QString terminal_output;  // terminal output from the job
    QDateTime start_time, end_time; // start and end times of the job
    QProcess *sim_process;    // runtime process of the job
    QStringList cml_arguments;// command line arguments when invoking the job
    bool completed;           // whether the job has completed simulation

    // read xml
    QStringList ignored_xml_elements; // XML elements to ignore when reading results

    // parameters
    QList<QPair<QString, QString>> sim_params;

    // results
    // TODO flag storing what types of results are available
  };

} // end of prim namespace

#endif
