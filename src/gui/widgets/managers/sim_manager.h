// @file:     sim_manager.h
// @author:   Samuel
// @created:  2017.09.27
// @editted:  2017.09.27 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     window that allows users to setup and dispatch new simulations,
//            as well as manage ongoing or completed simulations.

#ifndef _GUI_SIM_MANAGER_H_
#define _GUI_SIM_MANAGER_H_

#include <QtWidgets>
#include <QDialog>
#include <QShortcut>

#include "../property_form.h"
#include "../primitives/sim_engine.h"
#include "../primitives/sim_job.h"

namespace gui{

class SimManager : public QWidget
{
  Q_OBJECT

public:

  // constructor
  SimManager(QWidget *parent = 0);

  // destructor
  ~SimManager();

  // show sim setup dialog
  void showSimSetupDialog();

  // manager actions
  bool addJob(prim::SimJob *job);   // add a simulation job

  // ACCESSORS
  QComboBox *getComboEngSel(){return cb_eng_sel;}

  // various ways to get simulation engine
  prim::SimEngine *getEngine(int index) {return (index >= 0 && index < sim_engines.length()) ? sim_engines.at(index) : 0;}  // by index
  prim::SimEngine *getEngine(const QString &name);  // by name

  // variables
  QList<prim::SimEngine*>  sim_engines;   // stack of all simulators
  QList<prim::SimJob*>     sim_jobs;      // stack of all jobs


signals:
  void sig_simJob(prim::SimJob *new_job);

public slots:

  //! Without showing the dialog, just activate "Run" and start a job with
  //! whatever settings are on the dialog.
  void quickRun();

private slots:
  void updateSimParams();
  void submitSimSetup();

private:
  // initialize python path
  void initPythonPath();
  bool findWorkingPythonPath();

  // sim manager related (like showing all jobs, all engines, etc.)
  void initSimManager();

  // sim setup dialog is responsible for setting up new simulation jobs to run
  void initSimSetupDialog();
  void updateEngineSelectionList();

  //! Return a default generic job name with date time.
  QString defaultJobName();

  //! Save simulation settings of the currently selected engine as a preset 
  //! available for future reuse. A pop-up dialog prompts for the desired 
  //! preset name.
  void saveSimulationPreset();

  //! Export simulation settings of the currently selected engine as a preset 
  //! to the given file path.
  void exportSimulationPreset(const QString &f_path);

  //! Import simulation settings preset from the given file path for the 
  //! currently selected engine.
  //! TODO check and report differences in available setting fields.
  void importSimulationPreset(const QString &f_path);

  // save or reset engine settings
  // TODO remove these after completing the preset feature.
  void saveSettingsAsDefault();
  void resetToUserDefault();
  void resetToEngineDefault();

  void initEngines();

  // dialogs
  QWidget *sim_manager_dialog;
  QWidget *sim_setup_dialog;
  PropertyForm *curr_sim_params_form=0;

  // manager panes
  QListWidget *sim_list_pan;
  QVBoxLayout *sim_actions_pan;
  QGroupBox *sim_params_group;
  QVBoxLayout *sim_params_vl;
  QLineEdit *le_job_nm;

  // button group
  QHBoxLayout *bottom_buttons_hl;
  QVBoxLayout *new_setup_dialog_l;
  QPushButton *button_run;
  QPushButton *button_cancel;
  QShortcut *shortcut_enter;
  QComboBox *cb_eng_sel;
};


} // end gui namespace

#endif
