// @file:     sim_engine.cc
// @author:   Samuel
// @created:  2017.10.03
// @editted:  2017.10.03 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     SimEngine classes

#include "sim_engine.h"
#include <QUiLoader>

using namespace prim;

SimEngine::SimEngine(const QString &eng_desc_path, QWidget *parent)
  : QObject(parent), eng_desc_path(eng_desc_path)
{
  QFile eng_f(eng_desc_path);
  QFileInfo file_info(eng_desc_path);
  QString eng_rt = file_info.absolutePath();
  QString eng_nm;
  QDir eng_dir(eng_rt);

  if (!eng_f.open(QFile::ReadOnly | QFile::Text)) {
    qCritical() << tr("SimEngine: cannot open engine description file %1").arg(eng_f.fileName());
    return;
  }

  QXmlStreamReader rs(&eng_f);
  qDebug() << tr("Reading engine file from %1").arg(eng_f.fileName());

  QString read_eng_nm, read_eng_ver;
  QString read_interpreter, read_bin_path;

  // enter the root node
  rs.readNextStartElement();

  while (rs.readNextStartElement()) {
    // recognized XML elements
    if (rs.name() == "name") {
      eng_nm = rs.readElementText();
    } else if (rs.name() == "version") {
      setVersion(rs.readElementText());
    } else if (rs.name() == "bin_path") {
      setBinaryPath(eng_dir.absoluteFilePath(rs.readElementText()));
    } else if (rs.name() == "dep_path") {
      setDependenciesPath(eng_dir.absoluteFilePath(rs.readElementText()));
    } else if (rs.name() == "interpreter") {
      setInterpreter(rs.readElementText());
    } else if (rs.name() == "sim_params") {
      sim_params_map.readPropertiesFromXMLStream(&rs);
    } else {
      qDebug() << tr("SimEngine: invalid element encountered on line %1 - %2").arg(rs.lineNumber()).arg(rs.name().toString());
      rs.skipCurrentElement();
    }
  }

  eng_f.close();

  // store user engine settings path in case of need
  QString usr_cfg_dir_path = settings::AppSettings::instance()->getPath("phys/eng_usr_cfg_dir");
  usr_cfg_dir_path += file_info.dir().dirName();
  QDir usr_cfg_dir(usr_cfg_dir_path);
  eng_usr_cfg_path = usr_cfg_dir.absoluteFilePath(file_info.fileName());

  initSimEngine(eng_nm, eng_rt);
}

SimEngine::SimEngine(const QString &eng_nm, const QString &eng_rt, QWidget *parent)
  : QObject(parent)
{
  initSimEngine(eng_nm, eng_rt);
}

SimEngine::userPresetDirectoryPath()
{
  if (preset_dir_path.isEmpty()) {
    QDir preset_root_dir(settings::AppSettings::instance()->getPath("phys/preset_root_path"));
    if (!preset_root_dir.mkpath(".")) {
      qWarning() << tr("Unable to create preset root directory %1").arg(preset_root_dir.path());
      return;
    }
    if (!preset_root_dir.exists(name) && !preset_root_dir.mkdir(name)) {
      qWarning() << tr("Unable to create engine preset directory %1").arg(preset_root_dir.filePath(name));
      return;
    }
    
  }
  return preset_dir_path;
}

QString SimEngine::runtimeTempPath()
{
  if(tmp_path.isEmpty()){
    tmp_path = settings::AppSettings::instance()->getPath("phys/runtime_tmp_root_path");
  }
  return tmp_path;
}

void SimEngine::initSimEngine(const QString &eng_nm, const QString &eng_rt)
{
  name = eng_nm;
  root_dir_path = eng_rt;
}
