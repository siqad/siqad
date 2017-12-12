// @file:     settings.cc
// @author:   Jake
// @created:  2016.10.31
// @editted:  2017.05.08  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Implemetation of the Settings class and derived classes

#include "settings.h"


// TODO: Don't like that there is all this messy manual creation of the default
//      settings files here... but should still allow for the contingency that
//      some idiot will delete their default settings files from the resources
//      directory. Have a hidden '.default_gen.py' script which generates the
//      default "factory" settings files.

// TODO: Add a processing layer for reading/writting particulary types of data
//      from the .ini files: QColor and QPointF get mapped to QVariant hex strings
//      which aren't very edittable.

// initialize default settings
QSettings *settings::AppSettings::defs = settings::AppSettings::m_defs();
QSettings *settings::GUISettings::defs = settings::GUISettings::m_defs();
QSettings *settings::LatticeSettings::defs = settings::LatticeSettings::m_defs();

// set static singleton pointers to NULL
settings::AppSettings* settings::AppSettings::inst = 0;
settings::GUISettings* settings::GUISettings::inst = 0;
settings::LatticeSettings* settings::LatticeSettings::inst = 0;


// AppSettings::

settings::AppSettings *settings::AppSettings::instance()
{
  // if no instance has been created, initialize
  if(!inst)
    inst = new settings::AppSettings();
  return inst;
}


// GUISettings::

settings::GUISettings *settings::GUISettings::instance()
{
  // if no instance has been created, initialize
  if(!inst)
    inst = new settings::GUISettings();
  return inst;
}


// LatticeSettings::

settings::LatticeSettings *settings::LatticeSettings::instance()
{
  // if no instance has been created, initialize
  if(!inst)
    inst = new settings::LatticeSettings();
  return inst;
}

void settings::LatticeSettings::updateLattice(const QString &fname)
{
  // delete old lattice settings
  if(inst){
    delete inst;
    inst = 0;
  }

  // reconstruct defaults
  defs = settings::LatticeSettings::m_defs();

  // create new instance of lattice settings
  if(fname.isEmpty())
    inst = new LatticeSettings();
  else
    inst = new LatticeSettings(fname);
}




// DEFAULT SETTINGS CONSTRUCTORS

QSettings *settings::AppSettings::m_defs()
{
  qDebug("Loading default application settings...");
  QSettings *S = new QSettings("src/settings/defaults/app_settings.ini", QSettings::IniFormat);

  // overwrites existing default values with same keys... no check
  S->setValue("view/hidpi_support", false);

  S->setValue("log/override", true);
  S->setValue("log/tofile", true);
  S->setValue("log/logfile", QString("src/log/log.txt"));

  S->setValue("snap/diameter", 5.); //relative to scale_fact

  S->setValue("dir/lattice", QString("<BINPATH>/src/settings/lattices"));

  S->setValue("phys/debye_length", 50);
  S->setValue("phys/epsr", 10);

  S->setValue("phys/eng_lib_dir", QString("<BINPATH>/src/phys/"));
  S->setValue("phys/runtime_temp_dir", QString("<SYSTMP>/db-sim/phys/"));

  S->setValue("save/autosaveroot", QString("<SYSTMP>/db-sim/autosave/"));
  S->setValue("save/autosavenum", 3);
  S->setValue("save/autosaveinterval", 300); // in seconds

  return S;
}

QSettings* settings::GUISettings::m_defs()
{
  qDebug("Loading default gui settings...");
  QSettings *S = new QSettings("src/settings/defaults/gui_settings.ini", QSettings::IniFormat);

  S->setValue("MWIN/size", QSize(1400, 800));
  S->setValue("TBAR/mh", 60);
  S->setValue("SBAR/mw", 60);
  S->setValue("SBAR/ico", 0.7); // icon size, relative to mw
  S->setValue("SBAR/loc", 1);
  S->setValue("ODOCK/mw", 120); // option dock minimum width
  S->setValue("DDOCK/mh", 120); // dialog dock minimum height
  S->setValue("Panel/logw", 400);
  S->setValue("Panel/maxh", 150);

  S->setValue("lattice/fname", QString("src/settings/lattices/si_100_2x1.ini"));
  S->setValue("lattice/xy", QPoint(60,60));

  // QGraphicsView
  S->setValue("view/scale_fact", 10);           // pixels/angstrom in the main view
  S->setValue("view/bg_col", QColor(40,50,60)); // background color
  S->setValue("view/zoom_factor", 0.1);         // scaling factor for zoom operations
  S->setValue("view/zoom_boost", 2);            // must have factor*boost < 1
  S->setValue("view/zoom_min", .1);             // minimum zoom factor
  S->setValue("view/zoom_max", 10);             // maximum zoom factor
  S->setValue("view/wheel_pan_step", 20);       // screen pan per wheel tick
  S->setValue("view/wheel_pan_boost", 5);       // shift-boost factor
  S->setValue("view/padding", .1);              // additional space around draw region

  // dangling bond parameters
  S->setValue("dbdot/diameter_m", 1.3);                     // dot diameter
  S->setValue("dbdot/diameter_l", 2);                     // dot diameter
  S->setValue("dbdot/edge_width", .1);                    // edge width rel. to diameter
  S->setValue("dbdot/edge_col", QColor(255,255,255));     // edge color, unselected
  S->setValue("dbdot/selected_col", QColor(0, 100, 255)); // edge color, selected
  S->setValue("dbdot/fill_col", QColor(200,200,200));     // dot fill color
  S->setValue("dbdot/fill_col_sel", QColor(150,150,150)); // dot fill color (selected)
  S->setValue("dbdot/fill_col_drv", QColor(255,90,90));   // dot fill color for driver dot (forced electron=1)
  S->setValue("dbdot/fill_col_drv_sel", QColor(128,95,173));  // dot fill color for driver dot (forced electron=1)
  S->setValue("dbdot/fill_col_elec", QColor(0,255,0));   // dot fill color for driver dot (forced electron=1)
  S->setValue("dbdot/fill_col_elec_sel", QColor(0,150,0));  // dot fill color for driver dot (forced electron=1)

  // lattice dot parameters
  S->setValue("latdot/diameter", 1.0);                    // dot diameter
  S->setValue("latdot/edge_width", .05);                  // edge width rel. to diameter
  S->setValue("latdot/edge_col", QColor(255,255,255,50)); // edge color
  S->setValue("latdot/fill_col", QColor(0,0,0,0));        // fill color
  S->setValue("latdot/inner_fill", .5);                   // inner fill factor
  S->setValue("latdot/inner_fill_col", QColor(255, 255, 0));  // inner colour

  // ghost parameters
  S->setValue("ghost/dot_diameter", .6);                    // ghost dot diameter
  S->setValue("ghost/valid_col", QColor(0, 255, 0, 255));   // color for valid placements
  S->setValue("ghost/invalid_col", QColor(255, 0, 0, 255)); // color for invalid placements

  // ghost box parameters
  S->setValue("ghostbox/valid_col", QColor(0, 150, 0));

  // aggregate parameters
  S->setValue("aggregate/edge_col", QColor(9, 255, 200, 150));  // bounding box color
  S->setValue("aggregate/edge_col_hovered", QColor(9, 255, 200, 150));  // bounding box color

  // electrode parameters
  S->setValue("electrode/edge_width", .05);                   // edge width of box lines
  S->setValue("electrode/edge_col", QColor(60,60,60));     // edge color
  S->setValue("electrode/fill_col", QColor(100,100,100));     // fill color
  S->setValue("electrode/selected_col", QColor(0, 100, 255)); // edge color, selected

  return S;
}

QSettings* settings::LatticeSettings::m_defs()
{
  qDebug("Loading default lattice settings...");
  QSettings *S = new QSettings("src/settings/defaults/lattices/si_100_2x1.ini", QSettings::IniFormat);

  S->setValue("cell/N", 2);

  S->setValue("cell/b1", QPointF(0, 0));
  S->setValue("cell/b2", QPointF(2.4, 0));

  S->setValue("lattice/a1", QPointF(7.68, 0));
  S->setValue("lattice/a2", QPointF(0, 3.84));


  return S;
}
