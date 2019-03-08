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

    //! destructor
    ~SimEngine() {};

    // property map with simulator information
    gui::PropertyMap sim_params_map;  // a property map containing all of the simulation parameters

    // ACCESSORS

    //! Return the path to the engine description file.
    QString descriptionFilePath() {return eng_desc_path;}

    //! Set the engine description file path.
    void setDescriptionFilePath(const QString &p) {eng_desc_path = p;}

    //! Return the path to the saved user engine configuration file.
    //! TODO remove this after the completion of the preset feature.
    QString userConfigurationFilePath() {return eng_usr_cfg_path;}

    //! Return the path to the directory where user presets for this engine 
    //! should be saved.
    //! TODO implement
    QString userPresetDirectoryPath();

    //! Return the engine name.
    QString name() {return name;}

    //! Set the engine name.
    void setName(const QString &t_name) {name = t_name;}

    //! Return the engine version.
    QString version() {return version;}

    //! Set the engine version.
    void setVersion(const QString &ver) {version = ver;}

    //! Return the runtime interpreter of the engine (e.g. python).
    QString interpreter() {return interpreter;}

    //! Set the runtime interpreter of the engine (e.g. python).
    void setInterpreter(const QString &inter) {runtime_interpreter = inter;}

    //! Return the path to the binary (no interpreter) or script (interpreter 
    //! set) of the physics engine.
    QString binaryPath() {return bin_path;}

    //! Set the path to the binary (no interpreter) or script (interpreter set) 
    //! of the physics engine.
    void setBinaryPath(const QString &p) {bin_path = p;}

    //! Return the dependencies description (e.g. requirements.txt for Python).
    QString dependenciesPath() {return dep_path;}

    //! Set the dependencies description (e.g. requirements.txt for Python).
    void setDependenciesPath(const QString &p) {dep_path = p;}

    //! Return the temporary directory where this engine stores simulation 
    //! problems and results.
    QString runtimeTempPath();

  private:

    //! Initialize the simulation engine.
    void initSimEngine(const QString &eng_nm, const QString &eng_rt);

    // variables like binary location, temp file location, etc.
    QString eng_desc_path;      // description file of this engine
    QString eng_usr_cfg_path;   // user default settings of this engine TODO remove after preset feature
    QString name;               // name of this engine
    QString root_dir_path;      // root directory of this engine containing description and more
    QString preset_dir_path;    // path to the directory where user presets should be stored
    QString version;
    QString interpreter;        // runtime interpreter (e.g. Python), blank if not applicable
    QString bin_path;           // binary (standalone) or script (interpreted) path of this engine
    QString dep_path;           // dependencies path (requirements.txt for Python scripts)
    QString tmp_path;           // root directory for all problems files for this engine
  };

} // end of prim namespace

#endif
