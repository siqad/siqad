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

QMap<QString, QString> settings::Settings::path_map = QMap<QString, QString>();

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
  // TODO remove QSettings *S = new QSettings("src/settings/defaults/app_settings.ini", QSettings::IniFormat);
  QString the_path = pathReplacement("<CONFIG>/defaults/app_settings.ini");
  qDebug() << QObject::tr("app settings path: %1").arg(the_path);
  QSettings *S = new QSettings(
      pathReplacement("<CONFIG>/defaults/app_settings.ini"),
      QSettings::IniFormat
  );

  // overwrites existing default values with same keys... no check
  S->setValue("view/hidpi_support", false);

  S->setValue("log/override", false);
  S->setValue("log/tofile", false);
  S->setValue("log/logfile", QString("<SYSTMP>/log/log.txt"));

  S->setValue("snap/diameter", 5.); //relative to scale_fact

  S->setValue("dir/lattice", QString("<BINPATH>/src/settings/lattices"));

  S->setValue("phys/debye_length", 50);
  S->setValue("phys/epsr", 10);

  S->setValue("phys/eng_lib_dir", QString("<BINPATH>/src/phys/"));
  S->setValue("phys/runtime_temp_dir", QString("<SYSTMP>/phys/"));

  S->setValue("save/autosaveroot", QString("<SYSTMP>/autosave/"));
  S->setValue("save/autosavenum", 3);
  S->setValue("save/autosaveinterval", 300); // in seconds

  return S;
}

QSettings* settings::GUISettings::m_defs()
{
  qDebug("Loading default gui settings...");
  // TODO remove QSettings *S = new QSettings("src/settings/defaults/gui_settings.ini", QSettings::IniFormat);
  QSettings *S = new QSettings(
      pathReplacement("<CONFIG>/defaults/gui_settings.ini"),
      QSettings::IniFormat
  );

  S->setValue("MWIN/size", QSize(1400, 800));
  S->setValue("TBAR/mh", 60);
  S->setValue("SBAR/mw", 60);
  S->setValue("SBAR/ico", 0.7); // icon size, relative to mw
  S->setValue("SBAR/loc", 1);
  S->setValue("SIMVDOCK/mw", 120);  // option dock minimum width
  S->setValue("SIMVDOCK/loc", 2);   // sim visualise dock default to right
  S->setValue("DDOCK/mh", 120);     // dialog dock minimum height
  S->setValue("DDOCK/loc", 8);      // dialog dock default to bottom
  S->setValue("LAYDOCK/mw", 120);   // layer dock minimum width
  S->setValue("LAYDOCK/loc", 8);    // layer dock default to bottom
  S->setValue("Panel/logw", 400);

  S->setValue("lattice/fname", QString("src/settings/lattices/si_100_2x1.ini"));
  S->setValue("lattice/xy", QPoint(100,100));

  // QGraphicsView
  S->setValue("view/scale_fact", 10);           // pixels/angstrom in the main view
  S->setValue("view/bg_col", QColor(40,50,60)); // background color
  S->setValue("view/bg_col_pb", QColor(255,255,255)); // background color
  S->setValue("view/zoom_factor", 0.1);         // scaling factor for zoom operations
  S->setValue("view/zoom_boost", 2);            // must have factor*boost < 1
  S->setValue("view/zoom_min", .1);             // minimum zoom factor
  S->setValue("view/zoom_max", 10);             // maximum zoom factor
  S->setValue("view/wheel_pan_step", 20);       // screen pan per wheel tick
  S->setValue("view/wheel_pan_boost", 5);       // shift-boost factor
  S->setValue("view/padding", .1);              // additional space around draw region

  // dangling bond parameters
  S->setValue("dbdot/diameter_m", 1.5);                     // dot diameter
  S->setValue("dbdot/diameter_l", 2);                     // dot diameter
  S->setValue("dbdot/publish_scale", 2);                  // scaling for publish mode
  S->setValue("dbdot/edge_width", .1);                    // edge width rel. to diameter
  S->setValue("dbdot/edge_col", QColor(255,255,255));     // edge color
  S->setValue("dbdot/edge_col_sel", QColor(0, 100, 255)); // edge color (selected)
  S->setValue("dbdot/edge_col_hovered", QColor(0, 100, 255)); // edge color (hovered)
  S->setValue("dbdot/edge_col_pb", QColor(0, 100, 255, 0)); // edge color (publish mode)
  S->setValue("dbdot/fill_col", QColor(200,200,200));     // dot fill color
  S->setValue("dbdot/fill_col_sel", QColor(150,150,150)); // dot fill color (selected)
  S->setValue("dbdot/fill_col_hovered", QColor(220,220,220)); // dot fill color (hovered)
  S->setValue("dbdot/fill_col_pb", QColor(150,150,150)); // dot fill color (publish mode)
  S->setValue("dbdot/fill_col_drv", QColor(0,255,0));   // dot fill color for driver dot (forced electron=1)
  S->setValue("dbdot/fill_col_drv_sel", QColor(0,150,0));  // dot fill color for driver dot (forced electron=1)
  S->setValue("dbdot/fill_col_drv_hovered", QColor(0,180,0));  // dot fill color for driver dot (forced electron=1)
  S->setValue("dbdot/fill_col_drv_pb", QColor(0,255,0));  // dot fill color for driver dot (forced electron=1)
  S->setValue("dbdot/fill_col_elec", QColor(255,90,90));   // dot fill color for dot showing electron
  S->setValue("dbdot/fill_col_elec_sel", QColor(255,90,90));  // dot fill color for dot showing electron
  S->setValue("dbdot/fill_col_elec_hovered", QColor(255,90,90));  // dot fill color for dot showing electron
  S->setValue("dbdot/fill_col_elec_pb", QColor(255,90,90));  // dot fill color for dot showing electron

  // lattice dot parameters
  S->setValue("latdot/diameter", 1.0);                    // dot diameter
  S->setValue("latdot/edge_width", .08);                  // edge width rel. to diameter
  S->setValue("latdot/publish_scale", 2);                  // scaling for publish mode
  S->setValue("latdot/edge_col", QColor(255,255,255,70)); // edge color
  S->setValue("latdot/edge_col_pb", QColor(0,0,0,100)); // edge color (publish mode)
  S->setValue("latdot/fill_col", QColor(0,0,0,0));        // fill color
  S->setValue("latdot/fill_col_pb", QColor(0,0,0,0));     // fill color (publish mode)
  S->setValue("latdot/inner_fill", .5);                   // inner fill factor
  S->setValue("latdot/inner_fill_col", QColor(255, 255, 0));  // inner colour
  S->setValue("latdot/inner_fill_col_pb", QColor(255, 255, 0));  // inner colour (publish mode)

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
  S->setValue("electrode/edge_col", QColor(60,60,60));        // edge color
  S->setValue("electrode/fill_col", QColor(100,100,100));     // fill color
  S->setValue("electrode/selected_col", QColor(0, 100, 255)); // edge color, selected

  // potplot parameters
  S->setValue("potplot/edge_width", .05);                   // edge width of box lines
  S->setValue("potplot/edge_col", QColor(60,60,60));        // edge color
  S->setValue("potplot/fill_col", QColor(100,100,100));     // fill color
  S->setValue("potplot/selected_col", QColor(0, 100, 255)); // edge color, selected

  // afm parameters
  S->setValue("afmarea/area_border_width", 5);
  S->setValue("afmarea/afm_border_normal", QColor(0,90,255,150));
  S->setValue("afmarea/afm_border_hovered", QColor(0,170,255,150));
  S->setValue("afmarea/afm_border_selected", QColor(130,90,255,150));
  S->setValue("afmarea/area_fill_normal", QColor(80,80,80,100));
  S->setValue("afmarea/area_fill_hovered", QColor(120,120,120,100));
  S->setValue("afmarea/area_fill_selected", QColor(50,50,50,100));
  S->setValue("afmarea/scan_path_width", 1);
  S->setValue("afmarea/scan_path_fill_normal", QColor(50,50,50,150));
  S->setValue("afmarea/scan_path_fill_hovered", QColor(80,80,80,150));
  S->setValue("afmarea/scan_path_fill_selected", QColor(150,150,150,150));
  S->setValue("afmnode/fill_col_default", QColor(0, 90, 255, 200));   // default fill color
  S->setValue("afmnode/fill_col_hovered", QColor(0, 170, 255, 200));  // hovered fill color
  S->setValue("afmnode/fill_col_sel", QColor(130, 90, 255, 200));     // selected fill color
  S->setValue("afmnode/bd_col_default", QColor(255, 255, 255, 120));  // default border color
  S->setValue("afmnode/bd_col_hovered", QColor(255, 255, 255, 120));  // hovered border color
  S->setValue("afmnode/bd_col_sel", QColor(255, 255, 255, 120));      // selected border color
  S->setValue("afmnode/diameter", .8);     // node diameter
  S->setValue("afmnode/edge_width", .2);  // node edge width
  S->setValue("afmseg/line_col_default", QColor(0, 255, 166, 80));
  S->setValue("afmseg/line_col_hovered", QColor(43, 255, 0, 80));
  S->setValue("afmseg/line_col_sel", QColor(0, 150, 110, 80));
  S->setValue("afmseg/line_width", 8);

  return S;
}

QSettings* settings::LatticeSettings::m_defs()
{
  qDebug("Loading default lattice settings...");
  // TODO remove QSettings *S = new QSettings("src/settings/defaults/lattices/si_100_2x1.ini", QSettings::IniFormat);
  QSettings *S = new QSettings(
      pathReplacement(QString("<CONFIG>/defaults/lattices/si_100_2x1.ini")),
      QSettings::IniFormat
  );

  S->setValue("cell/N", 2);

  S->setValue("cell/b1", QPointF(0, 0));
  S->setValue("cell/b2", QPointF(0, 2.4));

  S->setValue("lattice/a1", QPointF(3.84, 0));
  S->setValue("lattice/a2", QPointF(0, 7.68));


  return S;
}
