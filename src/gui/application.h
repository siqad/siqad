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

  void setToolSelect();
  void setToolDrag();
  void setToolDBGen();

  void changeLattice();
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
