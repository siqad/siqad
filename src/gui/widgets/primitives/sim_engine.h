// @file:     sim_engine.h
// @author:   Samuel
// @created:  2017.10.03
// @editted:  2017.10.03 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     SimEngine object that SimManager interacts with

#ifndef _PRIM_SIM_ENG_H_
#define _PRIM_SIM_ENG_H_

#include <QtWidgets>
#include <QtCore>
#include <QXmlStreamReader>

#include "src/settings/settings.h"
#include "src/gui/property_map.h"

namespace prim{

  //! A simulation engine that can be used with the design tool.
  class SimEngine : public QObject
  {
    Q_OBJECT
  public:
    //! constructor, uses engine_description.xml
    SimEngine(const QString &eng_desc_path, QWidget *parent=0);
    //! constructor, uses an engine name and root path
    SimEngine(const QString &eng_nm, const QString &eng_rt, QWidget *parent=0);

    //initializer
    void initSimEngine(const QString &eng_nm, const QString &eng_rt);

    //! destructor
    ~SimEngine() {};

    //! read sim param dialog from provided *.ui file (xml)
    bool constructSimParamDialog();
    //! loads the simulation parameters entered in the SimManager dialog.
    QList<QPair<QString, QString>> loadSimParamsFromDialog();

    //! types of sim params expected for this engine
    struct ExpectedSimParam {
      ExpectedSimParam(const QString &param_nm, const QString &g_obj_nm, const QString &g_obj_type, const QString &g_def_txt = QString())
        : name(param_nm), gui_object_name(g_obj_nm), gui_object_type(g_obj_type), gui_default_text(g_def_txt)
      {
      }
      QString name;       // name of the parameter
      QString gui_object_name;  // name of field element to read
      QString gui_object_type;  // type of field element for casting (only support QLineEdit so far)
      QString gui_default_text; // default text to appear in the element TODO remove if not needed
    };
    //! Add an expected simulation parameter, based on the option_dialog.ui file.
    void addExpectedSimParam(const QString &param_nm, const QString &g_obj_nm, const QString &g_obj_type, const QString &g_def_txt) {expected_sim_params.append(ExpectedSimParam(param_nm, g_obj_nm, g_obj_type, g_def_txt));}
    //! Get a list of the expected simulation parameters.
    QList<ExpectedSimParam> *expectedSimParams() {return &expected_sim_params;}
    gui::PropertyMap sim_params_map;  // a property map containing all of the simulation parameters

    // ACCESSORS
    QString name() {return eng_name;}
    void setName(const QString &nm) {eng_name = nm;}
    QString version() {return eng_version;}
    void setVersion(const QString &ver) {eng_version = ver;}
    QString runtimeInterpreter() {return runtime_interpreter;}
    void setRuntimeInterpreter(const QString &inter) {runtime_interpreter = inter;}
    QString binaryPath() {return bin_path;}
    void setBinaryPath(const QString &path) {bin_path = path;}
    QString runtimeTempDir();
    void setLinkedScriptPath(const QString &path) {linked_script_path = path;}
    QString linkedScriptPath() {return linked_script_path;}
    QString paramDialogPath() {return param_dialog_path;}
    void setParamDialogPath(const QString &path) {param_dialog_path = path;}

    // gui
    QWidget *simParamDialog() {return sim_param_dialog;}

    // simulator info, for showing up in manager
    // available parameters and associated type, for user alteration


  private:
    // variables like binary location, temp file location, etc.
    QString eng_desc_file;      // description file of this engine
    QString eng_name;           // name of this engine
    QString eng_root;           // root directory of this engine containing description and more
    QString eng_version;
    QString runtime_interpreter;// runtime interpreter (e.g. Python), blank if not applicable
    QString bin_path;           // binary (standalone) or script (interpreted) path of this engine
    QString linked_script_path; // if the binary is a connector that calls another script, indicate the script here
    QString param_dialog_path;  // path to the gui dialog, defaults to "option_dialog.ui"
    QString runtime_temp_dir;   // root directory for all problems files for this engine

    // GUI
    QWidget *sim_param_dialog=0;// gui for setting sim params for this engine, loaded from *.ui
    QList<ExpectedSimParam> expected_sim_params;


    // TODO some stack/dictionary/etc with simulator info, for showing up in manager
    // TODO something that stores default parameters, associated types (so the appropriate fields are used), for user alteration
  };

} // end of prim namespace

#endif
