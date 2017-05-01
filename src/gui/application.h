// @file:     application.h
// @author:   Jake
// @created:  2016.10.31
// @editted:  2017.05.01  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Customized QMainWindow class, ApplicationGUI, for the GUI.

#ifndef _UI_APPLICATION_H_
#define _UI_APPLICATION_H_

// Qt inclusions
#include <QMainWindow>
#include <QObject>
#include <QFileDialog>

// Widget inclusions
#include "widgets/design_widget.h"
#include "widgets/dialog_panel.h"
#include "widgets/info_panel.h"

// solver engines
//#include "src/engines/core/problem.h"
//#include "src/engines/simulated_annealer.h"

namespace Ui
{
  class ApplicationGUI;
} // end namespace Ui

namespace gui{

// Main application window
class ApplicationGUI : public QMainWindow
{
  Q_OBJECT

public:

  // constructor
  explicit ApplicationGUI(QWidget *parent = 0);

  // deconstructor
  ~ApplicationGUI();

  // static declaration of DialogPanel for dialogstream
  static gui::DialogPanel *dialog_wg;

public slots:

  // Update the current tool functinality of the cursor
  void setToolSelect();
  void setToolDrag();
  void setToolDBGen();

  // Change the active lattice
  void changeLattice();

  // Start current simulation method
  // ... it might be worth modifyin the work-flow such that instead of running
  // the simulation method we push the problem onto a working stack to be run
  // in the background: will need to be able to display results on request
  // after job finished.
  void runSimulation();

protected:

private:

  void initGui();
  void initMenuBar();
  void initTopBar();
  void initSideBar();

  void loadSettings();
  void saveSettings();

  QString global_settings_fname_;
  QToolBar *top_bar;
  QToolBar *side_bar;

  // functional widgets
  gui::DesignWidget *design_wg;
  gui::InfoPanel *info_wg;

  // action pointers
  QAction *action_select_tool;
  QAction *action_drag_tool;
  QAction *action_dbgen_tool;
  QAction *action_run_simulation;


  // problem and solver parameters
  //core::Problem *problem;
};

} // end namespace gui



#endif
