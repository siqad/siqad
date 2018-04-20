// @file:     sim_engine.cc
// @author:   Samuel
// @created:  2017.10.03
// @editted:  2017.10.03 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     SimEngine classes

#include "sim_engine.h"
#include <QUiLoader>

namespace prim{


SimEngine::SimEngine(const QString &eng_desc_path, QWidget *parent)
  : QObject(parent)
{
  QString eng_nm, eng_rt;

  QFile eng_f(eng_desc_path);
  QFileInfo fileInfo(eng_desc_path);
  eng_rt = fileInfo.absolutePath();
  QDir eng_dir(eng_rt);

  if (!eng_f.open(QFile::ReadOnly | QFile::Text)) {
    qCritical() << tr("SimEngine: cannot open engine description file %1").arg(eng_f.fileName());
    return;
  }

  QXmlStreamReader rs(&eng_f);
  qDebug() << tr("Reading engine file from %1").arg(eng_f.fileName());

  QString read_eng_nm, read_eng_ver;
  QString read_interpreter, read_bin_path, read_linked_script_path;
  while (!rs.atEnd()) {
    if (rs.isStartElement()) {
      if (rs.name() == "physeng") {
        while (!(rs.isEndElement() && rs.name() == "physeng")) {
          if (!rs.readNextStartElement())
            continue; // skip until a start element is encountered

          if (rs.name() == "name") {
            eng_nm = rs.readElementText();
          } else if (rs.name() == "version") {
            setVersion(rs.readElementText());
          } else if (rs.name() == "bin_path") {
            setBinaryPath(eng_dir.absoluteFilePath(rs.readElementText()));
          } else if (rs.name() == "interpreter") {
            setRuntimeInterpreter(rs.readElementText());
          } else if (rs.name() == "linked_script_path") {
            setLinkedScriptPath(eng_dir.absoluteFilePath(rs.readElementText()));
          } else if (rs.name() == "gui_dialog_path") {
            setParamDialogPath(eng_dir.absoluteFilePath(rs.readElementText()));
          } else if (rs.name() == "sim_params") {
            sim_params_map.readPropertiesFromXMLStream(&rs);
          }
        } // end of physeng
        rs.readNext();
      } else {
        qDebug() << tr("SimEngine: invalid element encountered on line %1 - %2").arg(rs.lineNumber()).arg(rs.name().toString());
      }
    } else {
      rs.readNext();
    }
  }
  eng_f.close();

  initSimEngine(eng_nm, eng_rt);
}

SimEngine::SimEngine(const QString &eng_nm, const QString &eng_rt, QWidget *parent)
  : QObject(parent)
{
  initSimEngine(eng_nm, eng_rt);
}

void SimEngine::initSimEngine(const QString &eng_nm, const QString &eng_rt)
{
  eng_name = eng_nm;
  eng_root = eng_rt;
}

QString SimEngine::runtimeTempDir()
{
  if(runtime_temp_dir.isEmpty()){
    runtime_temp_dir = settings::AppSettings::instance()->getPath("phys/runtime_temp_dir");
  }
  return runtime_temp_dir;
}


} // end of prim namespace
