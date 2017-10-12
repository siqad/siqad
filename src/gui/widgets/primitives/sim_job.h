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
    SimJob(SimEngine *eng, QWidget *parent=0);

    // destructor
    ~SimJob() {};

    void setEngine(SimEngine *eng) {engine = eng;}


    // load job from XML (for jobs that keep running even if parent terminates)
    // TODO sim_manager probably needs to check folders for unfinished simulations
    bool loadJob(const QString &job_path);

    // call sim engine binary
    bool invokeBinary();

    // read result XML
    bool readResults(QString read_path);

    

  private:
    SimEngine *engine;
    QProcess *sim_process;
    QStringList arguments;

    // read xml
    QStringList ignored_xml_elements; // XML elements to ignore when reading results

    // parameters
    //QList<QPair<the field stuff, value>> sim_params;

    // results
    QList<QPair<float,float>> physlocs;   // physlocs[dot_ind].first or .second
    QList<QList<bool>> elec_dists;        // elec_dists[result_ind][dot_ind]
  };

} // end of prim namespace

#endif
