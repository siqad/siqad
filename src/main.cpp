#include <QApplication>
#include <QMainWindow>
#include <QResource>

#include "gui/application.h"


static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  if(gui::ApplicationGUI::dialog_wg==0){
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
        gui::ApplicationGUI::dialog_wg->echo(msg);
        break;
      case QtInfoMsg:
        gui::ApplicationGUI::dialog_wg->echo(msg);
        break;
      case QtWarningMsg:
        gui::ApplicationGUI::dialog_wg->echo("Warning: " + msg);
        break;
      case QtCriticalMsg:
        gui::ApplicationGUI::dialog_wg->echo("Critical: " + msg);
        break;
      case QtFatalMsg:
        gui::ApplicationGUI::dialog_wg->echo("Fatal error");
        abort();
    }
  }
}

int main(int argc, char **argv){

  qInstallMessageHandler(messageHandler);
  
  QApplication app(argc, argv);

  // Main Window
  gui::ApplicationGUI w;

  w.setWindowTitle("Dangling Bond Simulator");

  w.show();

  return app.exec();

}
