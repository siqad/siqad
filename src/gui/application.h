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

// Widget includes
#include "widgets/design_panel.h"
#include "widgets/dialog_panel.h"
#include "widgets/input_field.h"
#include "widgets/info_panel.h"


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

    // constructor
    explicit ApplicationGUI(QWidget *parent=0);

    // destructor
    ~ApplicationGUI();

    // static declaration of DialogPanel for dialogstream
    static gui::DialogPanel *dialog_pan;

  public slots:

    // cursor tool updating
    void setTool(gui::DesignPanel::ToolType tool);
    void setToolSelect();
    void setToolDrag();
    void setToolDBGen();

    // change lattice
    void changeLattice();

    // parse input field and act accordingly
    void parseInputField();

    // Start current simulation method
    // ... it might be worth modifyin the work-flow such that instead of running
    // the simulation method we push the problem onto a working stack to be run
    // in the background: will need to be able to display results on request
    // after job finished.
    void runSimulation();

    // SANDBOX

    void selectColor(); // tool for converting colors into QVariant strings

    void screenshot();  // take an svg capture of the GUI
    void designScreenshot();         // take an svg capture ofthe design window

    // SAVE/LOAD
    void saveDefault(){saveToFile(0);}
    void saveNew(){saveToFile(1);}
    void saveToFile(bool new_file=0);
    void openFromFile();

  protected:


  private:

    // graphics initialisation
    void initGUI();       // initialise the mainwindow GUI
    void initMenuBar();   // initialise the GUI menubar
    void initTopBar();    // initialise the GUI topbar, toolbar
    void initSideBar();   // initialise the GUI sidebar, toolbar

    // prepare any extra actions not attched to an icon or meny
    void initActions();
    
    // prepare the initial GUI state
    void initState();

    // application settings
    void loadSettings();  // load mainwindow settings from the settings instance
    void saveSettings();  // save mainwindow settings to the settings instance

    // VARIABLES

    // directory path persistence
    QDir img_dir;

    // purely graphics widgets
    QToolBar *top_bar;
    QToolBar *side_bar;

    // functional widgets, DialogPanel is a static in public above.
    gui::DesignPanel  *design_pan;  // mainwindow design panel
    gui::InfoPanel    *info_pan;    // mainwindow info panel
    gui::InputField   *input_field; // mainwindow input field

    // action pointers
    QAction *action_select_tool;  // change cursor tool to select
    QAction *action_drag_tool;    // change cursor tool to drag
    QAction *action_dbgen_tool;   // change cursor tool to gen
    QAction *action_run_sim;      // run the current simulation method

    // save file
    QFile file;                   // file that saveToFile writes to
    QString working_path;         // path currently in use
  };

} // end gui namespace

#endif
