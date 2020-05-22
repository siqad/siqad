// @file:     application.h
// @author:   Jake
// @created:  2016.10.31
// @editted:  2017.05.09  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Customized QMainWindow class, ApplicationGUI, for the GUI.

#ifndef _UI_APPLICATION_H_
#define _UI_APPLICATION_H_

// Qt includes
#include <QtWidgets>
#include <QDialog>

// Global
#include "../global.h"

// Settings Dialog
#include "../settings/settings_dialog.h"

// Widget includes
#include "widgets/design_panel.h"
#include "widgets/dialog_panel.h"
#include "widgets/input_field.h"
#include "widgets/info_panel.h"
#include "widgets/managers/sim_manager.h"
#include "widgets/managers/plugin_manager.h"
#include "widgets/managers/job_manager.h"
#include "widgets/components/sim_job.h"
#include "widgets/visualizers/sim_visualizer.h"
#include "commander.h"


// declare Application GUI in Ui namespace
namespace Ui
{
  class ApplicationGUI;
} // end namespace Ui

namespace gui{

  class ApplicationGUI : public QMainWindow
  {
    Q_OBJECT

  public:

    enum SaveFlag{Save, SaveAs, AutoSave, SaveSimulationProblem};

    // constructor
    explicit ApplicationGUI(const QString &f_path=QString(), QWidget *parent=0);

    // destructor
    ~ApplicationGUI();

    // static declaration of DialogPanel for dialogstream
    static gui::DialogPanel *dialog_pan;

  public slots:

    // update the window title
    void updateWindowTitle();

    // cursor tool updating
    void setTool(gui::ToolType tool);
    void setToolSelect();
    void setToolDrag();
    void setToolDBGen();
    void setToolElectrode();
    void setToolAFMArea();
    void setToolAFMPath();
    void setToolScreenshotArea();
    void setToolScaleBarAnchor();
    void setToolLabel();

    // add or remove actions from sidebar
    void showActionList(QList<QAction*>);
    void hideActionList(QList<QAction*>);

    // change lattice
    void changeLattice();

    // parse input field and act accordingly
    void parseInputField();

    // Repeat previous simulation
    void repeatSimulation();

    // Read simulation results
    bool readSimResult(const QString &result_path);

    // SANDBOX

    void selectColor(); // tool for converting colors into QVariant strings

    // Screenshots

    //! Initiate Screenshot Mode which allows the user to pick an area to
    //! screenshot.
    void beginScreenshotMode();

    //! End Screenshot mode.
    void endScreenshotMode();

    //! Toggle screenshot mode.
    void toggleScreenshotMode() {
      design_pan->displayMode() == gui::ScreenshotMode ? endScreenshotMode() : beginScreenshotMode();
    }

    //! Take an svg capture of the entire GUI.
    void screenshot();

    //! Take an svg capture of the design window currently shown.
    void fullDesignScreenshot();

    //! Take an svg capture of the design window in the given QRect (scene coord).
    void designScreenshot(const QString &target_img_path, QRectF rect, bool always_overwrite);

    //! Pop-up dialog to resolve unsaved changes (save or discard).
    bool resolveUnsavedChanges();

    //! Close current design and start new design.
    void newFile();

    //! Save design to file.
    bool saveToFile(SaveFlag flag=Save, const QString &path=QString(),
                    gui::DesignInclusionArea inclusion_area=gui::IncludeEntireDesign,
                    comp::JobStep *job_step=nullptr);

    //! Perform autosave.
    void autoSave();

    //! Open a previous save. A file chooser dialog would be presented if no
    //! file path is given.
    void openFromFile(const QString &f_path=QString());

    // Export to Labview
    bool exportToLabview();

    // About and Version
    void aboutVersion();

  protected:

    //! Override the close event to capture window close events.
    void closeEvent(QCloseEvent *e) Q_DECL_OVERRIDE;

    //! Override the drop enter event to allow opening files through dropping.
    void dragEnterEvent(QDragEnterEvent *e) Q_DECL_OVERRIDE;

    //! Override the drop event to allow opening files through dropping.
    void dropEvent(QDropEvent *e) Q_DECL_OVERRIDE;

  private:

    // graphics initialisation
    void initGUI();               // initialise the mainwindow GUI
    void initMenuBar();           // initialise the GUI menubar
    void initTopBar();            // initialise the GUI topbar, toolbar
    void initSideBar();           // initialise the GUI sidebar, toolbar
    void initDialogDock();        // initialise the bottom dialog dock
    void initSimVisualizerDock();  // initialise the side sim visualize dock
    void initLayerDock();         // initialise the side layer dock
    void initCommander();         // initialise the input field whitelist
    void initItemDock();          // initialise the side item dock
    void initInfoDock();          // initialise the bottom info dock
    // void initColorDialog();       // initialise the color dialog for changing item colors.
    void setLayerManagerWidget(QWidget *widget);
    void setItemManagerWidget(QWidget *widget);

    // prepare any extra actions not attched to an icon or meny
    void initActions();

    // prepare the initial GUI state
    void initState();

    // application settings
    void loadSettings();  // load mainwindow settings from the settings instance
    void saveSettings();  // save mainwindow settings to the settings instance

    // VARIABLES

    // flag to indicate closing/quitting
    bool is_closing = false;
    // display mode cache for returning to previous mode
    DisplayMode display_mode_cache;

    // save start time for instance recognition
    QDateTime start_time;

    // directory path persistence
    QDir img_dir;
    QDir save_dir;

    // purely graphics widgets
    QToolBar *top_bar;
    QToolBar *side_bar;

    // functional widgets, DialogPanel is a static in public above.
    gui::DesignPanel    *design_pan;      // mainwindow design panel
    gui::InfoPanel      *info_pan;        // mainwindow info panel
    gui::InputField     *input_field;     // mainwindow input field
    gui::SimManager     *sim_manager;     // pop-up simulator manager
    gui::PluginManager  *plugin_manager;  // pop-up plugin manager
    gui::JobManager     *job_manager;     // pop-up job manager
    gui::SimVisualizer   *sim_visualize;   // simulation visualizer that goes in sim visualize dock
    settings::SettingsDialog *settings_dialog;  // dialog for changing settings

    // dockable widgets
    QDockWidget *dialog_dock; // bottom panel for terminal dialog
    QDockWidget *sim_visualize_dock; // right side panel for sim visualization
    QDockWidget *layer_dock;  // right side panel for showing layers
    QDockWidget *item_dock;   // right side panel for showing items
    QDockWidget *info_dock;   // bottom panel that shows information about items on screen

    // action groups
    QActionGroup *ag_design=nullptr;              // action group containing design related actions

    // action lists
    QList<QAction*> al_screenshot;                // action list containing screenshot actions

    // action pointers
    QAction *action_select_tool=nullptr;          // change cursor tool to select
    QAction *action_drag_tool=nullptr;            // change cursor tool to drag
    QAction *action_dbgen_tool=nullptr;           // change cursor tool to gen
    QAction *action_electrode_tool=nullptr;       // change cursor tool to electrode
    QAction *action_afmarea_tool=nullptr;         // change cursor tool to AFM Area tool
    QAction *action_afmpath_tool=nullptr;         // change cursor tool to AFM Path tool
    QAction *action_label_tool=nullptr;           // change cursor tool to label
    QAction *action_run_sim=nullptr;              // run the current simulation method
    QAction *action_repeat_sim=nullptr;           // repeat the previous simulation
    QAction *action_run_ground_state=nullptr;     // run the current simulation method
    QAction *action_sim_visualize=nullptr;        // show the sim visualize dock which allows simulation visualization
    QAction *action_layer_sel=nullptr;
    QAction *action_item_manager=nullptr;
    QAction *action_circuit_lib=nullptr;
    QAction *action_dialog_dock_visibility=nullptr;
    QAction *action_screenshot_mode=nullptr;      // toggle screenshot mode
    QAction *action_screenshot_area_tool=nullptr;
    QAction *action_scale_bar_anchor_tool=nullptr;

    // save file
    QTimer autosave_timer;     // timer for autosaves
    QDir autosave_root;        // the root of autosave directories
    QDir autosave_dir;         // directory of autosave

    int autosave_ind=0;        // current autosave file index
    int autosave_num;          // number of autosaves to keep

    QString working_path;      // path currently in use
    Commander* commander;      // Handles commands

    bool reset_settings=false; // reset all settings at destruction

  };

} // end gui namespace

#endif
