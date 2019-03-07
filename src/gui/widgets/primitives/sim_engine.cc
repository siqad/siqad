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
  root_dir_path = file_info.absolutePath();
  QDir eng_dir(root_dir_path);

  if (!eng_f.open(QFile::ReadOnly | QFile::Text)) {
    qCritical() << tr("SimEngine: cannot open engine description file %1").arg(eng_f.fileName());
    return;
  }

  QXmlStreamReader rs(&eng_f);
  qDebug() << tr("Reading engine file from %1").arg(eng_f.fileName());

  // enter the root node
  rs.readNextStartElement();

  auto unrecognizedElement = [](QXmlStreamReader &rs) mutable
  {
    qWarning() << tr("SimEngine: invalid element encountered on line %1 - %2")
      .arg(rs.lineNumber()).arg(rs.name().toString());
    rs.skipCurrentElement();
  };

  while (rs.readNextStartElement()) {
    // recognized XML elements
    if (rs.name() == "name") {
      setName(rs.readElementText());
    } else if (rs.name() == "version") {
      setVersion(rs.readElementText());
    } else if (rs.name() == "bin_path") {
      setBinaryPath(eng_dir.absoluteFilePath(rs.readElementText()));
    } else if (rs.name() == "dep_path") {
      setDependenciesPath(eng_dir.absoluteFilePath(rs.readElementText()));
    } else if (rs.name() == "commands") {
      while (rs.readNextStartElement()) {
        if (rs.name() == "command") {
          QString cmd_label = rs.attributes().value("label").toString();
          QStringList cmd_list;
          while (rs.readNextStartElement()) {
            if (rs.name() == "program" || rs.name() == "arg") {
              cmd_list.append(rs.readElementText());
            } else {
              unrecognizedElement(rs);
            }
          }
          eng_command_formats.append(qMakePair(cmd_label, cmd_list));
        } else {
          unrecognizedElement(rs);
        }
      }
    } else if (rs.name() == "sim_params") {
      sim_params_map.readPropertiesFromXMLStream(&rs);
    } else {
      unrecognizedElement(rs);
    }
  }

  eng_f.close();

  // store user engine settings path in case of need
  QString usr_cfg_dir_path = settings::AppSettings::instance()->getPath("phys/eng_usr_cfg_dir");
  usr_cfg_dir_path += file_info.dir().dirName();
  QDir usr_cfg_dir(usr_cfg_dir_path);
  eng_usr_cfg_path = usr_cfg_dir.absoluteFilePath(file_info.fileName());
}

QString SimEngine::userPresetDirectoryPath()
{
  // initialize path if it doesn't already exist
  if (preset_dir_path.isEmpty()) {
    QDir preset_root_dir(settings::AppSettings::instance()->getPath("phys/preset_root_path"));
    QDir eng_preset_dir(preset_root_dir.filePath(name()));
    if (!eng_preset_dir.mkpath(".")) {
      qWarning() << tr("Unable to create engine preset directory %1").arg(eng_preset_dir.path());
      return QString();
    }
    preset_dir_path = eng_preset_dir.path();
  }
  return preset_dir_path;
}
