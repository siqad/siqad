// @file:     dialog_panel.cc
// @author:   Jake
// @created:  2016.11.02
// @editted:  2017.05.15  - Jake
// @license:  GNU LGPL v3
//
// @desc:     DialogPanel definitinos.


#include "dialog_panel.h"
#include "src/settings/settings.h"
#include <iostream>


gui::DialogPanel::DialogPanel(QWidget *parent)
  : QPlainTextEdit(parent)
{
  settings::AppSettings *app_settings = settings::AppSettings::instance();
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  setMaximumHeight(gui_settings->get<int>("Panel/maxh"));
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  setReadOnly(true);
  setLineWrapMode(QPlainTextEdit::NoWrap);

  // set up file if active
  file=0;
  if(app_settings->value("log/tofile").toBool()){
    file = new QFile(app_settings->get<QString>("log/logfile"));
    // create or clear log file in WriteOnly mode. If open fails, set file
    // pointer to NULL
    if(!file->exists())
      qDebug() << QString("Creating log file: %1").arg(file->fileName());
    if(!file->open(QIODevice::WriteOnly))
      file=0;
  }
}


gui::DialogPanel::~DialogPanel()
{
  // close log file if active
  if(file != 0){
    file->close();
    delete file;
    file=0;
  }
}


void gui::DialogPanel::echo(const QString& s)
{
  settings::AppSettings *app_settings = settings::AppSettings::instance();
  appendPlainText(s);

  // write to log file
  if(app_settings->get<bool>("log/tofile") && file != 0)
    QTextStream(file) << s << "\n";
}


void gui::DialogPanel::mousePressEvent(QMouseEvent *e)
{}
