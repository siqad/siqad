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
}

gui::ApplicationGUI::~ApplicationGUI()
{
  // save current settings
  saveSettings();

  // delete tool bars... may not be needed
  delete top_bar;
  delete side_bar;
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

  connect(quit, &QAction::triggered, qApp, QApplication::quit);
}

void gui::ApplicationGUI::initTopBar()
{
  settings::GUISettings gui_settings;

  top_bar = new QToolBar(tr("Top Bar"));

  // location behaviour
  top_bar->setFloatable(false);
  top_bar->setMovable(false);

  // size policy
  top_bar->setMinimumHeight(gui_settings.get<int>("TBAR/mh"));
  top_bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

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
  side_bar->setMinimumWidth(gui_settings.get<int>("SBAR/mw"));

  // actions
  action_select_tool = side_bar->addAction(QIcon(":/ico/select.svg"), tr("&Select tool"));
  action_drag_tool = side_bar->addAction(QIcon(":/ico/drag.svg"), tr("&Drag tool"));

  connect(action_select_tool, &QAction::triggered, this, &gui::ApplicationGUI::setToolSelect);
  connect(action_drag_tool, &QAction::triggered, this, &gui::ApplicationGUI::setToolDrag);

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
