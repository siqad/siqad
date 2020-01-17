// @file:     dialog_panel.cc
// @author:   Jake
// @created:  2016.11.02
// @editted:  2017.07.11  - Jake
// @license:  GNU LGPL v3
//
// @desc:     DialogPanel definitinos.


#include "dialog_panel.h"
#include "settings/settings.h"
#include <iostream>


gui::DialogPanel::DialogPanel(QWidget *parent)
  : QPlainTextEdit(parent)
{
  settings::AppSettings *app_settings = settings::AppSettings::instance();

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  setReadOnly(true);
  setLineWrapMode(QPlainTextEdit::WidgetWidth);
  setTextInteractionFlags(textInteractionFlags() 
                          | Qt::TextSelectableByKeyboard);

  // show message if this dialog isn't set to capture debug outputs
  if (!app_settings->get<bool>("log/override")) {
    appendPlainText("This dialog is currently inactive. If you would like "
                    "debugging messages to be shown, go to Tools -> Settings "
                    "and enable \"Show debug messages\".");
    return;
  }

  // set up file if active
  if (app_settings->get<bool>("log/tofile")){
    log_dir.setPath(app_settings->getPath("log/logdir"));
    if (!log_dir.exists()) {
      if (log_dir.mkpath(".")) {
        qDebug() << tr("Successfully created log file directory: %1")
          .arg(log_dir.absolutePath());
      } else {
        qWarning()
          << tr("Failed to create log file directory at %1").arg(log_dir.absolutePath())
          << endl << "This SiQAD session will not be logged on file.";
        return;
      }
    }

    QString datetime = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
    qint64 tag = QCoreApplication::applicationPid();
    filename = tr("log-%1-instance-%2.log").arg(datetime).arg(tag);
    file = new QFile(log_dir.absoluteFilePath(filename));
    // create or clear log file in WriteOnly mode. If open fails, set file
    // pointer to NULL
    if (!file->exists())
      qDebug() << tr("Creating log file: %1").arg(file->fileName());

    if (!file->open(QIODevice::WriteOnly)) {
      qWarning() << tr("Unable to create log file: %1").arg(file->errorString());
      file = nullptr;
      return;
    }

    QTextStream(file) << "Beginning SiQAD log." << endl;
  }

  purgeOldLogs();
}


gui::DialogPanel::~DialogPanel()
{
  // close log file if active
  if(file != nullptr){
    file->close();
    log_dir.rename(filename, "closed-"+filename);
    delete file;
    file=nullptr;
  }
}


void gui::DialogPanel::echo(const QString& s)
{
  settings::AppSettings *app_settings = settings::AppSettings::instance();
  appendPlainText(s);

  // write to log file
  if (app_settings->get<bool>("log/tofile") && file != nullptr)
    QTextStream(file) << s << "\n";
}


void gui::DialogPanel::purgeOldLogs()
{
  if (!log_dir.exists())
    return;

  settings::AppSettings *app_settings = settings::AppSettings::instance();
  int keep_count = app_settings->get<int>("log/keepcount");

  // get list of closed logs sorted by modification time
  QStringList closed_logs = log_dir.entryList(QStringList({"*.log"}),
      QDir::NoFilter, QDir::Time|QDir::Reversed);

  // remove closed logs until the file count is below the desired count
  while (closed_logs.length() >= keep_count && closed_logs.length() != 0) {
    qDebug() << tr("Removing closed log %1").arg(closed_logs[0]);
    if (!log_dir.remove(closed_logs[0])) {
      qWarning() << tr("Unable to remove closed log %1. Will not attempt \
          further removals.").arg(closed_logs[0]);
      break;
    }
    closed_logs.removeAt(0);
  }
}
