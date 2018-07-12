// @file:     application.cc
// @author:   Jake
// @created:  2016.10.31
// @editted:  2017.05.09  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Definitions of the Application functions



// std includes
#include <algorithm>

// Qt includes
#include <QtSvg>
#include <iostream>
#include <QMessageBox>

// gui includes
#include "application.h"
#include "src/settings/settings.h"


// init the DialogPanel to NULL until build in constructor
gui::DialogPanel *gui::ApplicationGUI::dialog_pan = 0;


// constructor
gui::ApplicationGUI::ApplicationGUI(QWidget *parent)
  : QMainWindow(parent)
{
  // save start time for instance recognition
  start_time = QDateTime::currentDateTime();

  // initialize default save_dir
  save_dir = QDir::homePath();

  // initialise GUI
  initGUI();

  // load settings, resize mainwindow
  loadSettings();
}

// destructor
gui::ApplicationGUI::~ApplicationGUI()
{
  // save current settings
  saveSettings();

  // delete dialog panel manually and set pointer to 0. This avoids segfaults from
  // attempts to echo in dialog panel
  delete dialog_pan;
  dialog_pan = 0;

  // disown the layer dock widget so it can be properly destructed by design panel
  layer_dock->setParent(0);

  // free memory, parent delete child Widgets so Graphical Items are already
  // handled. Still need to free Settings and commander
  delete commander;
  delete settings::AppSettings::instance();
  delete settings::GUISettings::instance();
  delete settings::LatticeSettings::instance();
  delete prim::Emitter::instance();
}



// GUI INITIALISATION

void gui::ApplicationGUI::initGUI()
{
  // initialise mainwindow panels
  dialog_pan = new gui::DialogPanel(this); // init first to capture std output
  design_pan = new gui::DesignPanel(this);
  input_field = new gui::InputField(this);
  info_pan = new gui::InfoPanel(this);

  // detachable/pop-up widgets, order matters in some cases due to pointers
  sim_manager = new gui::SimManager(this);
  sim_visualize = new gui::SimVisualize(sim_manager, this);

  // individual pop-up widgets
  settings_dialog = new settings::SettingsDialog(this);

  // initialise docks
  initSimVisualizeDock();
  initDialogDock();
  initLayerDock();
  initItemDock();
  tabifyDockWidget(item_dock, layer_dock);

  // initialise bars
  initMenuBar(); // must run before initTopBar
  initTopBar();
  initSideBar();

  // initialise parser
  initCommander();

  // inter-widget signals
  connect(sim_visualize, &gui::SimVisualize::showElecDistOnScene,
          design_pan, &gui::DesignPanel::displaySimResults);
  connect(sim_visualize, &gui::SimVisualize::showPotPlotOnScene,
          design_pan, &gui::DesignPanel::displayPotentialPlot);
  connect(sim_visualize, &gui::SimVisualize::clearPotPlots,
          design_pan, &gui::DesignPanel::clearPlots);
  connect(design_pan, &gui::DesignPanel::sig_quickRunSimulation,
          sim_manager, &gui::SimManager::quickRun);

  // widget-app gui signals
  connect(sim_manager, &gui::SimManager::emitSimJob,
          this, &gui::ApplicationGUI::runSimulation);
  connect(design_pan, &gui::DesignPanel::sig_resetDesignPanel,
          this, &gui::ApplicationGUI::designPanelReset);
  connect(design_pan, &gui::DesignPanel::sig_setLayerManagerWidget,
          this, &gui::ApplicationGUI::setLayerManagerWidget);
  connect(design_pan, &gui::DesignPanel::sig_undoStackCleanChanged,
          this, &gui::ApplicationGUI::updateWindowTitle);
  connect(design_pan, &gui::DesignPanel::sig_screenshot,
          this, QOverload<QRect>::of(&gui::ApplicationGUI::designScreenshot));
  connect(design_pan, &gui::DesignPanel::sig_showSimulationSetup,
          this, &gui::ApplicationGUI::simulationSetup);
  connect(design_pan, &gui::DesignPanel::sig_cancelScreenshot,
          this, &gui::ApplicationGUI::endScreenshotMode);

  // layout management
  QWidget *main_widget = new QWidget(this); // main widget for mainwindow
  QVBoxLayout *vbl = new QVBoxLayout();     // main layout, vertical

  // NOTE commented out info_pan for now so it doesn't create empty space
  //QHBoxLayout *hbl = new QHBoxLayout();     // lower layout, horizontal

  //hbl->addLayout(vbl_l, 1);
  //hbl->addWidget(info_pan, 1);

  //info_pan->hide();

  vbl->addWidget(design_pan, 2);
  //vbl->addLayout(hbl, 1);

  // set mainwindow layout
  main_widget->setLayout(vbl);
  setCentralWidget(main_widget);

  // additional actions
  initActions();

  // prepare initial GUI state
  initState();
}


void gui::ApplicationGUI::initMenuBar()
{
  // initialise menus
  QMenu *file = menuBar()->addMenu(tr("&File"));
  // QMenu *edit = menuBar()->addMenu(tr("&Edit"));
  QMenu *view = menuBar()->addMenu(tr("&View"));
  menuBar()->addMenu(tr("&Edit"));
  QMenu *tools = menuBar()->addMenu(tr("&Tools"));
  QMenu *help = menuBar()->addMenu(tr("&Help"));

  // file menu actions
  QAction *new_file = new QAction(QIcon::fromTheme("document-new"), tr("&New"), this);
  QAction *open_save = new QAction(QIcon::fromTheme("document-open"), tr("&Open..."), this);
  QAction *save = new QAction(QIcon::fromTheme("document-save"), tr("&Save"), this);
  QAction *save_as = new QAction(QIcon::fromTheme("document-save-as"), tr("Save &As..."), this);
  QAction *export_lvm = new QAction(tr("&Export to QSi LV"), this);
  QAction *quit = new QAction(QIcon::fromTheme("application-exit"), tr("&Quit"), this);
  new_file->setShortcut(tr("CTRL+N"));
  save->setShortcut(tr("CTRL+S"));
  save_as->setShortcut(tr("CTRL+SHIFT+S"));
  open_save->setShortcut(tr("CTRL+O"));
  //export_lvm->setShortcut(tr("CTRL+E"));
  quit->setShortcut(tr("CTRL+Q"));
  file->addAction(new_file);
  file->addAction(open_save);
  file->addAction(save);
  file->addAction(save_as);
  //file->addAction(export_lvm);
  file->addAction(quit);

  // view menu actions
  action_sim_visualize = sim_visualize_dock->toggleViewAction();
  action_layer_sel = layer_dock->toggleViewAction();
  action_dialog_dock_visibility = dialog_dock->toggleViewAction();
  action_sim_visualize->setIcon(QIcon(":/ico/simvisual.svg"));
  action_layer_sel->setIcon(QIcon(":/ico/layer.svg"));
  action_dialog_dock_visibility->setIcon(QIcon(":/ico/term.svg"));
  QAction *rotate_view_cw = new QAction(QIcon::fromTheme("object-rotate-right"), tr("Rotate 90 deg CW"), this);
  QAction *rotate_view_ccw = new QAction(QIcon::fromTheme("object-rotate-left"), tr("Rotate 90 deg CCW"), this);
  view->addAction(action_sim_visualize);
  view->addAction(action_layer_sel);
  view->addAction(action_dialog_dock_visibility);
  view->addSeparator();
  view->addAction(rotate_view_cw);
  view->addAction(rotate_view_ccw);

  // tools menu actions
  QAction *change_lattice = new QAction(tr("Change Lattice..."), this);
  QAction *select_color = new QAction(tr("Select Color..."), this);
  QAction *area_screenshot = new QAction(tr("Region Screenshot..."), this);
  QAction *design_screenshot = new QAction(tr("Design Screenshot..."), this);
  QAction *screenshot = new QAction(tr("Window Screenshot..."), this);
  QAction *action_settings_dialog = new QAction(tr("Settings"), this);

  tools->addAction(change_lattice);
  tools->addAction(select_color);
  tools->addSeparator();
  tools->addAction(area_screenshot);
  tools->addAction(design_screenshot);
  tools->addAction(screenshot);
  tools->addSeparator();
  tools->addAction(action_settings_dialog);

  // help menu actions
  QAction *about_version = new QAction(tr("About"), this);

  help->addAction(about_version);

  connect(new_file, &QAction::triggered, this, &gui::ApplicationGUI::newFile);
  connect(quit, &QAction::triggered, this, &gui::ApplicationGUI::closeFile);
  connect(save, &QAction::triggered, this, &gui::ApplicationGUI::saveDefault);
  connect(save_as, &QAction::triggered, this, &gui::ApplicationGUI::saveNew);
  connect(open_save, &QAction::triggered, this, &gui::ApplicationGUI::openFromFile);
  connect(export_lvm, &QAction::triggered, this, &gui::ApplicationGUI::exportToLabview);
  connect(rotate_view_cw, &QAction::triggered, design_pan, &gui::DesignPanel::rotateCw);
  connect(rotate_view_ccw, &QAction::triggered, design_pan, &gui::DesignPanel::rotateCcw);
  connect(change_lattice, &QAction::triggered, this, &gui::ApplicationGUI::changeLattice);
  connect(select_color, &QAction::triggered, this, &gui::ApplicationGUI::selectColor);
  connect(area_screenshot, &QAction::triggered, this, &gui::ApplicationGUI::beginScreenshotMode);
  connect(screenshot, &QAction::triggered, this, &gui::ApplicationGUI::screenshot);
  connect(design_screenshot, SIGNAL(triggered()),
          this, SLOT(designScreenshot()));
  connect(action_settings_dialog, &QAction::triggered, this, &gui::ApplicationGUI::showSettingsDialog);
  connect(about_version, &QAction::triggered, this, &gui::ApplicationGUI::aboutVersion);

}


void gui::ApplicationGUI::initTopBar()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  top_bar = new QToolBar(tr("Top Bar"));

  // location behaviour
  top_bar->setFloatable(false);
  top_bar->setMovable(false);

  // size policy
  qreal ico_scale = gui_settings->get<qreal>("SBAR/mw");
  ico_scale *= gui_settings->get<qreal>("SBAR/ico");

  top_bar->setMinimumHeight(gui_settings->get<int>("TBAR/mh"));
  top_bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  top_bar->setIconSize(QSize(ico_scale, ico_scale));

  // initialize QActions

  // run simulation
  action_run_sim = new QAction(QIcon(":/ico/runsim.svg"), tr("Run Simulation..."));
  action_run_sim->setShortcut(tr("CTRL+R"));
  connect(action_run_sim, &QAction::triggered,
          this, &gui::ApplicationGUI::simulationSetup);

  action_repeat_sim = new QAction(tr("Repeat Previous Simulation"), this);
  action_repeat_sim->setShortcut(tr("CTRL+SHIFT+R"));
  connect(action_repeat_sim, &QAction::triggered,
	  this, &gui::ApplicationGUI::repeatSimulation);
  addAction(action_repeat_sim);

  // ground state simulation
  action_run_ground_state = new QAction(QIcon(":/ico/sim_groundstate.svg"), tr("Run ground state simulation..."));
  // action_run_ground_state->setShortcut(tr("F11"));
  connect(action_run_ground_state, &QAction::triggered,
          this, &gui::ApplicationGUI::runGroundState);

  // add them to top bar
  top_bar->addAction(action_run_sim);
  top_bar->addAction(action_run_ground_state);
  top_bar->addAction(action_sim_visualize);           // already initialised in menu bar
  top_bar->addAction(action_layer_sel);               // already initialised in menu bar
  top_bar->addAction(action_dialog_dock_visibility);  // already initialised in menu bar

  //action_circuit_lib= top_bar->addAction(QIcon(":/ico/circuitlib.svg"), tr("Circuit Library"));

  addToolBar(Qt::TopToolBarArea, top_bar);
}


void gui::ApplicationGUI::initSideBar()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  // recall or initialise side bar location
  Qt::ToolBarArea area;
  if(gui_settings->contains("SBAR/loc"))
    area = static_cast<Qt::ToolBarArea>(gui_settings->get<int>("SBAR/loc"));
  else
    area = Qt::LeftToolBarArea;

  side_bar = new QToolBar(tr("Side Bar"));

  // location behaviour
  side_bar->setAllowedAreas(Qt::LeftToolBarArea|Qt::RightToolBarArea);
  side_bar->setFloatable(false);

  // size policy
  qreal ico_scale = gui_settings->get<qreal>("SBAR/mw");
  ico_scale *= gui_settings->get<qreal>("SBAR/ico");

  side_bar->setMinimumWidth(gui_settings->get<int>("SBAR/mw"));
  side_bar->setIconSize(QSize(ico_scale, ico_scale));

  // actions
  QActionGroup *action_group = new QActionGroup(side_bar);

  action_screenshot_tool = side_bar->addAction(QIcon(":/ico/screenshotarea.svg"),
      tr("Screenshot Area tool"));
  action_select_tool = side_bar->addAction(QIcon(":/ico/select.svg"),
      tr("Select tool"));
  action_drag_tool = side_bar->addAction(QIcon(":/ico/drag.svg"),
      tr("Drag tool"));
  action_dbgen_tool = side_bar->addAction(QIcon(":/ico/dbgen.svg"),
      tr("DB tool"));
  action_electrode_tool = side_bar->addAction(QIcon(":/ico/drawelectrode.svg"),
      tr("Electrode tool"));
  /*
  action_afmarea_tool = side_bar->addAction(QIcon(":/ico/drawafmarea.svg"),
      tr("AFM Area tool"));
  action_afmpath_tool = side_bar->addAction(QIcon(":/ico/drawafmpath.svg"),
      tr("AFM Path tool"));
  action_label_tool = side_bar->addAction(QIcon(":/ico/drawlabel.svg"),
      tr("Label tool"));
      */

  action_group->addAction(action_screenshot_tool);
  action_group->addAction(action_select_tool);
  action_group->addAction(action_drag_tool);
  action_group->addAction(action_dbgen_tool);
  action_group->addAction(action_electrode_tool);
  /*
  action_group->addAction(action_afmarea_tool);
  action_group->addAction(action_afmpath_tool);
  action_group->addAction(action_label_tool);
  */

  action_screenshot_tool->setVisible(false);  // only shown in ScreenshotMode

  action_screenshot_tool->setCheckable(true);
  action_select_tool->setCheckable(true);
  action_drag_tool->setCheckable(true);
  action_dbgen_tool->setCheckable(true);
  action_electrode_tool->setCheckable(true);
  /*
  action_afmarea_tool->setCheckable(true);
  action_afmpath_tool->setCheckable(true);
  action_label_tool->setCheckable(true);
  */

  action_select_tool->setChecked(true);

  connect(action_screenshot_tool, &QAction::triggered,
          this, &gui::ApplicationGUI::setToolScreenshotArea);
  connect(action_select_tool, &QAction::triggered,
          this, &gui::ApplicationGUI::setToolSelect);
  connect(action_drag_tool, &QAction::triggered,
          this, &gui::ApplicationGUI::setToolDrag);
  connect(action_dbgen_tool, &QAction::triggered,
          this, &gui::ApplicationGUI::setToolDBGen);
  connect(action_electrode_tool, &QAction::triggered,
          this, &gui::ApplicationGUI::setToolElectrode);
  /*
  connect(action_afmarea_tool, &QAction::triggered,
          this, &gui::ApplicationGUI::setToolAFMArea);
  connect(action_afmpath_tool, &QAction::triggered,
          this, &gui::ApplicationGUI::setToolAFMPath);
  connect(action_label_tool, &QAction::triggered,
          this, &gui::ApplicationGUI::setToolLabel);
          */

  addToolBar(area, side_bar);
}

void gui::ApplicationGUI::initDialogDock()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  // add dialog_pan and input_field into a single widget
  QVBoxLayout *dialog_dock_vl = new QVBoxLayout();
  dialog_dock_vl->addWidget(dialog_pan, 1);
  dialog_dock_vl->addWidget(input_field, 0);

  QWidget *dialog_dock_main = new QWidget();
  dialog_dock_main->setLayout(dialog_dock_vl);

  // recall or initialise dialog dock location
  Qt::DockWidgetArea area;
  area = static_cast<Qt::DockWidgetArea>(gui_settings->get<int>("DDOCK/loc"));

  dialog_dock = new QDockWidget(tr("Terminal Dialog"));

  dialog_dock->setAllowedAreas(Qt::BottomDockWidgetArea);  // location behaviour
  dialog_dock->setMinimumHeight(gui_settings->get<int>("DDOCK/mh")); // size TODO add to settings

  dialog_dock->setWidget(dialog_dock_main);
  dialog_dock->setVisible(settings::AppSettings::instance()->get<bool>("log/override"));
  addDockWidget(area, dialog_dock);
}

void gui::ApplicationGUI::initSimVisualizeDock()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  // recall or initialize sim visualize dock location
  Qt::DockWidgetArea area;
  area = static_cast<Qt::DockWidgetArea>(gui_settings->get<int>("SIMVDOCK/loc"));

  sim_visualize_dock = new QDockWidget(tr("Sim Visualize"));

  // location behaviour
  sim_visualize_dock->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea|Qt::BottomDockWidgetArea);

  // size policy
  sim_visualize_dock->setMinimumWidth(gui_settings->get<int>("SIMVDOCK/mw"));

  connect(sim_visualize_dock, &QDockWidget::visibilityChanged, design_pan, &gui::DesignPanel::simVisualizeDockVisibilityChanged);

  sim_visualize_dock->setWidget(sim_visualize);
  sim_visualize_dock->hide();
  addDockWidget(area, sim_visualize_dock);

  // set shortcuts
  sim_visualize_dock->toggleViewAction()->setShortcut(Qt::Key_V);
}


void gui::ApplicationGUI::initLayerDock()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  layer_dock = new QDockWidget("Layer Manager");

  Qt::DockWidgetArea area;
  area = static_cast<Qt::DockWidgetArea>(gui_settings->get<int>("LAYDOCK/loc"));

  layer_dock->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea|Qt::BottomDockWidgetArea);

  layer_dock->setMinimumWidth(gui_settings->get<int>("LAYDOCK/mw"));

  layer_dock->setWidget(design_pan->layerManagerSideWidget());
  layer_dock->show();
  addDockWidget(area, layer_dock);
}


void gui::ApplicationGUI::initItemDock()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  item_dock = new QDockWidget("Item Manager");

  Qt::DockWidgetArea area;
  area = static_cast<Qt::DockWidgetArea>(gui_settings->get<int>("ITEMDOCK/loc"));

  item_dock->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea|Qt::BottomDockWidgetArea);

  item_dock->setMinimumWidth(gui_settings->get<int>("ITEMDOCK/mw"));

  item_dock->setWidget(design_pan->itemManagerWidget());
  item_dock->show();
  addDockWidget(area, item_dock);
}


void gui::ApplicationGUI::initCommander()
{
  commander = new Commander();
  commander->setDesignPanel(design_pan);
  commander->setDialogPanel(dialog_pan);
  commander->addKeyword(tr("add_item"));
  commander->addKeyword(tr("remove_item"));
  commander->addKeyword(tr("echo"));
  commander->addKeyword(tr("help"));
  commander->addKeyword(tr("run"));
  commander->addKeyword(tr("move_item"));
}

void gui::ApplicationGUI::setLayerManagerWidget(QWidget *widget)
{
  qDebug() << "Making layer manager widget";
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  // recall or initialise layer dock location
  Qt::DockWidgetArea area;
  area = static_cast<Qt::DockWidgetArea>(gui_settings->get<int>("LAYDOCK/loc"));

  layer_dock->setWidget(widget);
}


void gui::ApplicationGUI::initActions()
{
  // input field
  connect(input_field, &gui::InputField::returnPressed,
            this, &gui::ApplicationGUI::parseInputField);

  // set tool
  connect(design_pan, &gui::DesignPanel::sig_toolChangeRequest,
            this, &gui::ApplicationGUI::setTool);
}


void gui::ApplicationGUI::initState()
{
  settings::AppSettings *app_settings = settings::AppSettings::instance();

  setTool(gui::ToolType::SelectTool);
  updateWindowTitle();
  autosave_timer.start(1000*app_settings->get<int>("save/autosaveinterval"));
}



// SETTINGS

void gui::ApplicationGUI::loadSettings()
{
  qDebug() << tr("Loading settings");
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  resize(gui_settings->get<QSize>("MWIN/size"));

  // autosave related settings
  settings::AppSettings *app_settings = settings::AppSettings::instance();
  autosave_num = app_settings->get<int>("save/autosavenum");
  autosave_root.setPath(app_settings->getPath("save/autosaveroot"));

  // autosave directory for current instance
  qint64 tag = QCoreApplication::applicationPid();
  //QString tag = start_time.toString("yyMMdd-HHmmss")
  autosave_dir.setPath(autosave_root.filePath(tr("instance-%1").arg(tag)));
  autosave_dir.setNameFilters(QStringList() << "*.*");
  autosave_dir.setFilter(QDir::Files);

  // create the autosave directory
  if(!autosave_dir.exists()){
    if(autosave_dir.mkpath("."))
      qDebug() << tr("Successfully created autosave directory");
    else
      qCritical() << tr("Failed to create autosave direcrory");
  }

  // auto save signal
  connect(&autosave_timer, &QTimer::timeout, this, &gui::ApplicationGUI::autoSave);

  // reset state
  initState();
}


void gui::ApplicationGUI::saveSettings()
{
  qDebug() << tr("Saving settings");
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  gui_settings->setValue("SBAR/loc", (int) toolBarArea(side_bar));

  // remove autosave during peaceful termination
  for(const QString &dirFile : autosave_dir.entryList())
    autosave_dir.remove(dirFile); // remove autosave files

  qDebug() << tr("Test: %1").arg(autosave_dir.absolutePath());

  if(autosave_dir.removeRecursively())
    qDebug() << tr("Removed autosave directory: %1").arg(autosave_dir.path());
  else
    qDebug() << tr("Failed to remove autosave directory: %1").arg(autosave_dir.path());
}




// PUBLIC SLOTS

void gui::ApplicationGUI::updateWindowTitle()
{
  QString title_name;

  // prefix the title by an asterisk to the name if the file has been edited
  if (design_pan->stateChanged())
    title_name += "*";

  QFileInfo w_path_info(working_path);
  title_name += (working_path.isEmpty()) ? "Untitled" : w_path_info.fileName();

  setWindowTitle(tr("%1 - %2")
    .arg(title_name)
    .arg(QCoreApplication::applicationName())
  );
}

void gui::ApplicationGUI::setTool(gui::ToolType tool)
{
  switch(tool){
    case gui::ToolType::SelectTool:
      action_select_tool->setChecked(true);
      setToolSelect();
      break;
    case gui::ToolType::DragTool:
      action_drag_tool->setChecked(true);
      setToolDrag();
      break;
    case gui::ToolType::DBGenTool:
      action_dbgen_tool->setChecked(true);
      setToolDBGen();
      break;
    case gui::ToolType::ElectrodeTool:
      action_electrode_tool->setChecked(true);
      setToolElectrode();
      break;
      /*
    case gui::ToolType::AFMAreaTool:
      action_afmarea_tool->setChecked(true);
      setToolAFMArea();
      break;
    case gui::ToolType::AFMPathTool:
      action_afmpath_tool->setChecked(true);
      setToolAFMPath();
      break;
      */
    case gui::ToolType::ScreenshotAreaTool:
      action_screenshot_tool->setChecked(true);
      setToolScreenshotArea();
      break;
      /*
    case gui::ToolType::LabelTool:
      action_label_tool->setChecked(true);
      setToolLabel();
      break;
      */
    default:
      break;
  }
}

void gui::ApplicationGUI::setToolSelect()
{
  qDebug() << tr("selecting select tool");
  design_pan->setTool(gui::ToolType::SelectTool);
}

void gui::ApplicationGUI::setToolDrag()
{
  qDebug() << tr("selecting drag tool");
  design_pan->setTool(gui::ToolType::DragTool);
}

void gui::ApplicationGUI::setToolDBGen()
{
  if(design_pan->displayMode() != gui::DisplayMode::DesignMode){
    qDebug() << tr("dbgen tool not allowed outside of design mode");
    return;
  }
  qDebug() << tr("selecting dbgen tool");
  design_pan->setTool(gui::ToolType::DBGenTool);
}

void gui::ApplicationGUI::setToolElectrode()
{
  qDebug() << tr("selecting electrode tool");
  design_pan->setTool(gui::ToolType::ElectrodeTool);
}

void gui::ApplicationGUI::setToolAFMArea()
{
  qDebug() << tr("selecting afmarea tool");
  design_pan->setTool(gui::ToolType::AFMAreaTool);
}

void gui::ApplicationGUI::setToolAFMPath()
{
  qDebug() << tr("selecting afmpath tool");
  design_pan->setTool(gui::ToolType::AFMPathTool);
}

void gui::ApplicationGUI::setToolScreenshotArea()
{
  qDebug() << tr("selecting screenshot area tool");
  design_pan->setTool(gui::ToolType::ScreenshotAreaTool);
}

void gui::ApplicationGUI::setToolLabel()
{
  qDebug() << tr("selecting label tool");
  design_pan->setTool(gui::ToolType::LabelTool);
}


void gui::ApplicationGUI::changeLattice()
{
  settings::AppSettings *app_settings = settings::AppSettings::instance();

  QString dir = app_settings->get<QString>("dir/lattice");
  QString fname = QFileDialog::getOpenFileName(
    this, tr("Select lattice file"), dir, tr("INI (*.ini)"));

  design_pan->buildLattice(fname);
}

void gui::ApplicationGUI::parseInputField()
{
  // get input from input_field, remove leading/trailing whitespace
  QString input = input_field->pop();
  // if input string is not empty, send it to commander to do the rest.
  if(!input.isEmpty()){
    commander->parseInputs(input);
  }
}

void gui::ApplicationGUI::designPanelReset()
{
  initState();
}

void gui::ApplicationGUI::simulationSetup()
{
  sim_manager->showSimSetupDialog();
}

void gui::ApplicationGUI::repeatSimulation()
{
  QMessageBox::StandardButton reply;
  reply = QMessageBox::question(this, "Quick run simulation",
				"Are you sure you want to run a simulation with previous settings?",
				QMessageBox::Yes | QMessageBox::No);
  if (reply == QMessageBox::Yes) {
    sim_manager->quickRun();
  }
}

void gui::ApplicationGUI::runGroundState()
{
  int index = sim_manager->getComboEngSel()->findText("SimAnneal");
  if (index != -1) { //if index found
    sim_manager->getComboEngSel()->setCurrentIndex(index);
    sim_manager->quickRun();
  } else {
    qDebug() << tr("Ground state engine not found.");
  }
}

void gui::ApplicationGUI::runSimulation(prim::SimJob *job)
{
  if(!job){
    qWarning() << tr("ApplicationGUI: Received job is not a valid pointer.");
    return;
  }

  setTool(gui::ToolType::SelectTool);

  qDebug() << tr("ApplicationGUI: About to run job '%1'").arg(job->name());

  // call saveToFile TODO don't forget to account for setup dialog settings
  saveToFile(Simulation, job->problemFile(), job);

  // call job binary and read output when done
  job->invokeBinary();
  job->readResults();

  // show side dock for user to look at sim result
  sim_visualize_dock->show();
  sim_visualize->updateJobSelCombo(); // TODO make sim_visualize capture job completion signals, so it updates the field on its own
}

bool gui::ApplicationGUI::readSimResult(const QString &result_path)
{
  QFile result_file(result_path);

  // check that output file exists and can be opened
  if(!result_file.open(QFile::ReadOnly | QFile::Text)){
    qDebug() << tr("Error when opening file to read: %1").arg(result_file.errorString());
    return false;
  }

  // read from XML stream
  QXmlStreamReader rs(&result_file); // result stream
  qDebug() << tr("Begin to read result from %1").arg(result_file.fileName());
  // TODO might just be better to pass this whole block to simulator class
  while(!rs.atEnd()){
    if(rs.isStartElement()){
      if(rs.name() == "eng_info"){
        rs.readNext();
        // basic engine info
        while(rs.name() != "eng_info"){
          // TODO read engine info
          // can't get rid of this because there's no guarantee that the result file is being
          // read by a machine that has the simulator installed.
        }
      }
      else if(rs.name() == "sim_param"){
        // TODO simulator class
      }
      else if(rs.name() == "physloc"){
        // TODO simulator class
      }
      else if(rs.name() == "elec_dist"){
        // TODO simulator class
      }
      else{
        qDebug() << tr("%1: invalid element encountered on line %2 - %3").arg(result_path).arg(rs.lineNumber()).arg(rs.name().toString());
        rs.readNext();
      }
    }
    else
      rs.readNext();
  }
  qDebug() << tr("Load complete");
  result_file.close();

  return true;
}





// SANDBOX

void gui::ApplicationGUI::selectColor()
{
  QColorDialog::getColor(Qt::white, this,
    tr("Select a color"), QColorDialog::ShowAlphaChannel);
}


void gui::ApplicationGUI::beginScreenshotMode()
{
  display_mode_cache = design_pan->displayMode();
  design_pan->setDisplayMode(ScreenshotMode);
  action_screenshot_tool->setVisible(true);
  setTool(ScreenshotAreaTool);
}

void gui::ApplicationGUI::endScreenshotMode()
{
  design_pan->setDisplayMode(display_mode_cache);
  action_screenshot_tool->setVisible(false);
  setTool(SelectTool);
}

void gui::ApplicationGUI::screenshot()
{
  // get save path
  QString fname = QFileDialog::getSaveFileName(this, tr("Save File"), img_dir.path(),
                      tr("SVG files (*.svg)"));

  if(fname.isEmpty())
    return;
  img_dir = QDir(fname);

  gui::ApplicationGUI *widget = this;
  QRect rect = widget->rect();
  initState();

  QSvgGenerator gen;
  gen.setFileName(fname);
  gen.setSize(rect.size());
  gen.setViewBox(rect);

  QPainter painter;
  painter.begin(&gen);
  widget->render(&painter);

  // render menus
  QRect r; QString s;
  for(QAction *act : menuBar()->actions()){
    r = menuBar()->actionGeometry(act);
    s = act->text();
    while(s.startsWith("&"))
      s.remove(0,1);
    painter.drawText(r, Qt::AlignCenter, s);
  }

  painter.end();
}


void gui::ApplicationGUI::designScreenshot()
{
  beginScreenshotMode();

  gui::DesignPanel *widget = this->design_pan;
  QRect rect = widget->rect();
  rect.setHeight(rect.height() - widget->horizontalScrollBar()->height());
  rect.setWidth(rect.width() - widget->verticalScrollBar()->width());
  // translate the rect from view coord to scene coord, there might be a simpler
  // solution but this works...
  rect.translate(design_pan->mapToScene(
                    design_pan->mapFromParent(rect.topLeft())).toPoint()
                - rect.topLeft());

  designScreenshot(rect);
}


void gui::ApplicationGUI::designScreenshot(QRect rect)
{
  qDebug() << tr("taking screenshot for rect (%1, %2) (%3, %4)")
      .arg(rect.left()).arg(rect.top()).arg(rect.right()).arg(rect.bottom());
  // get save path
  QString fname = QFileDialog::getSaveFileName(this, tr("Save File"), img_dir.path(),
                      tr("SVG files (*.svg)"));

  if(fname.isEmpty()) {
    endScreenshotMode();
    return;
  }
  img_dir = QDir(fname);

  QSvgGenerator gen;
  gen.setFileName(fname);
  //gen.setSize(rect.size());
  gen.setViewBox(rect);

  QPainter painter;
  painter.begin(&gen);
  design_pan->screenshot(&painter, rect);
  painter.end();

  endScreenshotMode();
}

// FILE HANDLING


// unsaved changes prompt, returns whether to proceed with operation or not
bool gui::ApplicationGUI::resolveUnsavedChanges()
{
  QMessageBox msg_box;
  msg_box.setText("The document contains unsaved changes.");
  msg_box.setInformativeText("Would you like to save?");
  msg_box.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
  msg_box.setDefaultButton(QMessageBox::Save);

  // proceed only if either 'Save' or 'Discard'
  int usr_sel = msg_box.exec();
  return (usr_sel==QMessageBox::Save) ? saveToFile() : usr_sel==QMessageBox::Discard;
}


// make new file
void gui::ApplicationGUI::newFile()
{
  // prompt user to resolve unsaved changes if program has been modified
  if(design_pan->stateChanged())
    if(!resolveUnsavedChanges())
      return;

  // reset widgets and reinitialize GUI state
  working_path.clear();
  design_pan->resetDesignPanel();
  initState();
}


// save/load:
bool gui::ApplicationGUI::saveToFile(gui::ApplicationGUI::SaveFlag flag, const QString &path, prim::SimJob *sim_job)
{
  QString write_path;

  QFileDialog save_dialog;
  // determine target file
  if (!path.isEmpty()) {
    write_path = path;
  } else if (working_path.isEmpty() || flag==SaveAs) {
    save_dialog.setDefaultSuffix("sqd");
    write_path = save_dialog.getSaveFileName(this, tr("Save File"),
                  save_dir.filePath("new-db-layout.sqd"), tr("SQD (*.sqd);;All files (*)"));
    if (write_path.isEmpty())
      return false;
  } else {
    write_path = working_path;
  }

  // add .xml extension if there isn't
  if (! (write_path.endsWith(".sqd", Qt::CaseInsensitive) ||
         write_path.endsWith(".qad", Qt::CaseInsensitive) ||
         write_path.endsWith(".xml", Qt::CaseInsensitive)) )
    write_path.append(".sqd");

  // set file name of [whatevername].writing while writing to prevent loss of previous save if this save fails
  QFile file(write_path+".writing");

  if(!file.open(QIODevice::WriteOnly)){
    qDebug() << tr("Save: Error when opening file to save: %1").arg(file.errorString());
    return false;
  }

  // WRITE TO XML
  QXmlStreamWriter stream(&file);
  qDebug() << tr("Save: Beginning write to %1").arg(file.fileName());
  stream.setAutoFormatting(true);
  stream.writeStartDocument();

  // call the save functions for each relevant class
  stream.writeStartElement("siqad");

  // save program flags
  stream.writeComment("Program Flags");
  stream.writeStartElement("program");

  QString file_purpose = flag==Simulation ? "simulation" : "save";
  stream.writeTextElement("file_purpose", file_purpose);
  stream.writeTextElement("version", "TBD");
  stream.writeTextElement("date", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));

  stream.writeEndElement();

  // save simulation parameters
  if(flag == Simulation && sim_job){
    stream.writeStartElement("sim_params");
    QList<QPair<QString, QString>> sim_params = sim_job->simParams();
    for(auto sim_param_pair : sim_params)
      stream.writeTextElement(sim_param_pair.first, sim_param_pair.second);

    stream.writeEndElement();
  }

  // save design panel content (including GUI flags, layers and their corresponding contents (electrode, dbs, etc.)
  design_pan->saveToFile(&stream, flag==Simulation);

  // close root element & close file
  stream.writeEndElement();
  file.close();

  // delete the existing file and rename the new one to it
  QFile::remove(write_path);
  file.rename(write_path);

  qDebug() << tr("Save: Write completed for %1").arg(file.fileName());

  // update working path if needed
  if(flag == Save || flag == SaveAs){
    save_dir.setPath(write_path);
    working_path = write_path;
    updateWindowTitle();
  }

  return true;
}


void gui::ApplicationGUI::saveDefault()
{
  // default manual save without forcing file chooser
  if(saveToFile(Save))
    design_pan->stateSet();
}


void gui::ApplicationGUI::saveNew()
{
  // save to new file path with file chooser
  if(saveToFile(SaveAs))
    design_pan->stateSet();
}


void gui::ApplicationGUI::autoSave()
{
  // no check for changes in state... unneccesary complexity

  qDebug() << tr("Autosave: %1").arg(autosave_dir.absolutePath());
  if(!autosave_dir.exists()){
    qCritical() << tr("Autosave: unable to create tmp instance directory at %1").arg(autosave_dir.path());
    return;
  }

  autosave_ind = (autosave_ind+1) % autosave_num;
  QString autosave_path = autosave_dir.filePath(tr("autosave-%1.xml").arg(autosave_ind));
  saveToFile(AutoSave, autosave_path);

  qDebug() << tr("Autosave complete");
}


void gui::ApplicationGUI::openFromFile()
{
  // prompt user to resolve unsaved changes if program has been modified
  if(design_pan->stateChanged())
    if(!resolveUnsavedChanges())
      return;

  // file dialog
  QFileDialog load_dialog;
  load_dialog.setDefaultSuffix("qad");
  QString prompt_path = load_dialog.getOpenFileName(this, tr("Open File"),
      save_dir.absolutePath(), tr("SQD (*.sqd);;All files (*.*)"));

  if(prompt_path.isEmpty())
    return;

  working_path = prompt_path;
  save_dir.setPath(QFileInfo(prompt_path).absolutePath());
  updateWindowTitle();
  QFile file(working_path);

  if(!file.open(QFile::ReadOnly | QFile::Text)){
    qDebug() << tr("Error when opening file to read: %1").arg(file.errorString());
    return;
  }

  // read from XML stream
  QXmlStreamReader stream(&file);
  qDebug() << tr("Beginning load from %1").arg(file.fileName());

  // TODO load program status here instead of from file
  // TODO if save type is simulation, warn the user when opening the file, especially the fact that sim params will not be retained the next time they save

  // enter the root node and hand the loading over to design panel
  stream.readNextStartElement();
  design_pan->loadFromFile(&stream);

  // clean up
  file.close();
  qDebug() << tr("Load complete");
}


void gui::ApplicationGUI::closeFile()
{
  // prompt user to resolve unsaved changes if program has been modified
  if(design_pan->stateChanged())
    if(!resolveUnsavedChanges())
      return;

  QApplication::quit();
}


bool gui::ApplicationGUI::exportToLabview()
{
  settings::LatticeSettings *lat_settings = settings::LatticeSettings::instance();

  qreal h_dimer_len = lat_settings->get<QPointF>("lattice/a1").x();
  qreal v_dimer_len = lat_settings->get<QPointF>("lattice/a2").y();
  qreal dimer_width = lat_settings->get<QPointF>("cell/b2").x();

  // TODO implement some sort of check for lattice type

  // fetch list of all dbdots
  QList<prim::DBDot*> dbdots = design_pan->getSurfaceDBs(); // NOTE only gets visible layer
  if(dbdots.size() == 0){
    qDebug() << tr("ApplicationGUI: There are no DBDots, nothing can be exported.");
    return false;
  }

  // convert from coord to index (make use of %)
  int x,y;
  QPointF phys_loc;
  QMap<int, QList<int>> db_y_map; // [y,x] y is already sorted by QMap, x needs to be further sorted
  for(auto db : dbdots){
    phys_loc = db->physLoc();
    //qDebug() << tr("x=%1, y=%2");
    //qDebug() << tr("  2*floor(%1 / %2) = %3").arg(phys_loc.x()).arg(h_dimer_len).arg(2*floor(phys_loc.x() / h_dimer_len));
    //qDebug() << tr("  %1 % %2 / %3 = %4").arg(phys_loc.x()).arg(h_dimer_len).arg(dimer_width).arg(fmod(phys_loc.x(), h_dimer_len) / dimer_width);
    x = round(2*floor(phys_loc.x() / h_dimer_len) + fmod(phys_loc.x(), h_dimer_len) / dimer_width);
    //qDebug() << tr("  %1").arg(x);
    y = round(phys_loc.y() / v_dimer_len);
    auto insert_y = db_y_map.find(y);
    if(insert_y == db_y_map.end())
      db_y_map.insert(y, QList<int>({x}));
    else
      insert_y->append(x);
  }

  // sort
  bool sort_asc = true;
  int max_x=0, max_x_local=0; // find max x while performing the sort
  for(auto it = db_y_map.begin(); it != db_y_map.end(); ++it){
    if(sort_asc){
      std::sort((*it).begin(), (*it).end());
      max_x_local = (*it).last();
    }
    else{
      std::sort((*it).begin(), (*it).end(), std::greater<int>());
      max_x_local = (*it).first();
    }
    max_x = max_x > max_x_local ? max_x : max_x_local;
    sort_asc = !sort_asc; // flip the sorting order for the next column
  }
  int max_y = db_y_map.lastKey();

  // construct array with determined samples and channels
  int size_x = max_x+1;
  int size_y = max_y+1;
  int** grid = new int* [size_x];
  for(int i=0; i<size_x; i++)
    grid[i] = new int[size_y]();

  int db_i=1;
  for(auto y_key : db_y_map.keys())
    for(auto x : db_y_map.value(y_key))
      grid[x][y_key] = db_i++;


  // write to file
  QString fn = QFileDialog::getSaveFileName(this, tr("Export to QSi LabView"),
                save_dir.filePath("qsi_labview.lvm"), tr("LabView files (*.lvm)"));

  QFile ef(fn);
  if(!ef.open(QIODevice::WriteOnly)){
    qDebug() << tr("Export to LVM: Error when opening file to export, %1").arg(ef.errorString());
    return false;
  }

  QTextStream output(&ef);

  // Channels
  output << tr("Channels\t%1\n").arg(size_y);   // channels = max y

  // Header info
  QString sample_date = QDateTime::currentDateTime().toString("yyyy/MM/dd");
  QString sample_time = QDateTime::currentDateTime().toString("HH:mm:ss.z");
  QList<QString> out_header;
  out_header.append("Samples");
  out_header.append("Date");
  out_header.append("Time");
  out_header.append("X_Dimension");
  out_header.append("X0");
  out_header.append("Delta_X");
  out_header.append("***End_of_Header***");
  out_header.append("");  // column names of grid
  for(int i=0; i<size_y; i++){
    out_header[0] += tr("\t%1\t").arg(size_x);        // Samples
    out_header[1] += tr("\t%1\t").arg(sample_date);   // Date
    out_header[2] += tr("\t%1\t").arg(sample_time);   // Time
    out_header[3] += "\tTime\t";                      // X_Dimension
    out_header[4] += "\t0\t";                         // X0
    out_header[5] += "\t1\t";                         // Delta_X
    // *** End_of_Header ***
    out_header[7] += tr("X_Value\tUntitled%1\t").arg(i>0 ? tr(" %1").arg(i) : "");  // col names of grid
  }
  out_header[7] += "Comment";

  for(QString text_row : out_header)
    output << tr("%1\n").arg(text_row);

  QString out_grid = "";
  for(int x_ind = 0; x_ind < size_x; x_ind++){
    for(int y_ind = 0; y_ind < size_y; y_ind++){
      out_grid += tr("%1\t%2").arg(x_ind).arg(grid[x_ind][y_ind]);
      if(y_ind != size_y - 1)
        out_grid += "\t"; // don't add extra tab if it's the last column
      else
        out_grid += "\n";
    }
  }

  output << out_grid;

  // idea for format for where to start: what if we just always start at the left dimer row?
  ef.close();

  qDebug() << tr("Export to LVM: Write completed for %1").arg(ef.fileName());

  return true;
}


void gui::ApplicationGUI::aboutVersion()
{
  QString app_name = QCoreApplication::applicationName();
  QString version = QCoreApplication::applicationVersion();

  QMessageBox::about(this, tr("About"), tr("Application: %1\nVersion: %2").arg(app_name).arg(version));
}
