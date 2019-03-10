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
#include <QDir>

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

    //! Set the simulation interpreter using the given format. TODO path replacement instructions
    void setInterpreterFormat(const QString &t_interp_format) {interp_format = t_interp_format;}

    //! Set the simulation invocation command format.
    void setCommandFormat(const QString &t_command_format) {command_format = t_command_format;}

    // simulation parameters

    //! Returns a list key-value pairs representing simulation parameters.
    QList<QPair<QString, QString>> simParams() const {return sim_params;}

    //! Appends two strings to the list of simulation parametres as a key-value pair
    void addSimParam(const QString &field, const QString &value) {sim_params.append(qMakePair(field, value));}

    //! Appends a list of key-value pairs to the list of simulation parameters
    void addSimParams(const QList<QPair<QString, QString>> &add_params) {sim_params.append(add_params);}

    //! Appends all entries in the provided PropertyMap to the list of simulation parameters
    void addSimParams(const gui::PropertyMap &sim_param_map);


    // JOB EXECUTION

    //! call sim engine binary
    bool invokeBinary();


    // JOB RESULT

    bool readResults();     //!< read result XML

    // result storage and access
    //! elec_dists struct for showing where electrons are
    struct elecDist{
      QList<int> dist;
      float energy=0;     // energy of this distribution
      int config_count=0; // how many times does this distribution occur
      int elec_count=0;   // number of electrons in this configuration

      bool operator < (const elecDist &other) const {
        return (energy < other.energy);
      }

      bool operator == (const elecDist &other) const {
        if (dist.size() != other.dist.size() ||
            energy != other.energy || 
            config_count != other.config_count ||
            elec_count != other.elec_count)
          return false;
        for (int i=0; i<dist.size(); i++)
          if (dist.at(i) != other.dist.at(i))
            return false;
        return true;
      }
    };

    //! Process electron distributions after reading from XML.
    void processElecDists(QMap<QString, elecDist> elec_dists_map);

    //! Apply a filter to the electron distributions. If elec_count == -1, the
    //! filter would be cancelled.
    void applyElecDistsFilter(int elec_count=-1);

    //! Return list of electron distributions with the applied filter, or the 
    //! entire list if no filters have been applied.
    QList<elecDist> filteredElecDists() {return elec_dists_filtered;}

    //! Find degenerate average occupation.
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

    QString runtimeTempPath();        //!< runtime job directory
    QString problemFilePath();        //!< runtime problem file
    QString resultFilePath();         //!< runtime result file

    QDateTime startTime() {return start_time;}
    QDateTime endTime() {return end_time;}

    bool isComplete() {return completed;} //!< indicate whether the job has been completed

    QString terminalOutput() {return terminal_output;}
    void saveTerminalOutput();

    // variables TODO put them back to private later, with proper accessors
    QList<QVector<float>> potentials; //!< potentials[result_ind][0] is x, ...[1] is y, ...[2] is potential value
    QList<QPair<float,float>> physlocs;   //!< physlocs[dot_ind].first or .second

    // electron distribution
    int preferred_elec_count=0;           //! the default dist selection will be of this electron count
    int default_elec_dist_ind=0;
    QList<int> elec_counts;               //! available electron counts
    QList<elecDist> elec_dists;           //! electron distributions
    QList<elecDist> elec_dists_filtered;  //! filtered electron distribution
    QList<float> elec_dists_avg;          //! the average charge across all dots

    QList<LineScanPath> line_scan_paths;  //!< line scan path props and results
  private:

    void deduplicateDist();           // deduplicate charge distribution results

    // variables
    QString job_name;         // job name for identification
    QString interp_format;    // format for interpreter path or command
    QString command_format;   // format of the invocation command
    SimEngine *engine;        // the engine used by this job
    QString job_tmp_dir_path; // job directory for storing runtime data
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
