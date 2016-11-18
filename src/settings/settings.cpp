#include "settings.h"

#include <QSize>
#include <QColor>
#include <QPoint>
#include <QString>

// initialize default settings
QSettings *settings::AppSettings::defs = settings::AppSettings::m_defs();
QSettings *settings::GUISettings::defs = settings::GUISettings::m_defs();
QSettings *settings::LatticeSettings::defs = settings::LatticeSettings::m_defs();


QSettings* settings::AppSettings::m_defs()
{
  qDebug("Loading default app settings...");
  QSettings *S = new QSettings("src/settings/defaults/app_settings.ini", QSettings::IniFormat);

  // overwrites existing default values with same keys... no check

  S->setValue("log/override", true);
  S->setValue("log/tofile", false);
  S->setValue("log/logfile", QString("src/log/log.txt"));

  return S;
}

QSettings* settings::GUISettings::m_defs()
{
  qDebug("Loading default gui settings...");
  QSettings *S = new QSettings("src/settings/defaults/gui_settings.ini", QSettings::IniFormat);

  S->setValue("MWIN/size", QSize(1400, 800));
  S->setValue("TBAR/mh", 60);
  S->setValue("SBAR/mw", 60);
  S->setValue("SBAR/loc", 1);
  S->setValue("Panel/logw", 400);
  S->setValue("Panel/maxh", 150);

  S->setValue("lattice/fname", QString("src/settings/lattices/si_100_2x1.ini"));
  S->setValue("lattice/xy", QPoint(20,20));

  // QGraphicsView
  S->setValue("view/bg_col", QColor(40,50,60));
  S->setValue("view/zoom_factor", 0.1);
  S->setValue("view/zoom_boost", 2);    // must have factor*boost < 1
  S->setValue("view/zoom_min", .1);     // minimum scale factor
  S->setValue("view/zoom_max", 10);     // maximum scale factor
  S->setValue("view/wheel_pan_step", 20);
  S->setValue("view/wheel_pan_boost", 5);

  // dangling bond
  S->setValue("dbdot/scale_fact", 10);  // fixed: pixels per angstrom for dot locations
  S->setValue("dbdot/diameter", 1.3);    // diameter relative to scale_fact

  S->setValue("dbdot/edge_width", .1);  // relative pen width
  S->setValue("dbdot/edge_col", QColor(255,255,255));

  S->setValue("dbdot/fill_col", QColor(200,200,200));

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
