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
#include "../components/plugin_engine.h"
//#include "../components/sim_engine.h" TODO remove
#include "../components/sim_job.h"

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
    EngineDataset(comp::PluginEngine *t_engine)
      : engine(t_engine)
    {
      id = t_engine->uniqueIdentifier();
      if (engine->commandFormats().length() > 0)
        command_format = engine->jointCommandFormat(0).second;
      prop_form = new PropertyForm(engine->defaultPropertyMap());
    }

    //! Construct a dataset with all values specified.
    EngineDataset(uint t_id, comp::PluginEngine *t_engine, 
                  const QString &t_command_format, PropertyForm *t_prop_form)
      : id(t_id), engine(t_engine), command_format(t_command_format), 
        prop_form(t_prop_form) {};

    //! Check whether this dataset is empty.
    bool isEmpty()
    {
      return engine == nullptr;
    }

    uint id=0;
    comp::PluginEngine *engine=nullptr;
    QString command_format;           // command format with arguments delimited by "\n".
    PropertyForm *prop_form=nullptr;
  };

  // constructor
  SimManager(QWidget *parent = 0);

  // destructor
  ~SimManager();

  // manager actions
  bool addJob(comp::SimJob *job);   // add a simulation job

  // variables
  // TODO remove QList<comp::SimEngine*>  sim_engines;   // stack of all simulators
  QList<comp::SimJob*>     sim_jobs;      // stack of all jobs


signals:
  void sig_simJob(comp::SimJob *new_job);

protected:

  //! Override the show event to update job time title every time the widget is
  //! shown.
  void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;

public slots:

  //! Without showing the dialog, just activate "Run" and start a job with
  //! whatever settings are on the dialog.
  void quickRun();

private slots:
  void submitSimSetup();

private:

  // sim manager related (like showing all jobs, all engines, etc.)
  void initSimManager();

  //! Return a default generic job name with date time.
  QString defaultJobName();

  //! Save simulation settings of the currently selected engine as a preset 
  //! available for future reuse. A pop-up dialog prompts for the desired 
  //! preset name.
  void saveSimulationPreset();

  //! Export simulation settings of the currently selected engine as a preset 
  //! to the given file path.
  //! NOTE work in progress
  void exportSimulationPreset(const QString &f_path);

  //! Import simulation settings preset from the given file path for the 
  //! currently selected engine.
  //! NOTE work in progress
  //! TODO check and report differences in available setting fields.
  void importSimulationPreset(const QString &f_path);

  // save or reset engine settings
  // TODO remove these after completing the preset feature.
  void saveSettingsAsDefault();
  void resetToUserDefault();
  void resetToEngineDefault();

  // Map unique engine identifier to simulation engines (plugin engines which 
  // provide simulation service)
  QMap<uint, comp::PluginEngine*> plugin_engines;

  // map QListWidgetItem to the corresponding engine property form
  //QMap<QListWidgetItem*,QPair<QString, PropertyForm*>> eng_list_property_form; // TODO remove
  QMap<QListWidgetItem*, EngineDataset*> eng_datasets;

  // dialogs
  QWidget *sim_manager_dialog;

  // manager panes
  QLineEdit *le_job_name;
};


} // end gui namespace

#endif
