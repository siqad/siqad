// @file:     plugin_manager.cc
// @author:   Samuel
// @created:  2019.03.13
// @license:  GNU LGPL v3
//
// @desc:     Widget for loading and managing plugins.

#include "plugin_manager.h"
#include "settings/settings.h"

namespace gui{

QString python_path;

PluginManager::PluginManager(QWidget *parent)
  : QWidget(parent, Qt::Dialog)
{
  if (gui::python_path.isEmpty())
    initPythonPath();
  initServiceTypes();
  initPluginEngines();
}

PluginManager::~PluginManager()
{
  for (comp::PluginEngine *engine : plugin_engines)
    delete engine;
  plugin_engines.clear();

  // TODO delete all plugin jobs
}


// PRIVATE

void PluginManager::initPythonPath()
{
  // NOTE dropped in from SimManager implementation, TODO improve (e.g. virualenv, docker, etc.)
  QString s_py = settings::AppSettings::instance()->get<QString>("user_python_path");

  if (!s_py.isEmpty()) {
    gui::python_path = s_py;
    qDebug() << tr("Python path retrieved from user settings: %1").arg(gui::python_path);
  } else {
    if (!findWorkingPythonPath())
      qWarning() << "No Python 3 interpreter found. Please set it in the settings dialog.";
  }
}

bool PluginManager::findWorkingPythonPath()
{
  // NOTE dropped in from SimManager implementation, TODO improve
  QStringList test_py_paths;
  QString kernel_type = QSysInfo::kernelType();
  auto get_py_paths = [](const QString &os) -> QStringList {
    return settings::AppSettings::instance()->getPaths("python_search_"+os);
  };
  if (kernel_type == "linux" || kernel_type == "freebsd") {
    test_py_paths << get_py_paths("linux");
  } else if (kernel_type == "winnt") {
    test_py_paths << get_py_paths("winnt");
  } else if (kernel_type == "darwin") {
    test_py_paths << get_py_paths("darwin");
  } else {
    qWarning() << tr("No Python search path defined for your kernel type %1. Please enter your Python binary path in the Settings dialog and restart the application.").arg(kernel_type);
    return false;
  }

  QString test_script = QDir(QCoreApplication::applicationDirPath()).filePath("phys/is_python3.py");
  if (!QFile::exists(test_script)) {
    qDebug() << tr("Python version test script %1 not found").arg(test_script);
    return false;
  }


  for (QString test_py_path : test_py_paths) {
    QStringList splitted_path = test_py_path.split(',');
    if (splitted_path.size() == 0)
      continue;

    // set up command and arguments
    QString command = splitted_path.at(0);
    QStringList args = splitted_path.mid(1);
    args << test_script;

    QString output;
    QProcess *py_process = new QProcess(this);
    //py_process->start(test_py_path, {test_script});
    py_process->start(command, args);
    py_process->waitForStarted(1000);

    // run the test script
    while(py_process->waitForReadyRead(1000))
      output.append(QString::fromStdString(py_process->readAll().toStdString()));
    
    if (output.contains("Python3 Interpretor Found")) {
      gui::python_path = test_py_path;
      qDebug() << tr("Python path found: %1").arg(gui::python_path);
      return true;
    } else {
      qDebug() << tr("Python path %1 is invalid. Output: %2").arg(test_py_path).arg(output);
    }
  }

  return false;
}

void PluginManager::initServiceTypes()
{
  // initialize service types
  QFile services_file(":/plugin_services.xml");

  if (!services_file.open(QFile::ReadOnly)) {
    qCritical() << tr("Failed to open service list file: %1")
      .arg(services_file.fileName());
    return;
  }

  QXmlStreamReader rs(&services_file);

  // enter the XML root node
  rs.readNextStartElement();

  auto unrecognizedXMLElement = [](QXmlStreamReader &rs) mutable
  {
    qWarning() << tr("Invalid element encountered on line %1 - %2")
      .arg(rs.lineNumber()).arg(rs.name().toString());
    rs.skipCurrentElement();
  };

  // read service list
  while (rs.readNextStartElement()) {
    if (rs.name() != "service") {
      unrecognizedXMLElement(rs);
      rs.skipCurrentElement();
      continue;
    }
    // read service details
    comp::PluginEngine::Service service;
    while (rs.readNextStartElement()) {
      if (rs.name() == "name") {
        service.name = rs.readElementText();
      } else if (rs.name() == "category") {
        service.category = rs.readElementText();
      } else if (rs.name() == "label") {
        service.label = rs.readElementText();
      } else {
        unrecognizedXMLElement(rs);
      }
    }
    if (!service.name.isEmpty()) {
      comp::PluginEngine::official_services.append(service);
    }
  }
}

void PluginManager::initPluginEngines()
{
  // initialize engines
  QStringList eng_lib_dir_paths = settings::AppSettings::instance()->getPaths("phys/eng_lib_dirs");

  // go through all possible plugin locations
  for (QString eng_lib_dir_path : eng_lib_dir_paths) {
    QDir eng_lib_dir(eng_lib_dir_path);
    if (!eng_lib_dir.exists()) {
      qDebug() << tr("Engine lib path does not exist, ignored: %1").arg(eng_lib_dir_path);
      continue;
    }
    qDebug() << tr("Engine lib path found: %1").arg(eng_lib_dir_path);
    QStringList engine_dir_paths = eng_lib_dir.entryList(QStringList({"*"}),
        QDir::AllDirs | QDir::NoDotAndDotDot);

    // find all existing engines in the engine library
    QStringList eng_dec_paths;
    QStringList eng_filter(QStringList() << "*.physeng" << "*.sqplug");
    for (QString engine_dir_path : engine_dir_paths) {
      qDebug() << tr("Checking %1 for engine description file").arg(engine_dir_path);
      QDir eng_dir(eng_lib_dir.filePath(engine_dir_path));
      QStringList matched_eng_files = eng_dir.entryList(eng_filter, QDir::Files);

      // add engine declaration files to a list
      for (QString matched_eng_file : matched_eng_files) {
        eng_dec_paths << eng_dir.absoluteFilePath(matched_eng_file);
        qDebug() << tr("Found engine file: %1").arg(eng_dec_paths.back());
      }
    }

    // import engines corresponding to the list of declaration files
    for (QString eng_dec_path : eng_dec_paths) {
      comp::PluginEngine *eng = new comp::PluginEngine(eng_dec_path);
      plugin_engines.insert(eng->uniqueIdentifier(), eng);
    }
  }

  qDebug() << tr("Finished reading plugin files.");
}

}
