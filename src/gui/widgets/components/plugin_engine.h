// @file:     plugin_engine.h
// @author:   Samuel
// @created:  2019.03.13
// @license:  GNU LGPL v3
//
// @desc:     Plugin engine object storing information related to this plugin.

#ifndef _COMP_PLUG_ENG_H_
#define _COMP_PLUG_ENG_H_

#include <QtWidgets>
#include <QtCore>
#include <QXmlStreamReader>
#include <QMetaEnum>

#include "global.h"
#include "gui/property_map.h"

namespace comp{

  class PluginEngine : public QObject
  {
    Q_OBJECT

  public:

    struct Service
    {
      QString name;
      QString category;
      QString label;
    };

    struct Institution
    {
      QStringList content_lines;
      QString website_text;
      QString url;
    };

    struct Link
    {
      QString display_text;
      QString url;
    };

    //! Data that can be requested from SiQAD by plugins.
    //! TODO might not really need this, it isn't too much of an overhead to 
    //! just always save everything like before.
    enum RequestableDataset{All, DBDots, Electrodes};
    Q_ENUM(RequestableDataset);

    //! Data that can be returned to SiQAD from plugins.
    enum ReturnableDataset{Commands, ElectronConfigs, PotentialAnimation, 
      PotentialPlot};
    Q_ENUM(ReturnableDataset);

    //! Custom roles of the standard items returned by this class (e.g. for 
    //! showing plugin engines in tables). Don't use the entry "Ignore".
    enum CustomStandardItemRole{
      Ignore=Qt::UserRole,
      EnginePropertyFieldRole
    };

    //! Available standard item fields that plugin engines can return (e.g. for 
    //! showing plugin engines in tables).
    enum StandardItemField{UniqueIdentifierField, ServicesField, NameField, 
      VersionField, RootPathField, BinaryPathField, DependenciesPathField,
      DescriptionPathField, UserPresetDirectoryPathField};

    //! Constructor taking in the description file path to this public.
    PluginEngine(const QString &desc_file_path, QWidget *parent=nullptr);

    //! Destructor.
    ~PluginEngine() {};

    //! Initialize virtualenv and install requirements if needed.
    void prepareVirtualenv();

    //! Return the current plugin status in text.
    QString pluginStatusStr();

    QString getLogoPath() const { return plugin_logo_path; }

    //! Return the author list.
    QStringList getAuthors() const { return authors; }

    //! Return the institution list.
    QList<Institution> getInstitutions() const { return institutions; }

    //! Return the link list.
    QList<Link> getLinks() const { return links; }

    //! Return a list of standard items representing a row of engine properties.
    //! The fields variable is a list indicating which fields are wanted. If an 
    //! empty list is received, all possible fields are returned.
    //! It is the caller's responsibility to clean up the pointers after use.
    QList<QStandardItem *> standardItemRow(QList<StandardItemField> fields=QList<StandardItemField>()) const;

    //! Return a property map containing the default runtime properties of this 
    //! plugin.
    gui::PropertyMap defaultPropertyMap() const {return default_prop_map;}

    //! Return the available plugin invocation command formats as a QList of
    //! QPair. The first element of each pair is a descriptive command label and
    //! the second element is the actual command.
    QList<QPair<QString, QStringList>> commandFormats() const {return command_formats;}

    //! Return the command format corresponding to the given index. The first 
    //! element is the label and the second element is a string representing the
    //! command with arguments delimited by an optional delimiter specification 
    //! (if not specified, by default "\n").
    QPair<QString, QString> jointCommandFormat(int i, QString delim="\n") const
    {
      return qMakePair(command_formats.at(i).first, 
                       command_formats.at(i).second.join(delim));
    }

    //! Return a QStringList of services provided by this plugin.
    //! TODO figure out a way to standardize services.
    QStringList services() const {return plugin_services;}

    //! Return the unique identifier for this engine. The identifier is a qHash 
    //! of the engine name combined with the engine description file path. The 
    //! hash is different for each invocation.
    uint uniqueIdentifier() const {return unique_identifier;}

    //! Return the plugin name.
    QString name() const {return plugin_name;}

    //! Return the use virtualenv bool.
    bool useVirtualenv() {return py_use_virtualenv;}

    //! Return the virtual environment path.
    QString virtualenvPath();

    //! Return the Python interpreter in the virtual environment.
    //! Empty string if not set.
    QString pythonBin();

    //! Return the plugin version.
    QString version() const {return plugin_version;}

    //! Return the path to the plugin root directory.
    QString pluginRootPath() const {return plugin_root_path;}

    //! Return the path to the binary executable or the script (depending on 
    //! whether this engine is compiled or ran by an interpreter).
    QString binaryPath() const {return bin_path;}

    //! Return the dependencies description path (e.g. requirements.txt for 
    //! Python).
    QString dependenciesFilePath() const {return dep_path;}

    //! Return the path to the plugin description file.
    QString descriptionFilePath() const {return desc_file_path;}

    //! Return the path to the directory where user presets for this plugin 
    //! should be saved.
    QString userPresetDirectoryPath() const {return preset_dir_path;}

    //! Static variable containing all officially supported services. Services 
    //! that aren't on this list are binned under "Custom" in filters and lists.
    static QList<Service> official_services;

    //! Return a QLabel which reflects the venv init status.
    QLabel *widgetVenvStatus() {return l_venv_status;}

    //! Return a QPushButton which creates a pop-up box showing the venv init
    //! log when pressed.
    QPushButton *widgetVenvInitLog();

    //! Return whether the plugin engine is ready to use. For now, this is 
    //! only relevant when venv is needed; compiled plugins are always assumed 
    //! to be ready.
    bool readyToUse() {return ready_to_use;}


  private:

    // default runtime properties
    gui::PropertyMap default_prop_map;

    // preset invocation command formats
    QList<QPair<QString, QStringList>> command_formats;

    // datasets that can be returned by this plugin engine
    QSet<ReturnableDataset> returnable_datasets;

    uint unique_identifier;       // a unique identifier for this engine
    bool py_use_virtualenv;       // use virtualenv for python scripts
    bool venv_use_system_site;    // use system site packages for venv
    QString plugin_name;          // plugin name
    QString plugin_version;       // plugin version
    QStringList plugin_services;  // plugin service types
    QString plugin_root_path;     // plugin root path
    QString bin_path;             // binary/script path
    QString dep_path;             // dependencies path
    QString desc_file_path;       // description file path (normally *.sqplug)
    QString preset_dir_path;      // user configuration directory path
    QString plugin_logo_path;     // plugin logo path
    QStringList authors;          // list of authors
    QList<Institution> institutions;  // list of institutions
    QList<Link> links;            // list of relevant links

    bool ready_to_use;            // holds whether the plugin is ready to use
    bool venv_init_success;       // holds whether venv initialization was successful
    QString venv_init_stdout;     // std out from virtualenv initialization
    QString venv_init_stderr;     // std err from virtualenv initialization

    // widgets served to Plugin Manager
    QString venv_status_str;
    QLabel *l_venv_status;          // label for venv init status (or N/A if not needed)
    QPushButton *pb_venv_init_log;  // pushbutton for viewing venv init log
  };

}; // end of comp namespace

#endif
