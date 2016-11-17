#include "settings.h"

#include <QSize>
#include <QColor>
#include <QPoint>

// initialize default settings
QSettings *settings::AppSettings::defs = settings::AppSettings::m_defs();
QSettings *settings::GUISettings::defs = settings::GUISettings::m_defs();
QSettings *settings::LatticeSettings::defs = settings::LatticeSettings::m_defs();


QSettings* settings::AppSettings::m_defs()
{
  qDebug("Loading defaults app settings...");
  QSettings *S = new QSettings("src/settings/defaults/app_settings.ini", QSettings::IniFormat);

  // overwrites existing default values with same keys... no check

  S->setValue("log/override", true);
  S->setValue("log/tofile", false);
  S->setValue("log/logfile", QString("src/log/log.txt"));

  return S;
}

QSettings* settings::GUISettings::m_defs()
{
  qDebug("Loading defaults gui settings...");
  QSettings *S = new QSettings("src/settings/defaults/gui_settings.ini", QSettings::IniFormat);

  S->setValue("MWIN/size", QSize(1400, 800));
  S->setValue("TBAR/mh", 60);
  S->setValue("SBAR/mw", 60);
  S->setValue("SBAR/loc", 1);
  S->setValue("Panel/logw", 400);
  S->setValue("Panel/maxh", 150);

  S->setValue("LATTICE/fname", QString("src/settings/lattices/si_100_2x1.ini"));
  S->setValue("LATTICE/xy", QPoint(10,10));

  // colors
  S->setValue("view/bg_col", QColor(0,0,0));

  return S;
}

QSettings* settings::LatticeSettings::m_defs()
{
  qDebug("Loading defaults lattice settings...");
  QSettings *S = new QSettings("src/settings/defaults/lattices/si_100_2x1.ini", QSettings::IniFormat);

  S->setValue("cell/N", 2);

  S->setValue("cell/b1", QPointF(0, 0));
  S->setValue("cell/b2", QPointF(2.4, 0));

  S->setValue("lattice/a1", QPointF(7.68, 0));
  S->setValue("lattice/a2", QPointF(0, 3.84));

  return S;
}
