#include <QApplication>
#include <QMainWindow>
#include <QResource>

#include "gui/application.h"

int main(int argc, char **argv){

  QApplication app(argc, argv);

  // Attribute settings


  // Main Window
  gui::ApplicationGUI w;

  w.setWindowTitle("Dangling Bond Simulator");

  w.show();

  return app.exec();

}
