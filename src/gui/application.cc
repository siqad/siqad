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
#include "settings/settings.h"


// init the DialogPanel to NULL until build in constructor
gui::DialogPanel *gui::ApplicationGUI::dialog_pan = 0;


// constructor
gui::ApplicationGUI::ApplicationGUI(const QString &f_path, QWidget *parent)
  : QMainWindow(parent)
{
  // save start time for instance recognition
  start_time = QDateTime::currentDateTime();

  // initialize default save_dir
  save_dir.setPath(QDir::homePath());

  // initialise GUI
  initGUI();

  // load settings, resize mainwindow
  loadSettings();

  // load the specified file
  if (!f_path.isEmpty() && QFile(f_path).exists()) {
    QMessageBox *msg = new QMessageBox(this);
    msg->setAttribute(Qt::WA_DeleteOnClose);
    msg->setText("Wait to load file...");
    msg->setModal(false);
    msg->open();
    cli_load_timer.start(1000);
    connect(&cli_load_timer, &QTimer::timeout,
        [this,f_path,msg](){
          openFromFile(f_path);
          cli_load_timer.stop();
          msg->close();
    });
  }
}

// destructor
gui::ApplicationGUI::~ApplicationGUI()
{
  QStringList settings_remove_paths;
  if (reset_settings) {
    // store list of config files to be removed at the end
    settings::AppSettings *S = settings::AppSettings::instance();
    settings_remove_paths.append(S->pathReplacement(settings::app_settings_path));
    settings_remove_paths.append(S->pathReplacement(settings::gui_settings_path));
    settings_remove_paths.append(S->pathReplacement(settings::lattice_settings_path));
  } else {
    saveSettings();
  }

  // delete dialog panel manually. This avoids segfaults from attempts to echo
  // in dialog panel.
  delete dialog_pan;
  dialog_pan = nullptr;

  // disown the layer dock widget so it can be properly destructed by design panel
  layer_dock->setParent(0);
  item_dock->setParent(0);

  // free memory, parent delete child Widgets so Graphical Items are already
  // handled. Still need to free Settings and commander
  delete commander;
  delete settings::AppSettings::instance();
  delete settings::GUISettings::instance();
  delete settings::LatticeSettings::instance();
  delete prim::Emitter::instance();

  // remove config files if marked for removal.
  for (QString rm_path : settings_remove_paths) {
    QFile f(rm_path);
    f.remove();
  }
}


// PROTECTED

void gui::ApplicationGUI::closeEvent(QCloseEvent *e)
{
  // prompt user to resolve unsaved changes if program has been modified
  if(design_pan->stateChanged()) {
    if(!resolveUnsavedChanges()) {
      e->ignore();
      return;
    }
  }

  //QApplication::quit();
  is_closing = true;
  e->accept();
}

void gui::ApplicationGUI::dragEnterEvent(QDragEnterEvent *e)
{
  if (e->mimeData()->hasUrls())
    e->acceptProposedAction();
}

void gui::ApplicationGUI::dropEvent(QDropEvent *e)
{
  const QMimeData *mime_data = e->mimeData();

  QList<QUrl> url_list = mime_data->urls();
  if (url_list.length() == 1) {
    QString f_path = url_list.at(0).toLocalFile();
    if (f_path.right(4) == ".sqd")
      openFromFile(f_path);
    else
      qWarning() << tr("Only accept dropping of *.sqd files. Your attempted path was %1.").arg(f_path);
  } else {
    qWarning() << tr("Drop event only supports opening exactly 1 file, %1 \
        received instead.").arg(url_list.length());
  }
}


// GUI INITIALISATION

void gui::ApplicationGUI::initGUI()
{
  // Qt GUI flags
  setAcceptDrops(true);

  // Initialize GUI icon
  setWindowIcon(QIcon(":/ico/siqad.svg"));

  // (Windows and macOS): If the system doesn't provide a Qt theme, set the 
  // bundled Breeze theme as the default.
  // Linux builds don't come bundled with a Qt theme.
  if (QIcon::fromTheme("document-new").isNull()) {
    QStringList extra_search_paths = settings::AppSettings::instance()->getPaths("extra_icon_theme_path");
    //QIcon::setThemeSearchPaths(theme_search_paths);
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << extra_search_paths);
    QIcon::setThemeName("Breeze");
  }

  // initialise mainwindow panels
  dialog_pan = new gui::DialogPanel(this); // init first to capture std output
  input_field = new gui::InputField(this);
  design_pan = new gui::DesignPanel(this);
  info_pan = new gui::InfoPanel(this);

  // detachable/pop-up widgets, order matters in some cases due to pointers
  plugin_manager = new gui::PluginManager(this);
  sim_manager = new gui::SimManager(this);
  sim_visualize = new gui::SimVisualizer(design_pan, this);
  job_manager = new gui::JobManager(plugin_manager, sim_visualize, this);
  settings_dialog = new settings::SettingsDialog(this);

  // initialise docks
  initDialogDock();
  initLayerDock();
  initItemDock();
  initSimVisualizerDock();
  initInfoDock();
  tabifyDockWidget(item_dock, layer_dock);

  // initialise bars
  initMenuBar(); // must run before initTopBar
  initTopBar();
  initSideBar();

  // initialise parser
  initCommander();

  // inter-widget signals
  connect(design_pan, &gui::DesignPanel::sig_quickRunSimulation,
          sim_manager, &gui::SimManager::quickRun);
  connect(design_pan, &gui::DesignPanel::sig_cursorPhysLoc,
          info_pan, &gui::InfoPanel::updateCursorPhysLoc);
  connect(design_pan, &gui::DesignPanel::sig_zoom,
          info_pan, &gui::InfoPanel::updateZoom);
  connect(design_pan, &gui::DesignPanel::sig_selectedItems,
          info_pan, &gui::InfoPanel::updateSelItemCount);
  connect(job_manager, &gui::JobManager::sig_executeSQCommand,
          [this](const QString &command)
          {
            commander->parseInputs(command);
          });
  connect(job_manager, &gui::JobManager::sig_showJob,
          sim_visualize, &gui::SimVisualizer::showJob);

  // widget-app gui signals
  connect(job_manager, &gui::JobManager::sig_exportJobProblem,
          [this](comp::JobStep *js, gui::DesignInclusionArea inclusion_area)
          {
            saveToFile(SaveSimulationProblem, js->problemPath(), inclusion_area, js);
          });
  connect(settings_dialog, &settings::SettingsDialog::sig_resetSettings,
          [this](){reset_settings = true;});
  connect(design_pan, &gui::DesignPanel::sig_preDPResetCleanUp,
          [this]()
          {
            // clear SimVisualize content before DesignPanel cleanup
            sim_visualize->clearJob();
          });
  connect(design_pan, &gui::DesignPanel::sig_postDPReset,
          [this](){initState();});
  connect(design_pan, &gui::DesignPanel::sig_setLayerManagerWidget,
          this, &gui::ApplicationGUI::setLayerManagerWidget);
  connect(design_pan, &gui::DesignPanel::sig_setItemManagerWidget,
          this, &gui::ApplicationGUI::setItemManagerWidget);
  connect(design_pan, &gui::DesignPanel::sig_undoStackCleanChanged,
          this, &gui::ApplicationGUI::updateWindowTitle);
  connect(design_pan, &gui::DesignPanel::sig_screenshot,
          this, QOverload<const QString&, QRectF, bool>::of(&gui::ApplicationGUI::designScreenshot));
  //connect(design_pan, &gui::DesignPanel::sig_showSimulationSetup,
  //        [this](){sim_manager->showSimSetupDialog();});
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

  //initialise the color dialog
  // initColorDialog();
  // color_dialog = new QColorDialog(this);
  // color_dialog->setOption(QColorDialog::ShowAlphaChannel,true);
  // color_dialog->setOption(QColorDialog::DontUseNativeDialog,true);

  // prepare initial GUI state
  initState();
}

// void gui::ApplicationGUI::initColorDialog()
// {
//   color_dialog = new QColorDialog(this);
//   color_dialog->setOption(QColorDialog::ShowAlphaChannel,true);
//   color_dialog->setOption(QColorDialog::DontUseNativeDialog,true);
// }

void gui::ApplicationGUI::initMenuBar()
{
  // initialise menus
  QMenu *file = menuBar()->addMenu(tr("&File"));
  QMenu *view = menuBar()->addMenu(tr("&View"));
  //QMenu *edit = menuBar()->addMenu(tr("&Edit"));
  QMenu *tools = menuBar()->addMenu(tr("&Tools"));
  QMenu *help = menuBar()->addMenu(tr("&Help"));

  // inline function to load all lattice files from the lattice directory
  auto loadLatticeFiles = [this](QMenu *menu)
  {
    QDir lattice_dir(":/lattices");
    QStringList lattice_files = lattice_dir.entryList(QStringList("*.xml"), QDir::Files);
    for (QString f : lattice_files) {
      QAction *a = new QAction(f, this);
      menu->addAction(a);
      connect(a, &QAction::triggered,
          [this, f](){
            newFile(tr(":/lattices/%1").arg(f));
          });
    }
  };

  // file menu actions
  QMenu *new_file_menu = new QMenu(tr("&New"), this);
  new_file_menu->setIcon(QIcon::fromTheme("document-new", QIcon(":/ico/fb/document-new.svg")));
  loadLatticeFiles(new_file_menu);
  new_file_menu->addSeparator();
  QAction *browse_lattice_file = new QAction(tr("Browse Lattice..."), this);
  new_file_menu->addAction(browse_lattice_file);

  QAction *open_save = new QAction(
      QIcon::fromTheme("document-open", QIcon(":/ico/fb/document-open.svg")), 
      tr("&Open..."), this);
  QAction *save = new QAction(
      QIcon::fromTheme("document-save", QIcon(":/ico/fb/document-save.svg")), 
      tr("&Save"), this);
  QAction *save_as = new QAction(
      QIcon::fromTheme("document-save-as", QIcon(":/ico/fb/document-save.svg")), 
      tr("Save &As..."), this);
  QAction *import_job_results = new QAction(tr("Import Past Results"), this);
  QAction *quit = new QAction(QIcon::fromTheme("application-exit",
        QIcon(":/ico/fb/exit.svg")), tr("&Quit"), this);
  // new_file->setShortcut(tr("CTRL+N"));
  save->setShortcut(tr("CTRL+S"));
  save_as->setShortcut(tr("CTRL+SHIFT+S"));
  open_save->setShortcut(tr("CTRL+O"));
  quit->setShortcut(tr("CTRL+Q"));
  file->addMenu(new_file_menu);
  // file->addAction(new_file);
  file->addAction(open_save);
  file->addAction(save);
  file->addAction(save_as);
  file->addSeparator();
  file->addAction(import_job_results);
  file->addSeparator();
  file->addAction(quit);

  // view menu actions
  action_sim_visualize = sim_visualize_dock->toggleViewAction();
  action_layer_sel = layer_dock->toggleViewAction();
  action_item_manager = item_dock->toggleViewAction();
  action_dialog_dock_visibility = dialog_dock->toggleViewAction();
  action_sim_visualize->setIcon(QIcon(":/ico/simvis.svg"));
  action_layer_sel->setIcon(QIcon(":/ico/layer.svg"));
  action_dialog_dock_visibility->setIcon(QIcon(":/ico/term.svg"));
  QAction *zoom_in = new QAction(
      QIcon::fromTheme("zoom-in", QIcon(":/ico/fb/zoom-in.svg")),
      tr("Zoom In"));
  QAction *zoom_out = new QAction(
      QIcon::fromTheme("zoom-fit-best", QIcon(":/ico/fb/zoom-out.svg")),
      tr("Zoom Out"));
  QAction *fit_items_in_view = new QAction(
      QIcon::fromTheme("zoom-fit-best", QIcon(":/ico/fb/zoom-fit-all.svg")),
      tr("Fit Items in View"));
  QAction *rotate_view_cw = new QAction(
      QIcon::fromTheme("object-rotate-right", QIcon(":/ico/fb/object-rotate-right.svg")),
      tr("Rotate 90 deg CW"), this);
  QAction *rotate_view_ccw = new QAction(
      QIcon::fromTheme("object-rotate-left", QIcon(":/ico/fb/object-rotate-left.svg")), 
      tr("Rotate 90 deg CCW"), this);
  view->addAction(action_sim_visualize);
  view->addAction(action_layer_sel);
  view->addAction(action_item_manager);
  view->addAction(action_dialog_dock_visibility);
  view->addSeparator();
  view->addAction(zoom_in);
  view->addAction(zoom_out);
  view->addAction(fit_items_in_view);
  view->addSeparator();
  view->addAction(rotate_view_cw);
  view->addAction(rotate_view_ccw);

  zoom_in->setShortcuts({Qt::Key_Equal, Qt::Key_Plus});
  zoom_out->setShortcut(Qt::Key_Minus);
  fit_items_in_view->setShortcut(Qt::CTRL | Qt::Key_Equal);

  //edit menu actions
  //QAction *action_color = new QAction(tr("Color..."), this);
  //edit->addAction(action_color);

  // tools menu actions
  QAction *select_color = new QAction(tr("Select Color..."), this);
  QAction *window_screenshot = new QAction(tr("Window Screenshot..."), this);
  action_screenshot_mode = new QAction(QIcon(":/ico/screenshotmode.svg"), tr("Screenshot Mode"));
  QAction *action_plugin_man = new QAction(tr("Plugin Manager"), this);
  QAction *action_settings_dialog = new QAction(tr("Settings"), this);

  action_screenshot_mode->setCheckable(true);

  
  //tools->addAction(select_color);
  //tools->addSeparator();
  //tools->addAction(window_screenshot);  // TODO disabled due to imperfect screenshot results
  tools->addAction(action_screenshot_mode);
  tools->addSeparator();
  tools->addAction(action_plugin_man);
  tools->addSeparator();
  tools->addAction(action_settings_dialog);

  // help menu actions
  QAction *open_log_dir = new QAction(tr("Open Log Directory"), this);
  QAction *open_autosave_dir = new QAction(tr("Open Autosave Directory"), this);
  QAction *open_config_dir = new QAction(tr("Open Config Directory"), this);
  QAction *about_version = new QAction(tr("About"), this);

  help->addAction(open_log_dir);
  help->addAction(open_autosave_dir);
  help->addAction(open_config_dir);
  help->addSeparator();
  help->addAction(about_version);

  // connect(new_file, &QAction::triggered, this, &gui::ApplicationGUI::newFile);
  connect(quit, &QAction::triggered, this, &QWidget::close);
  connect(save, &QAction::triggered,
      [this](){
        if (saveToFile(Save))
          design_pan->stateSet();
      });
  connect(save_as, &QAction::triggered,
      [this](){
        if (saveToFile(SaveAs))
          design_pan->stateSet();
      });
  connect(open_save, &QAction::triggered,
      [this](){openFromFile();});
  connect(import_job_results, &QAction::triggered,
      [this](){
        comp::SimJob *j = comp::SimJob::importSimJob();
        if (j != nullptr) {
          job_manager->addJob(j);
          sim_visualize->showJob(j);
        }
      });
  connect(zoom_in, &QAction::triggered,
      [this](){design_pan->stepZoom(true);});
  connect(zoom_out, &QAction::triggered,
      [this](){design_pan->stepZoom(false);});
  connect(fit_items_in_view, &QAction::triggered,
      [this](){design_pan->fitItemsInView(false);});
  connect(rotate_view_cw, &QAction::triggered, design_pan, &gui::DesignPanel::rotateCw);
  connect(rotate_view_ccw, &QAction::triggered, design_pan, &gui::DesignPanel::rotateCcw);
  connect(browse_lattice_file, &QAction::triggered, this, &gui::ApplicationGUI::chooseLatticeForNewFile);
  //connect(action_color, &QAction::triggered, this, &gui::ApplicationGUI::selectColor);
  connect(select_color, &QAction::triggered, this, &gui::ApplicationGUI::selectColor);
  connect(window_screenshot, &QAction::triggered, this, &gui::ApplicationGUI::screenshot);
  connect(action_plugin_man, &QAction::triggered,
      [this](){
        plugin_manager->show();
      });
  connect(open_log_dir, &QAction::triggered,
      []() {
        settings::AppSettings *s = settings::AppSettings::instance();
        QString log_path = s->getPath("log/logdir");
        QDesktopServices::openUrl(QUrl::fromLocalFile(log_path));
      });
  connect(open_autosave_dir, &QAction::triggered,
      [this]() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(autosave_dir.absolutePath()));
      });
  connect(open_config_dir, &QAction::triggered,
      [this]() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(settings::AppSettings::instance()->pathReplacement("<CONFIG>")));
      });
  connect(action_screenshot_mode, &QAction::triggered,
          this, &gui::ApplicationGUI::toggleScreenshotMode);
  connect(action_settings_dialog, &QAction::triggered,
      [this](){settings_dialog->show();});
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
  action_run_sim = new QAction(QIcon(":/ico/jobnew.svg"), tr("New Job..."));
  action_run_sim->setShortcut(tr("CTRL+R"));
  connect(action_run_sim, &QAction::triggered,
          [this](){job_manager->showNewJob();});

  action_job_log = new QAction(QIcon(":/ico/joblog.svg"), tr("View Jobs"));
  action_job_log->setShortcut(tr("CTRL+L"));
  connect(action_job_log, &QAction::triggered,
          [this](){job_manager->showViewJobs();});

  /*
  action_repeat_sim = new QAction(tr("Repeat Previous Simulation"), this);
  action_repeat_sim->setShortcut(tr("CTRL+SHIFT+R"));
  connect(action_repeat_sim, &QAction::triggered,
	  this, &gui::ApplicationGUI::repeatSimulation);
  addAction(action_repeat_sim);
  */


  // add them to top bar
  top_bar->addAction(action_run_sim);
  top_bar->addAction(action_job_log);
  top_bar->addAction(action_sim_visualize);           // already initialised in menu bar
  top_bar->addAction(action_layer_sel);               // already initialised in menu bar
  top_bar->addAction(action_dialog_dock_visibility);  // already initialised in menu bar
  top_bar->addAction(action_screenshot_mode);

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
  QActionGroup *ag_design = new QActionGroup(side_bar);

  action_select_tool = side_bar->addAction(QIcon(":/ico/select.svg"),
      tr("Select tool"));
  action_drag_tool = side_bar->addAction(QIcon(":/ico/drag.svg"),
      tr("Drag tool"));
  action_dbgen_tool = side_bar->addAction(QIcon(":/ico/dbgen.svg"),
      tr("DB tool"));
  action_electrode_tool = side_bar->addAction(QIcon(":/ico/drawelectrode.svg"),
      tr("Electrode tool"));
  action_screenshot_area_tool = side_bar->addAction(QIcon(":/ico/screenshotarea.svg"),
      tr("Screenshot area tool"));
  action_scale_bar_anchor_tool = side_bar->addAction(QIcon(":/ico/scalebaranchortool.svg"),
      tr("Scale bar anchor tool"));

  action_dbgen_tool->setShortcut(tr("D"));
  action_electrode_tool->setShortcut(tr("E"));

  ag_design->addAction(action_select_tool);
  ag_design->addAction(action_drag_tool);
  ag_design->addAction(action_dbgen_tool);
  ag_design->addAction(action_electrode_tool);
  /*
  ag_design->addAction(action_label_tool);
  */
  ag_design->addAction(action_screenshot_area_tool);
  ag_design->addAction(action_scale_bar_anchor_tool);

  al_screenshot.append(action_screenshot_area_tool);
  al_screenshot.append(action_scale_bar_anchor_tool);
  hideActionList(al_screenshot);

  action_select_tool->setCheckable(true);
  action_drag_tool->setCheckable(true);
  action_dbgen_tool->setCheckable(true);
  action_electrode_tool->setCheckable(true);
  /*
  action_label_tool->setCheckable(true);
  */
  action_screenshot_area_tool->setCheckable(true);
  action_scale_bar_anchor_tool->setCheckable(true);

  connect(action_select_tool, &QAction::triggered,
          this, &gui::ApplicationGUI::setToolSelect);
  connect(action_drag_tool, &QAction::triggered,
          this, &gui::ApplicationGUI::setToolDrag);
  connect(action_dbgen_tool, &QAction::triggered,
          this, &gui::ApplicationGUI::setToolDBGen);
  connect(action_electrode_tool, &QAction::triggered,
          this, &gui::ApplicationGUI::setToolElectrode);
  connect(action_screenshot_area_tool, &QAction::triggered,
          this, &gui::ApplicationGUI::setToolScreenshotArea);
  connect(action_scale_bar_anchor_tool, &QAction::triggered,
          this, &gui::ApplicationGUI::setToolScaleBarAnchor);

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

void gui::ApplicationGUI::initSimVisualizerDock()
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

  connect(sim_visualize_dock, &QDockWidget::visibilityChanged,
          [this](const bool &visible)
          {
            if (!visible) {
              sim_visualize->clearJob();
              design_pan->clearSimResults();
            }
          });
  connect(sim_visualize, &gui::SimVisualizer::sig_showJobInvoked,
          [this]() {
            sim_visualize_dock->show();
          });
  connect(sim_visualize, &gui::SimVisualizer::sig_loadProblemFile,
          [this](const QString &fpath) {
            design_pan->clearSimResults();
            QFile file(fpath);
            QXmlStreamReader rs(&file);
            if(!file.open(QFile::ReadOnly | QFile::Text)){
              qDebug() << tr("Error when opening file to read: %1").arg(file.errorString());
              return;
            }
            rs.readNextStartElement();
            design_pan->loadFromFile(&rs, true);
            design_pan->enableSimVis();
            design_pan->setDisplayMode(DisplayMode::SimDisplayMode);
          });

  QScrollArea *sa_sim_vis = new QScrollArea;
  sa_sim_vis->setWidget(sim_visualize);
  sa_sim_vis->setWidgetResizable(true);

  sim_visualize_dock->setWidget(sa_sim_vis);
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

  QScrollArea *sa_layer_dock = new QScrollArea;
  sa_layer_dock->setWidget(design_pan->layerManagerSideWidget());
  sa_layer_dock->setWidgetResizable(true);

  layer_dock->setWidget(sa_layer_dock);
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


void gui::ApplicationGUI::initInfoDock()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  info_dock = new QDockWidget("Information Panel");

  Qt::DockWidgetArea area;
  area = static_cast<Qt::DockWidgetArea>(gui_settings->get<int>("INFODOCK/loc"));

  info_dock->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea|Qt::BottomDockWidgetArea);

  info_dock->setMinimumWidth(gui_settings->get<int>("INFODOCK/mw"));

  info_dock->setWidget(info_pan);
  info_dock->show();
  addDockWidget(area, info_dock);
}

void gui::ApplicationGUI::initCommander()
{
  commander = new Commander();
  commander->setDesignPanel(design_pan);
  commander->setDialogPanel(dialog_pan);
  commander->addKeyword(tr("add"));
  commander->addKeyword(tr("remove"));
  commander->addKeyword(tr("echo"));
  commander->addKeyword(tr("help"));
  commander->addKeyword(tr("run"));
  commander->addKeyword(tr("move"));
}

void gui::ApplicationGUI::setLayerManagerWidget(QWidget *widget)
{
  qDebug() << "Making layer manager widget";

  QScrollArea *sa_layer_dock = new QScrollArea;
  sa_layer_dock->setWidget(widget);
  sa_layer_dock->setWidgetResizable(true);

  layer_dock->setWidget(sa_layer_dock);
}

void gui::ApplicationGUI::setItemManagerWidget(QWidget *widget)
{
  qDebug() << "Re-setting item manager widget";
  item_dock->setWidget(widget);
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
  sim_visualize->designPanelResetActions();
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
      qCritical() << tr("Failed to create autosave directory");
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

  if(autosave_dir.removeRecursively())
    qDebug() << tr("Removed autosave directory: %1").arg(autosave_dir.path());
  else
    qDebug() << tr("Failed to remove autosave directory: %1").arg(autosave_dir.path());
}




// PUBLIC SLOTS

void gui::ApplicationGUI::updateWindowTitle()
{
  if (!is_closing){
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
    case gui::ToolType::ScreenshotAreaTool:
      action_screenshot_area_tool->setChecked(true);
      setToolScreenshotArea();
      break;
    case gui::ToolType::ScaleBarAnchorTool:
      action_scale_bar_anchor_tool->setChecked(true);
      setToolScaleBarAnchor();
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
  if(design_pan->displayMode() != gui::DisplayMode::DesignMode){
    qDebug() << tr("electrode tool not allowed outside of design mode");
    return;
  }
  qDebug() << tr("selecting electrode tool");
  design_pan->setTool(gui::ToolType::ElectrodeTool);
}

void gui::ApplicationGUI::setToolScreenshotArea()
{
  qDebug() << tr("selecting screenshot area tool");
  design_pan->setTool(gui::ToolType::ScreenshotAreaTool);
}

void gui::ApplicationGUI::setToolScaleBarAnchor()
{
  qDebug() << tr("selecting scale bar anchor tool");
  design_pan->setTool(gui::ToolType::ScaleBarAnchorTool);
}

void gui::ApplicationGUI::setToolLabel()
{
  qDebug() << tr("selecting label tool");
  design_pan->setTool(gui::ToolType::LabelTool);
}


void gui::ApplicationGUI::showActionList(QList<QAction*> al)
{
  for (QAction *ac : al) {
    ac->setVisible(true);
  }
}

void gui::ApplicationGUI::hideActionList(QList<QAction*> al)
{
  for (QAction *ac : al) {
    ac->setVisible(false);
  }
}


void gui::ApplicationGUI::chooseLatticeForNewFile()
{
  QString fname = QFileDialog::getOpenFileName(
    this, tr("Select lattice file"), QString(), tr("XML (*.xml)"));

  newFile(fname);
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
      QString elem_name = rs.name().toString();
      if(elem_name == "eng_info"){
        rs.readNext();
        // basic engine info
        while(rs.name().toString() != "eng_info"){
          // TODO read engine info
          // can't get rid of this because there's no guarantee that the result file is being
          // read by a machine that has the simulator installed.
        }
      }
      else if(elem_name == "sim_param"){
        // TODO simulator class
      }
      else if(elem_name == "physloc"){
        // TODO simulator class
      }
      else if(elem_name == "elec_dist"){
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
  design_pan->showColorDialog(design_pan->selectedItems());
}

void gui::ApplicationGUI::beginScreenshotMode()
{
  action_screenshot_mode->setChecked(true);
  display_mode_cache = design_pan->displayMode();
  design_pan->setDisplayMode(ScreenshotMode);
  showActionList(al_screenshot);
}

void gui::ApplicationGUI::endScreenshotMode()
{
  action_screenshot_mode->setChecked(false);
  design_pan->setDisplayMode(display_mode_cache);
  hideActionList(al_screenshot);
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


void gui::ApplicationGUI::fullDesignScreenshot()
{
  // TODO fold this into the designScreenshot function, if rect input is null
  // then default to full design screenshot.
  // TODO the save dialog will probably also be handled by the screenshot manager

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

  // get save path
  QString fpath = QFileDialog::getSaveFileName(this, tr("Save File"), img_dir.path(),
                      tr("SVG files (*.svg)"));

  designScreenshot(fpath, rect, true);
}


void gui::ApplicationGUI::designScreenshot(const QString &target_img_path, QRectF rect, bool always_overwrite)
{
  qDebug() << tr("taking screenshot for rect (%1, %2) (%3, %4)")
      .arg(rect.left()).arg(rect.top()).arg(rect.right()).arg(rect.bottom());

  settings::GUISettings *S = settings::GUISettings::instance();

  // check if target directory is writable
  if (!QFileInfo(QFileInfo(target_img_path).dir().absolutePath()).isWritable()) {
    qDebug() << tr("Directory not writable.");
    //endScreenshotMode();
    return;
  }

  // check if target file already exists
  if (!always_overwrite && QFileInfo(target_img_path).exists()) {
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "File exists",
        tr("The target image file name %1 already exists. Do you want to \
          overwrite it?").arg(target_img_path),
        QMessageBox::Yes|QMessageBox::No);
    // TODO add another button for browsing another path
    if (reply == QMessageBox::No) {
      //endScreenshotMode();
      return;
    }
  }

  // take screenshot of full display panel view if rect is null
  if (rect.isNull()) {
    qDebug() << "No screenshot clip area specified, taking full design screenshot";
    gui::DesignPanel *dp = this->design_pan;
    QRect dp_frame = dp->frameRect();
    QPointF dp_tl = dp->mapToScene(dp->mapFromParent(dp_frame.topLeft()));
    QPointF dp_br = dp->mapToScene(dp->mapFromParent(dp_frame.bottomRight()));
    rect = QRectF(dp_tl, dp_br);
  }

  QSvgGenerator gen;
  gen.setFileName(target_img_path);
  qreal screenshot_px_per_ang = S->get<qreal>("view/screenshot_px_per_ang");
  qreal sf = screenshot_px_per_ang / prim::Item::scale_factor; // shrink factor
  QRectF svgrect = QRectF(rect.x(), rect.y(), rect.width()*sf, rect.height()*sf);
  gen.setViewBox(svgrect);

  QPainter painter;
  painter.begin(&gen);
  design_pan->screenshot(&painter, rect, svgrect);

  painter.end();

  //endScreenshotMode();
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
void gui::ApplicationGUI::newFile(QString lattice_file_path)
{
  // prompt user to resolve unsaved changes if program has been modified
  if(design_pan->stateChanged())
    if(!resolveUnsavedChanges())
      return;

  // reset widgets and reinitialize GUI state
  working_path.clear();
  design_pan->resetDesignPanel(lattice_file_path);
  initState();
}


// save/load:
bool gui::ApplicationGUI::saveToFile(gui::ApplicationGUI::SaveFlag flag,
                                     const QString &path,
                                     gui::DesignInclusionArea inclusion_area,
                                     comp::JobStep *job_step)
{
  if (design_pan->displayMode() == gui::SimDisplayMode
      && (flag == Save || flag == SaveAs)) {
    // Don't allow saving to file when displaying simulation results. This is 
    // due to the ability to visualize previous simulation results, which may
    // include different components in the design. Allowing users to save in
    // SimVis mode could lead to confusion.
    QMessageBox *msg = new QMessageBox(this);
    msg->setAttribute(Qt::WA_DeleteOnClose);
    msg->setText("Please exit Simulation Visualization before saving.");
    msg->setModal(false);
    msg->open();
    return false;
  }

  QString write_path;

  QFileDialog save_dialog;
  // determine target file
  if (!path.isEmpty()) {
    // ask for user confirmation if the path already contains a file and the
    // save flag is not an AutoSave
    if (QFile(path).exists() && flag != AutoSave) {
      QMessageBox::StandardButton reply = QMessageBox::question(this, "Warning",
          tr("The path %1 already contains a file. Overwrite?").arg(path),
          QMessageBox::Yes|QMessageBox::No);
      if (reply == QMessageBox::No) {
        qDebug() << tr("User chose not to overwrite %1, save terminated.").arg(path);
        return false;
      }
    }
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

  // add .sqd extension if there isn't
  if (!QStringList({"sqd", "qad", "xml"}).contains(QFileInfo(write_path).suffix()))
    write_path.append(".sqd");

  // set file name to write_path.writing while writing to prevent loss of
  // previous save if this save fails
  QFile file(write_path+".writing");

  if(!file.open(QIODevice::WriteOnly)){
    qDebug() << tr("Save: Error when opening file to save: %1").arg(file.errorString());
    return false;
  }

  // WRITE TO XML
  QXmlStreamWriter ws(&file);
  qDebug() << tr("Save: Beginning write to %1").arg(file.fileName());
  ws.setAutoFormatting(true);
  ws.writeStartDocument();

  // call the save functions for each relevant class
  ws.writeStartElement("siqad");

  // save program flags
  ws.writeComment("Program Flags");
  ws.writeStartElement("program");

  QString file_purpose;
  switch(flag){
    case SaveSimulationProblem:
      file_purpose = "simulation";
      break;
    case AutoSave:
      // Introduced in SiQAD v0.2.2
      file_purpose = "autosave";
      break;
    default:
      file_purpose = "save";
      break;
  }
  ws.writeTextElement("file_purpose", file_purpose);
  ws.writeTextElement("version", QCoreApplication::applicationVersion());
  ws.writeTextElement("date", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));

  ws.writeEndElement();

  // save simulation parameters
  if (flag == SaveSimulationProblem && job_step != nullptr) {
    ws.writeStartElement("sim_params");
    for (const QString &key : job_step->jobParameters().keys()) {
      ws.writeTextElement(key, job_step->jobParameters().value(key));
    }
    ws.writeEndElement();
  }

  // save design panel content (including GUI flags, layers and their corresponding contents (electrode, dbs, etc.)
  design_pan->writeToXmlStream(&ws, inclusion_area);

  // close root element & close file
  ws.writeEndElement();
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


void gui::ApplicationGUI::openFromFile(const QString &f_path)
{
  // prompt user to resolve unsaved changes if program has been modified
  if(design_pan->stateChanged())
    if(!resolveUnsavedChanges())
      return;

  QString open_path;
  if (!f_path.isEmpty()) {
    open_path = f_path;
  } else {
    QFileDialog load_dialog;
    load_dialog.setDefaultSuffix("sqd");
    open_path = load_dialog.getOpenFileName(this, tr("Open File"),
        save_dir.absolutePath(), tr("SQD (*.sqd);;All files (*.*)"));
    if(open_path.isEmpty()) {
      qDebug() << "No file chosen, cancelling file open operation.";
      return;
    }
  }

  QFile file(open_path);

  if(!file.open(QFile::ReadOnly | QFile::Text)){
    qDebug() << tr("Error when opening file to read: %1").arg(file.errorString());
    return;
  }

  working_path = open_path;
  save_dir.setPath(QFileInfo(open_path).absolutePath());
  updateWindowTitle();

  // read from XML stream
  QXmlStreamReader rs(&file);
  qDebug() << tr("Beginning load from %1").arg(file.fileName());

  // TODO load program status here instead of from file
  // TODO if save type is simulation, warn the user when opening the file, especially the fact that sim params will not be retained the next time they save

  // enter the root node and hand the loading over to design panel
  rs.readNextStartElement();
  design_pan->loadFromFile(&rs);

  // clean up
  file.close();
  qDebug() << tr("Load complete");
}

void gui::ApplicationGUI::aboutVersion()
{
  QString app_name = QCoreApplication::applicationName();
  QString version = QCoreApplication::applicationVersion();

  QMessageBox::about(this, tr("About"), tr("Application: %1\nVersion: %2").arg(app_name).arg(version));
}
