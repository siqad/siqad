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

  struct EngineDataset
  {
    //! Empty constructor.
    EngineDataset() {};

    //! Construct a dataset using engine defaults.
    EngineDataset(prim::SimEngine *t_engine)
      : engine(t_engine)
    {
      interp_format = engine->interpreter();
      if (engine->commandFormats().length() > 0)
        command_format = engine->commandFormats().at(0).second;
      prop_form = new PropertyForm(engine->sim_params_map);
    }

    //! Construct a dataset with all values specified.
    EngineDataset(prim::SimEngine *t_engine, const QString &t_interp_format,
                  const QString &t_command_format, PropertyForm *t_prop_form)
      : engine(t_engine), interp_format(t_interp_format), 
        command_format(t_command_format), prop_form(t_prop_form) {};

    //! Check whether this dataset is empty.
    bool isEmpty()
    {
      return engine == nullptr;
    }

    prim::SimEngine *engine=nullptr;
    QString interp_format;
    QString command_format;
    PropertyForm *prop_form=nullptr;
  };

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

  // map QListWidgetItem to the corresponding engine property form
  //QMap<QListWidgetItem*,QPair<QString, PropertyForm*>> eng_list_property_form; // TODO remove
  QMap<QListWidgetItem*, EngineDataset*> eng_datasets;

  // dialogs
  QWidget *sim_manager_dialog;
  QWidget *sim_setup_dialog;
  PropertyForm *curr_sim_params_form=0;

  // manager panes
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
