// @file:     sim_job.h
// @author:   Samuel
// @created:  2017.10.10
// @editted:  2017.10.10 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     SimJob object that describes a simulation job and stores the results from that job

#ifndef _PRIM_SIM_JOB_H_
#define _PRIM_SIM_JOB_H_

#include <QtWidgets>
#include <QtCore>
#include "sim_engine.h"
#include "src/settings/settings.h" // TODO probably need this later

namespace prim{

  class SimJob : public QObject
  {
    Q_OBJECT
  public:
    // constructor
    SimJob(const QString &nm, SimEngine *eng=0, QWidget *parent=0);

    // destructor
    ~SimJob() {};

    // elec_dists struct
    struct elecDist{
      QString dist_str;
      QList<int> dist_ls; // TODO might not be necessary
      float energy;
    };

    void setEngine(SimEngine *eng) {engine = eng;}


    // load job from XML (for jobs that keep running even if parent terminates)
    // TODO sim_manager probably needs to check folders for unfinished simulations
    bool loadJob(const QString &job_path);

    // call sim engine binary
    bool invokeBinary();

    // result related tasks
    bool readResults();  // read result XML
    bool processResults();                // process results


    // ACCESSORS
    QString name() {return job_name;}
    QString engineName() {return engine ? engine->name() : "SimAnneal";} // TODO cheating here, change SimAnneal back to Undefined later
    QString runtimeTempDir();     // runtime job directory
    QString problemFile();        // runtime problem file
    QString resultFile();         // runtime result file
    QDateTime startTime() {return start_time;}
    QDateTime endTime() {return end_time;}
    bool isComplete() {return completed;} // indicate whether the job has been completed
    int distCount() {return dist_count;}  // return the number of charge distributions this has
    

    // variables TODO put them back to private later, with proper accessors
    QList<QPair<float,float>> physlocs;   // physlocs[dot_ind].first or .second
    QList<QList<int>> elec_dists;         // elec_dists[result_ind][dot_ind] TODO change this to QList of QVectors
    //QList<elecDist> elec_dists;
  private:

    void deduplicateDist();               // deduplicate charge distribution results
    
    // variables
    // TODO struct job_desc
    QString job_name;
    SimEngine *engine;
    QString run_job_dir;                  // job directory for storing runtime data, must access with accessor
    QString problem_path;
    QString result_path;
    QDateTime start_time, end_time;
    QProcess *sim_process;
    QStringList arguments;
    bool completed;                       // whether the job has completed simulation

    // read xml
    QStringList ignored_xml_elements;     // XML elements to ignore when reading results

    // parameters
    //QList<QPair<the field stuff, value>> sim_params;

    // results
    // TODO struct results
    int dist_count;                       // number of distributions
  };

} // end of prim namespace

#endif
