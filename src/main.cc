// @file:     main.cc
// @author:   Jake
// @created:  2016.10.31
// @editted:  2017.05.08  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Top level preamble and initialization of the ApplicationGUI.
//            Modify only if you know what you are doing.

#include <QApplication>
#include <QMainWindow>
#include <QResource>
#include <QDebug>

#include "gui/application.h"
#include "settings/settings.h"

#include <cstdlib>
#include <ctime>


// If the ApplicationGUI DialogPanel is defined, redirects the standard
// outputs to display in the panel (calls DialogPanel::echo).
static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  if(gui::ApplicationGUI::dialog_pan==0){
    QByteArray localMsg = msg.toLocal8Bit();
    switch(type){
      case QtDebugMsg:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
      case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
      case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
      case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
      case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
      }
  }
  else{
    switch(type){
      case QtDebugMsg:
        gui::ApplicationGUI::dialog_pan->echo(msg);
        break;
      case QtInfoMsg:
        gui::ApplicationGUI::dialog_pan->echo(msg);
        break;
      case QtWarningMsg:
        gui::ApplicationGUI::dialog_pan->echo("Warning: " + msg);
        break;
      case QtCriticalMsg:
        gui::ApplicationGUI::dialog_pan->echo("Critical: " + msg);
        break;
      case QtFatalMsg:
        gui::ApplicationGUI::dialog_pan->echo("Fatal error");
        abort();
    }
  }
}

int main(int argc, char **argv){
  // initialise rand
  srand(time(NULL));

  settings::AppSettings *app_settings = settings::AppSettings::instance();

  if(app_settings->get<bool>("view/hidpi_support"))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  // call the message handler only if the override flag is set
  // note that if the override is set and the log file is suppresed there will
  // be no way to get error messages if the app crashes.
  if(app_settings->get<bool>("log/override"))
    qInstallMessageHandler(messageHandler);
  else
    qDebug("Using default qdebug target");

  // initialise QApplication
  QApplication app(argc, argv);
  app.setApplicationVersion(APP_VERSION);

  // Main Window
  gui::ApplicationGUI w;

  w.setWindowTitle("Dangling Bond Simulator");
  w.show();

  return app.exec();

}
