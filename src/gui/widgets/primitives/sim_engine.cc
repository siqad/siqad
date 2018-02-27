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
  qDebug() << tr("Reading engine library from %1").arg(eng_f.fileName());

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
            while (!(rs.isEndElement() && rs.name() == "sim_params")) {
              if (!rs.readNextStartElement())
                continue;

              if (rs.name() == "param") {
                // add expectedSimParams entry
                QString gui_obj_nm, gui_obj_type, gui_def_txt, param_nm;
                for (QXmlStreamAttribute &attr : rs.attributes()) {
                  if (attr.name().toString() == QLatin1String("gui_object_name")) {
                    gui_obj_nm = attr.value().toString();
                  } else if (attr.name().toString() == QLatin1String("gui_object_type")) {
                    gui_obj_type = attr.value().toString();
                  } else if (attr.name().toString() == QLatin1String("default_text")) {
                    gui_def_txt = attr.value().toString();
                  }
                }
                param_nm = rs.readElementText();
                addExpectedSimParam(param_nm, gui_obj_nm, gui_obj_type, gui_def_txt);
                qDebug() << QObject::tr("addExpectedSimParam(%1, %2, %3, %4)").arg(param_nm).arg(gui_obj_nm).arg(gui_obj_type).arg(gui_def_txt);
              }
            } // end of sim_params
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

  constructSimParamDialog();
}

bool SimEngine::constructSimParamDialog()
{
  // check if file exists
  QDir eng_dir(eng_root);
  if (param_dialog_path.isEmpty())
    param_dialog_path = eng_dir.absoluteFilePath("option_dialog.ui");

  // check file readability
  QFile ui_file(param_dialog_path);
  if (!ui_file.open(QFile::ReadOnly | QFile::Text)) {
    qCritical() << QObject::tr("SimEngine: cannot open UI file %1").arg(ui_file.fileName());
    return false;
  }

  if (simParamDialog()) {
    delete sim_param_dialog;
    sim_param_dialog = 0;
  }

  // use QUiLoader to load the ui
  QUiLoader ui_loader;
  sim_param_dialog = ui_loader.load(&ui_file); // TODO in sim manager, set itself as the parent of this dialog
  ui_file.close();

  return true;
}

QList<QPair<QString, QString>> SimEngine::loadSimParamsFromDialog()
{
  QList<QPair<QString, QString>> retrieved_sim_params;
  if (!simParamDialog()) {
    qDebug() << QObject::tr("Engine %1 doesn't have its own simulation parameter widget, skipping load.").arg(name());
    return retrieved_sim_params;
  }

  // acquire all expected simulation parameters
  for (ExpectedSimParam param : expected_sim_params) {
    QWidget *find_widget = simParamDialog()->findChild<QWidget*>(param.gui_object_name);
    if (param.gui_object_type == "QLineEdit") {
      retrieved_sim_params.append(QPair<QString, QString>(param.name, static_cast<QLineEdit*>(find_widget)->text()));
      qDebug() << QObject::tr("Added sim param %1 with content %2").arg(param.name).arg(static_cast<QLineEdit*>(find_widget)->text());
    } else if (param.gui_object_type == "QComboBox") {
      retrieved_sim_params.append(QPair<QString, QString>(param.name, static_cast<QComboBox*>(find_widget)->currentText()));
      qDebug() << QObject::tr("Added sim param %1 with content %2").arg(param.name).arg(static_cast<QComboBox*>(find_widget)->currentText());
    }
    // TODO radio buttons
  }

  return retrieved_sim_params;
}


QString SimEngine::runtimeTempDir()
{
  if(runtime_temp_dir.isEmpty()){
    runtime_temp_dir = settings::AppSettings::instance()->getPath("phys/runtime_temp_dir");
  }
  return runtime_temp_dir;
}


} // end of prim namespace
