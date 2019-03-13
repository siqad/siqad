// @file:     job_manager.h
// @author:   Samuel
// @created:  2019.03.14
// @license:  GNU LGPL v3
//
// @desc:     Manage all plugin jobs both setting them up, keeping track of 
//            them, as well as sending results to the appropriate target widget.

#ifndef _GUI_JOB_MANAGER_H_
#define _GUI_JOB_MANAGER_H_

#include <QtWidgets>

#include "plugin_manager.h"
#include "../property_form.h"
#include "../components/plugin_engine.h"
#include "../components/sim_job.h"
#include "../visualizers/sim_visualizer.h"

namespace gui{

  // forward declarations
  class JobStepViewListItem;
  class JobSetupDetailsPane;

  class JobManager : public QWidget
  {
    Q_OBJECT

  public:

    //! Engine dataset owned by the standard item representing a job step when 
    //! setting up a plugin job. This struct is accessible through QVariant.
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

    //! Constructor.
    JobManager(PluginManager *plugin_manager, SimVisualizer *sim_visualizer, 
               QWidget *parent=nullptr);

    //! Destructor.
    ~JobManager();

    //! Add a job to the manager and connect appropriate signals. If the job
    //! pointer already exists in the manager, does nothing. Does not invoke
    //! the job.
    void addJob(comp::SimJob *job);

    //! Run the specified job, if the job hasn't already been added to the 
    //! manager it will be added.
    void runJob(comp::SimJob *job);

    //! Process a finished job.
    void processFinishedJob(comp::SimJob *job, comp::SimJob::JobState finish_state);

    //! Returns whether the job can be shown in SimVisualizer (might want to make
    //! this a SimVisualizer function instead).
    bool eligibleForSimVisualizer(comp::SimJob *job);

  signals:

    //! Request application to save a job problem file to the specified path, 
    //! can be used either in preparation of running a simulation or exporting 
    //! for future use.
    void sig_exportJobProblem(comp::JobStep *job_step, gui::DesignInclusionArea inclusion_area);

    //! Emit instruction for SimVisualizer to show simulation results.
    void sig_showSimResult(comp::SimJob *job);

    //! Emit a SiQAD command for commander to parse.
    void sig_executeSQCommand(QString command);

  private:

    //! Initialize the job setup widget GUI.
    void initJobManagerGUI();

    //! Initilize the job setup panel.
    QWidget *initJobSetupPanel();

    //! Initialize the job view panel.
    QWidget *initJobViewPanel();

    //! Return the engine currently selected on the engine list, or a null
    //! pointer if none is selected.
    comp::PluginEngine *selectedEngine();

    PluginManager *plugin_manager;
    SimVisualizer *sim_visualizer;         // pointer to the sim_visualizer

    QList<comp::SimJob*> sim_jobs;        // list of all jobs
    QListView *lv_engines;                // list view of engines in the engine list
    QListView *lv_job_steps;              // list view of job steps
    QTreeView *tv_job_view;               // tree view of all jobs
    QStandardItemModel *eng_model;        // data model storing the engines in lv_engines
    QSortFilterProxyModel *eng_filter_proxy_model;    // proxy model for filtering eng_model
    QStandardItemModel *cat_filter_model; // data model storing the filter items used for filtering eng_model
    QStandardItemModel *job_steps_model;  // data model storing the job steps engine sequence
    QStandardItemModel *job_view_model;   // data model storing the list of submitted and completed jobs
    QList<comp::PluginEngine::StandardItemField> eng_list_fields; // order of fields in eng_model

  };

  //! JobStepViewListItem is a QStandardItem subclass which is used specifically
  //! in lv_job_steps and job_steps_model.
  class JobStepViewListItem : public QStandardItem
  {
  public:

    //! Constructor.
    JobStepViewListItem(const QString &text)
      : QStandardItem(text)
    {};

    //! Destructor, clean up the engine dataset stored in this item.
    ~JobStepViewListItem()
    {
      if (eng_dataset != nullptr) {
        delete eng_dataset->prop_form;
        delete eng_dataset;
        eng_dataset = nullptr;
      }
    };

    JobManager::EngineDataset *eng_dataset=nullptr;

  };

  class JobSetupDetailsPane : public QWidget
  {
    Q_OBJECT

  public:

    struct JobDetails
    {
      QString name;
      gui::DesignInclusionArea inclusion_area;
    };

    //! Constructor.
    JobSetupDetailsPane(QWidget *parent=nullptr);

    //! Destructor.
    ~JobSetupDetailsPane() {};

    //! Return the pointer to the current engine dataset.
    JobManager::EngineDataset *engineDataset() {return eng_dataset;}

    //! Set the current engine dataset.
    void setEngineDataset(JobManager::EngineDataset *t_eng_dataset);

    //! Return the final job properties.
    JobDetails finalJobDetails()
    {
      JobDetails job_details;
      if (le_job_name != nullptr)
        job_details.name = le_job_name->text();
      typedef gui::DesignInclusionArea IA;
      QMetaEnum inc_a_enum = QMetaEnum::fromType<IA>();
      job_details.inclusion_area = static_cast<IA>(inc_a_enum.keyToValue(
            cbb_inclusion_area->currentText().toLatin1()));
      return job_details;
    }
    

  private:

    JobManager::EngineDataset *eng_dataset=nullptr; // currently used engine dataset
    QLineEdit *le_job_name;                         // job name
    QComboBox *cbb_inclusion_area;                  // inclusion area
    QMenu *menu_command_preset;                     // command format preset selection menu
    QTextEdit *te_command;                          // command format edit field
    QVBoxLayout *vl_plugin_params;                  // layout holding engine property form
  };


} // end of namespace gui

#endif
