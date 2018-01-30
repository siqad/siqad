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


SimEngine::SimEngine(const QString &eng_nm, const QString &eng_root, QWidget *parent)
  : QObject(parent), eng_name(eng_nm), eng_root(eng_root)
{
  // readEngineDecl
  constructSimParamDialog();
}


bool SimEngine::readEngineDecl(QFile *in_f)
{
  // check desc path readability

  // start QXmlStreamReader

  // read simulator properties and desired parameters from simulator XML
  return true;
}


bool SimEngine::constructSimParamDialog()
{
  // check if file exists
  QDir eng_dir(eng_root);
  if (!eng_dir.exists("option_dialog.ui")) {
    qDebug() << tr("SimEngine: Skipping sim param dialog construction for %1, file not found.").arg(name());
    return false;
  }

  // check file readability
  QFile ui_file(eng_dir.absoluteFilePath("option_dialog.ui"));
  if (!ui_file.open(QFile::ReadOnly | QFile::Text)) {
    qCritical() << QObject::tr("SimEngine: cannot open UI file").arg(ui_file.fileName());
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


QString SimEngine::runtimeTempDir()
{
  if(runtime_temp_dir.isEmpty()){
    runtime_temp_dir = settings::AppSettings::instance()->getPath("phys/runtime_temp_dir");
  }
  return runtime_temp_dir;
}


} // end of prim namespace
