// @file:     application.cpp
// @author:   Jake
// @created:  2016.10.31
// @editted:  2017.05.01  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Definitions of the Application functions



// ---------------------------------------------------------------------
// Qt inclusions
#include <QApplication>
#include <QSettings>
#include <QString>
#include <QTextStream>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QPushButton>

// ---------------------------------------------------------------------
// GUI inclusions
#include "application.h"
#include "src/settings/settings.h"
#include "widgets/primitives/dbdot.h"


#include <iostream>


// initialise dialog panel to NULL until built in constructor
gui::DialogPanel *gui::ApplicationGUI::dialog_wg = 0;


gui::ApplicationGUI::ApplicationGUI(QWidget *parent)
  : QMainWindow(parent)
{
  // initialise UI
  initGui();

  // load settings
  loadSettings();

  // setup problem and solver parameters
  //problem = new  core::Problem();
}

gui::ApplicationGUI::~ApplicationGUI()
{
  // save current settings
  saveSettings();

  // delete tool bars... may not be needed
  delete top_bar;
  delete side_bar;
  //delete problem;
}

void gui::ApplicationGUI::initGui()
{
  // menubar
  initMenuBar();

  // top toolbar
  initTopBar();

  // sidepanel
  initSideBar();

  // design widget
  design_wg = new gui::DesignWidget(this);

  // dialog panel
  dialog_wg = new gui::DialogPanel(this);

  // info panel
  info_wg = new gui::InfoPanel(this);

  // layout management
  QWidget *main_widget = new QWidget(this);
  QVBoxLayout *vbl = new QVBoxLayout();
  QHBoxLayout *hbl = new QHBoxLayout();

  hbl->addWidget(dialog_wg, 1);
  hbl->addWidget(info_wg, 1);

  vbl->addWidget(design_wg, 2);
  vbl->addLayout(hbl, 1);

  main_widget->setLayout(vbl);
  setCentralWidget(main_widget);
}

void gui::ApplicationGUI::initMenuBar()
{
  // initialise menus

  QMenu *file = menuBar()->addMenu("&File");
  QMenu *edit = menuBar()->addMenu("&Edit");
  QMenu *view = menuBar()->addMenu("&View");
  QMenu *tools = menuBar()->addMenu("&Tools");

  // file menu actions
  QAction *quit = new QAction("&Quit", this);
  quit->setShortcut(tr("CTRL+Q"));
  file->addAction(quit);

  QAction *change_lattice = new QAction("&Change Lattice...", this);
  tools->addAction(change_lattice);

  connect(quit, &QAction::triggered, qApp, QApplication::quit);
  connect(change_lattice, &QAction::triggered, this, &gui::ApplicationGUI::changeLattice);
}

void gui::ApplicationGUI::initTopBar()
{
  settings::GUISettings gui_settings;

  top_bar = new QToolBar(tr("Top Bar"));

  // location behaviour
  top_bar->setFloatable(false);
  top_bar->setMovable(false);

  // size policy
  qreal ico_scale = gui_settings.get<qreal>("SBAR/mw");
  ico_scale *= gui_settings.get<qreal>("SBAR/ico");

  top_bar->setMinimumHeight(gui_settings.get<int>("TBAR/mh"));
  top_bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  top_bar->setIconSize(QSize(ico_scale, ico_scale));

  action_run_simulation = top_bar->addAction(QIcon(":/ico/runsim.svg"), tr("&Run Simulation..."));

  connect(action_run_simulation, &QAction::triggered, this, &gui::ApplicationGUI::runSimulation);
  addToolBar(Qt::TopToolBarArea, top_bar);
}


void gui::ApplicationGUI::initSideBar()
{
  settings::GUISettings gui_settings;

  // recall or initialise side bar location
  Qt::ToolBarArea area;
  if(gui_settings.contains("SBAR/loc"))
    area = static_cast<Qt::ToolBarArea>(gui_settings.get<int>("SBAR/loc"));
  else
    area = Qt::LeftToolBarArea;

  side_bar = new QToolBar(tr("Side Bar"));

  // location behaviour
  side_bar->setAllowedAreas(Qt::LeftToolBarArea|Qt::RightToolBarArea);
  side_bar->setFloatable(false);

  // size policy
  qreal ico_scale = gui_settings.get<qreal>("SBAR/mw");
  ico_scale *= gui_settings.get<qreal>("SBAR/ico");

  side_bar->setMinimumWidth(gui_settings.get<int>("SBAR/mw"));
  side_bar->setIconSize(QSize(ico_scale, ico_scale));

  // actions
  action_select_tool = side_bar->addAction(QIcon(":/ico/select.svg"), tr("&Select tool"));
  action_drag_tool = side_bar->addAction(QIcon(":/ico/drag.svg"), tr("&Drag tool"));
  action_dbgen_tool = side_bar->addAction(QIcon(":/ico/dbgen.svg"), tr("&DB tool"));

  connect(action_select_tool, &QAction::triggered, this, &gui::ApplicationGUI::setToolSelect);
  connect(action_drag_tool, &QAction::triggered, this, &gui::ApplicationGUI::setToolDrag);
  connect(action_dbgen_tool, &QAction::triggered, this, &gui::ApplicationGUI::setToolDBGen);

  addToolBar(area, side_bar);
}



void gui::ApplicationGUI::loadSettings()
{
  qDebug("Loading settings");
  settings::GUISettings gui_settings;

  resize(gui_settings.get<QSize>("MWIN/size"));
}

void gui::ApplicationGUI::saveSettings()
{
  qDebug("Saving settings");
  settings::GUISettings gui_settings;

  gui_settings.setValue("SBAR/loc", (int)toolBarArea(side_bar));
}




// SLOTS

void gui::ApplicationGUI::setToolSelect()
{
  qDebug("selecting select tool");
  design_wg->setTool(gui::DesignWidget::SelectTool);
}

void gui::ApplicationGUI::setToolDrag()
{
  qDebug("selecting drag tool");
  design_wg->setTool(gui::DesignWidget::DragTool);
}

void gui::ApplicationGUI::setToolDBGen()
{
  qDebug("selecting dbgen tool");
  design_wg->setTool(gui::DesignWidget::DBGenTool);
}

void gui::ApplicationGUI::changeLattice()
{
  settings::AppSettings app_settings;

  QString dir = app_settings.get<QString>("dir/lattice");
  QString fname = QFileDialog::getOpenFileName(
    this, tr("Select lattice file"), dir, tr("INI (*.ini)"));

  design_wg->buildLattice(fname);
}

void gui::ApplicationGUI::runSimulation()
{

  // if(problem==0)
  //   qFatal("Problem instance not initialised");

  QList<prim::DBDot*> dbs = design_wg->getSurfaceDBs();
  if(dbs.count()==0){
    qWarning("No dangling bonds on the surface...");
    return;
  }

  //problem->setSurface(dbs);

  // engines::SimulatedAnnealer SA;
  //
  // SA.loadProblem(problem);
  // SA.solve();
  //
  // float *occs = new float[problem->N];
  // SA.getOccs(occs);

  // set dot fill factors
}
