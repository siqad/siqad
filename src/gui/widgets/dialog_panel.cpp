#include "dialog_panel.h"
#include "src/settings/settings.h"

#include <iostream>
#include <QDebug>
#include <QTextStream>
#include <QDir>


gui::DialogPanel::DialogPanel(QWidget *parent)
  : QPlainTextEdit(parent)
{
  settings::AppSettings app_settings;
  settings::GUISettings gui_settings;

  setMaximumHeight(gui_settings.get<int>("Panel/maxh"));
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  setReadOnly(true);
  setLineWrapMode(QPlainTextEdit::NoWrap);
  //setMinimumWidth(gui_settings.get<int>("Panel/logw", defaults::gui::panel_logw));

  // set up file if active
  file=0;
  if(app_settings.value("log/tofile").toBool()){
    file = new QFile(app_settings.get<QString>("log/logfile"));
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
  settings::AppSettings app_setings;

  // close log file if active
  if(file != 0){
    file->close();
    delete file;
    file=0;
  }

}


// Add given text as a new line in the dialog panel
void gui::DialogPanel::echo(const QString& s)
{
  settings::AppSettings app_settings;
  appendPlainText(s);

  // write to log file
  if(app_settings.get<bool>("log/tofile")){
    if(file!=0){
      QTextStream stream(file);
      stream << s << "\n";
    }
  }

}


void gui::DialogPanel::mousePressEvent(QMouseEvent *e)
{
}
