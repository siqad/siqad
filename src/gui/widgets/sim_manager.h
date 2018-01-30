// @file:     sim_manager.h
// @author:   Samuel
// @created:  2017.09.27
// @editted:  2017.09.27 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     window that allows users to setup and dispatch new simulations,
//            as well as manage ongoing or completed simulations.

#ifndef _GUI_SIM_MANGER_H_
#define _GUI_SIM_MANGER_H_

#include <QtWidgets>
#include <QDialog>
#include "primitives/sim_engine.h"
#include "primitives/sim_job.h"

namespace gui{

class SimManager : public QWidget
{
  Q_OBJECT

public:

  // constructor
  SimManager(QWidget *parent = 0);

  // destructor
  ~SimManager();

  // show manager dialog
  void showManager() {show();}

  // show sim setup dialog
  void showSimSetupDialog();

  // manager actions
  void newSimSetup();               // walks user through simulation setup
  bool addJob(prim::SimJob *job);   // add a simulation job

  // ACCESSORS

  // various ways to get simulation engine
  prim::SimEngine *getEngine(int index) {return (index >= 0 && index < sim_engines.length()) ? sim_engines.at(index) : 0;}  // by index
  prim::SimEngine *getEngine(const QString &name);  // by name

  // TODO get job by name
  

  // variables
  QList<prim::SimEngine*>  sim_engines;  // stack of all simulators
  QList<prim::SimJob*>     sim_jobs;     // stack of all jobs

signals:
  void emitSimJob(prim::SimJob *new_job);

private slots:
  void updateSimParams();
  void xmlFind();

private:
  void initSimManager();
  void initMenu();
  void initListPan();
  void initSimActionsPan();

  void initSimSetupDialog();
  void updateEngineSelectionList();
  void submitSimSetup();
  void createButtonLayout();

  void initEngines();
  void simParamSetup();             // take user options for simulation parameters
  bool exportSimProblem();          // generate problem XML

  // dialogs
  QWidget *sim_manager_dialog;
  QWidget *sim_setup_dialog;        // TODO maybe make a subclass for this
  bool using_engine_sim_param_dialog=false;

  // manager panes
  QListWidget *sim_list_pan;
  QVBoxLayout *sim_actions_pan;
  QGroupBox *sim_params_group;
  QVBoxLayout *sim_params_vl;
  QComboBox *combo_eng_sel;

  // sim_setup_dialog fields
  // SimAnneal
  QLabel *label_result_queue_size;
  QLabel *label_preanneal_cycles;
  QLabel *label_anneal_cycles;
  QLabel *label_global_v0;
  QLabel *label_debye_length;
  QLineEdit *le_result_queue_size;
  QLineEdit *le_job_nm;
  QLineEdit *le_preanneal_cycles;
  QLineEdit *le_anneal_cycles;
  QLineEdit *le_global_v0;
  QLineEdit *le_debye_length;

  // PoisSolver
  QLabel *label_xml_path;
  QLineEdit *le_xml_path;
  QPushButton *button_xml_find;

  // button group
  QHBoxLayout *bottom_buttons_hl;
  QVBoxLayout *new_setup_dialog_l;
  QPushButton *button_run;
  QPushButton *button_cancel;
};


} // end gui namespace

#endif
