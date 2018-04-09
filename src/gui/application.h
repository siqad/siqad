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

// Global
#include "../global.h"

// Settings Dialog
#include "../settings/settings_dialog.h"

// Widget includes
#include "widgets/design_panel.h"
#include "widgets/dialog_panel.h"
#include "widgets/input_field.h"
#include "widgets/info_panel.h"
#include "widgets/layer_editor.h"
#include "widgets/sim_manager.h"
#include "widgets/sim_visualize_panel.h"
#include "widgets/primitives/sim_job.h" // TODO move these stuff to SimManager later


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

    enum SaveFlag{Save, SaveAs, AutoSave, Simulation};

    // constructor
    explicit ApplicationGUI(QWidget *parent=0);

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

    // change lattice
    void changeLattice();

    // parse input field and act accordingly
    void parseInputField();

    void designPanelReset();

    // Start current simulation method
    // ... it might be worth modifying the work-flow such that instead of running
    // the simulation method we push the problem onto a working stack to be run
    // in the background: will need to be able to display results on request
    // after job finished.
    void simulationSetup();
    void runSimulation(prim::SimJob *job);             // high-level structure for running simulation
    bool readSimResult(const QString &result_path);                // read simulator output

    // SANDBOX

    void selectColor(); // tool for converting colors into QVariant strings

    // Screenshots

    //! Initiate Screenshot Mode which allows the user to pick an area to
    //! screenshot.
    void beginScreenshotMode();

    //! End Screenshot mode.
    void endScreenshotMode();

    //! Take an svg capture of the entire GUI.
    void screenshot();

    //! Take an svg capture of the design window.
    void designScreenshot();

    //! Take an svg capture of the design window in the given area.
    void designScreenshot(QRect rect);

    // show pop-up dialogs
    void showSettingsDialog() {settings_dialog->show();}

    // FILE HANDLING
    bool resolveUnsavedChanges();       // returns whether to proceed or not
    void newFile();                     // create a new file
    bool saveToFile(SaveFlag flag=Save, const QString &path=QString(), prim::SimJob *sim_job=0);   // actual save function
    void saveDefault();                 // save normally (calls saveToFile)
    void saveNew();                     // save as a new file (calls saveToFile)
    void autoSave();                    // perform autosave at specified interval (ms)
    void openFromFile();                // open a previous save
    void closeFile();                   // close the file when quitting the program

    // Export to Labview
    bool exportToLabview();

    // About and Version
    void aboutVersion();

  protected:


  private:

    // graphics initialisation
    void initGUI();               // initialise the mainwindow GUI
    void initMenuBar();           // initialise the GUI menubar
    void initTopBar();            // initialise the GUI topbar, toolbar
    void initSideBar();           // initialise the GUI sidebar, toolbar
    void initDialogDock();        // initialise the bottom dialog dock
    void initSimVisualizeDock();  // initialise the side sim visualize dock
    void initLayerDock();         // initialise the side layer dock

    // prepare any extra actions not attched to an icon or meny
    void initActions();

    // prepare the initial GUI state
    void initState();

    // application settings
    void loadSettings();  // load mainwindow settings from the settings instance
    void saveSettings();  // save mainwindow settings to the settings instance

    // VARIABLES

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
    gui::DesignPanel  *design_pan;  // mainwindow design panel
    gui::InfoPanel    *info_pan;    // mainwindow info panel
    gui::InputField   *input_field; // mainwindow input field
    gui::LayerEditor  *layer_editor;// pop-up layer editor
    gui::SimManager   *sim_manager; // pop-up simulator manager
    gui::SimVisualize *sim_visualize; // simulation visualizer that goes in sim visualize dock
    settings::SettingsDialog *settings_dialog;  // dialog for changing settings

    // dockable widgets
    QDockWidget *dialog_dock; // bottom panel for terminal dialog
    QDockWidget *sim_visualize_dock; // right side panel for sim visualization
    QDockWidget *layer_dock;  // right side panel for showing layers

    // action pointers
    QAction *action_screenshot_tool;
    QAction *action_select_tool;  // change cursor tool to select
    QAction *action_drag_tool;    // change cursor tool to drag
    QAction *action_dbgen_tool;   // change cursor tool to gen
    QAction *action_electrode_tool;   // change cursor tool to electrode
    QAction *action_afmarea_tool; // change cursor tool to AFM Area tool
    QAction *action_afmpath_tool; // change cursor tool to AFM Path tool
    QAction *action_label_tool;   // change cursor tool to label
    QAction *action_run_sim;      // run the current simulation method
    QAction *action_sim_visualize;// show the sim visualize dock which allows simulation visualization
    QAction *action_layer_sel;
    QAction *action_circuit_lib;
    QAction *action_dialog_dock_visibility;

    // save file
    QTimer autosave_timer;     // timer for autosaves
    QDir autosave_root;        // the root of autosave directories
    QDir autosave_dir;         // directory of autosave

    int autosave_ind=0;        // current autosave file index
    int autosave_num;          // number of autosaves to keep

    QString working_path;         // path currently in use

  };

} // end gui namespace

#endif
