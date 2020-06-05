// @file:     plugin_manager.h
// @author:   Samuel
// @created:  2019.03.13
// @license:  GNU LGPL v3
//
// @desc:     Widget for loading and managing plugins.

#ifndef _GUI_PLUGIN_MANAGER_H_
#define _GUI_PLUGIN_MANAGER_H_

#include <QtWidgets>

#include "../components/plugin_engine.h"

namespace gui{

  class PluginManager : public QWidget
  {
    Q_OBJECT

  public:

    //! Constructor.
    PluginManager(QWidget *parent=nullptr);

    //! Destructor.
    ~PluginManager();

    //! Refresh the plugin list.
    void refreshPluginList();

    //! Return the plugin count.
    int count() const {return plugin_engines.count();}

    //! Return a map of all plugins.
    QMap<uint, comp::PluginEngine*> pluginEngines() const {return plugin_engines;}

    //! Return the plugin engine corresponding to the selected unique identifier.
    comp::PluginEngine *getEngine(uint uid) {return plugin_engines.value(uid);}
    
    //! Return a list of plugins with the specified list of return types.

    // TODO engine list with specific requested data types

    // TODO engine list with specific return types

    // TODO engine list with specific services


  private:

    //! Initialize Python path. If a user preference has been set before, use 
    //! that one. Otherwise, check whether any of the default Python search 
    //! paths contain an invokable Python 3 interpreter and show a pop-up dialog
    //! asking the user to choose the preferred one. The user may also enter a 
    //! custom path in that dialog.
    void initPythonPath();

    //! Find python path.
    bool findWorkingPythonPath();

    //! Initialize plugin service types.
    void initServiceTypes();

    //! Initialize engines.
    void initPluginEngines();

    //! Initialize GUI.
    void initGui();

    // Map of plugin unique identifier to plugin engine pointers. This map 
    // contains all plugin engines.
    QMap<uint, comp::PluginEngine*> plugin_engines;

    // GUI elements
    QTreeView *tv_plugins;              // tree view of all plugins

    // GUI data models
    QStandardItemModel *plugins_model;  // programmed model for tv_plugins
  };

}

#endif
