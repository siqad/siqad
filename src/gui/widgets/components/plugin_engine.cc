// @file:     plugin_engine.cc
// @author:   Samuel
// @created:  2019.03.13
// @license:  GNU LGPL v3
//
// @desc:     Plugin engine implementation.

#include "plugin_engine.h"
#include "settings/settings.h"


using namespace comp;

QList<PluginEngine::Service> PluginEngine::official_services;

PluginEngine::PluginEngine(const QString &desc_file_path, QWidget *parent)
  : QObject(parent), desc_file_path(desc_file_path)
{
  venv_status_str = "Not needed";
  l_venv_status = new QLabel(venv_status_str);
  pb_venv_init_log = new QPushButton;

  QFileInfo desc_file_info(desc_file_path);
  plugin_root_path = desc_file_info.absolutePath();
  py_use_virtualenv = false; // only set to true if the physeng file requests
  venv_init_success = false; // only set to true after initialization succeeds

  QFile desc_file(desc_file_path);
  if (!desc_file.open(QFile::ReadOnly | QFile::Text)) {
    qCritical() << tr("Failed to open plugin description file: %1")
      .arg(desc_file_info.absoluteFilePath());
    return;
  }

  QXmlStreamReader rs(&desc_file);
  qDebug() << tr("Reading plugin file from %1").arg(desc_file_info.absoluteFilePath());

  // enter the XML root node
  rs.readNextStartElement();

  auto unrecognizedXMLElement = [](QXmlStreamReader &rs) mutable
  {
    qWarning() << tr("Invalid element encountered on line %1 - %2")
      .arg(rs.lineNumber()).arg(rs.name().toString());
    rs.skipCurrentElement();
  };

  while (rs.readNextStartElement()) {
    if (rs.name() == "name") {
      plugin_name = rs.readElementText();
    } else if (rs.name() == "version") {
      plugin_version = rs.readElementText();
    } else if (rs.name() == "services") {
      plugin_services = rs.readElementText().split(",");
    } else if (rs.name() == "bin_path") {
      // TODO perform path replacement instead
      bin_path = QDir(plugin_root_path).absoluteFilePath(rs.readElementText());
      // attempt to search for bin_path + ".exe" if running on Windows
      if (!QFileInfo(bin_path).exists() && QSysInfo::kernelType() == "winnt") {
        QString alt_bin_path = bin_path + ".exe";
        if (QFileInfo(alt_bin_path).exists()) {
          bin_path = alt_bin_path;
        }
      }
    } else if (rs.name() == "py_use_virtualenv") {
      // introduced in SiQAD v0.2.2
      py_use_virtualenv = rs.readElementText() == "1";
      venv_status_str = "Pending init";
      l_venv_status->setText(venv_status_str);
    } else if (rs.name() == "venv_use_system_site_packages") {
      // introduced in SiQAD v0.2.2
      venv_use_system_site = rs.readElementText() == "1";
    } else if (rs.name() == "dep_path") {
      // TODO perform path replacement instead
      dep_path = QDir(plugin_root_path).absoluteFilePath(rs.readElementText());
    } else if (rs.name() == "commands") {
      while (rs.readNextStartElement()) {
        if (rs.name() == "command") {
          QString cmd_label = rs.attributes().value("label").toString();
          QStringList cmd_list;
          while (rs.readNextStartElement()) {
            if (rs.name() == "program" || rs.name() == "arg") {
              cmd_list.append(rs.readElementText());
            } else {
              unrecognizedXMLElement(rs);
            }
          }
          command_formats.append(qMakePair(cmd_label, cmd_list));
        } else {
          unrecognizedXMLElement(rs);
        }
      }
    } else if (rs.name() == "requested_datasets") {
      // TODO remove or implement
      rs.skipCurrentElement();
    } else if (rs.name() == "return_datasets") {
      QMetaEnum req_enum = QMetaEnum::fromType<ReturnableDataset>();
      returnable_datasets.insert(static_cast<ReturnableDataset>(
            req_enum.keyToValue(rs.readElementText().toLatin1())));
    } else if (rs.name() == "sim_params") {
      default_prop_map.readPropertiesFromXMLStream(&rs);
    } else {
      unrecognizedXMLElement(rs);
    }
  }

  desc_file.close();

  unique_identifier = qHash(plugin_name + desc_file_path);

  // initialize engine preset storage path if it doesn't already exist
  if (preset_dir_path.isEmpty()) {
    QDir preset_root_dir(settings::AppSettings::instance()->getPath("plugs/preset_root_path"));
    QDir eng_preset_dir(preset_root_dir.filePath(name()));
    if (!eng_preset_dir.mkpath(".")) {
      qWarning() << tr("Unable to create engine preset directory %1").arg(eng_preset_dir.path());
    }
    preset_dir_path = eng_preset_dir.path();
  }

  // prepare virtual environment if needed
  if (py_use_virtualenv) {
    prepareVirtualenv();
  }
}

void PluginEngine::prepareVirtualenv()
{
  if (!py_use_virtualenv) {
    return;
  }

  venv_status_str = "Initializing venv";
  l_venv_status->setText(venv_status_str);

  auto term_out = [this](QProcess *p) {
    connect(p, &QProcess::readyReadStandardOutput,
        [this,p](){
          venv_init_stdout.append(QString::fromUtf8(p->readAllStandardOutput()));
        });
    connect(p, &QProcess::readyReadStandardError,
        [this,p](){
          venv_init_stderr.append(QString::fromUtf8(p->readAllStandardError()));
        });
  };

  auto venv_pip = [this, term_out]() {
    venv_status_str = "Downloading pip packages";
    l_venv_status->setText(venv_status_str);

    // install pip dependencies
    QProcess *dep_process = new QProcess;
    dep_process->setProcessChannelMode(QProcess::MergedChannels);
    dep_process->setProgram(pythonBin());
    dep_process->setArguments(QStringList({
          "-m",
          "pip",
          "install",
          "-r",
          QDir(pluginRootPath()).filePath("requirements.txt")
          }));
    dep_process->start();
    qDebug() << tr("(This may take some time) installing pip dependencies for venv %1...").arg(virtualenvPath());

    connect(dep_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [this](int ecode, QProcess::ExitStatus estatus)
        {
          if (ecode != 0 || estatus != QProcess::NormalExit) {
            qWarning() << tr("Plugin %1 failed to install all pip dependencies, "
                "exit code %2.").arg(name()).arg(ecode);
            venv_status_str = "Pip download failed";
            l_venv_status->setText(venv_status_str);
          } else {
            qDebug() << tr("Plugin %1 finished installing pip dependencies.").arg(name());
            venv_init_success = true;
            venv_status_str = "Ready";
            l_venv_status->setText(venv_status_str);
          }
        });

    term_out(dep_process);
  };

  if (gui::python_path.isEmpty()) {
    venv_status_str = "No Python interpreter found";
    l_venv_status->setText(venv_status_str);
    qWarning() << tr("No Python interpreter found, cannot initialize venv for "
        "plugin %1").arg(name());
    return;
  }
  
  QProcess *venv_process = new QProcess();
  venv_process->setProcessChannelMode(QProcess::MergedChannels);
  venv_process->setProgram(gui::python_path); 
  QStringList venv_args = {
      "-m",
      "venv",
      virtualenvPath()
      };
  if (venv_use_system_site) {
    venv_args << "--system-site-packages";
  }
  venv_process->setArguments(venv_args);
  venv_process->start();
  qDebug() << tr("Creating Python venv at %1...").arg(virtualenvPath());

  connect(venv_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
      [this,venv_pip](int ecode, QProcess::ExitStatus estatus)
      {
        if (ecode != 0 || estatus != QProcess::NormalExit) {
          qWarning() << tr("Plugin %1 failed to initialize Python venv, exit "
              "code %2.").arg(name()).arg(ecode);
          venv_status_str = "Init failed";
          l_venv_status->setText(venv_status_str);
        } else if (pythonBin().isEmpty()) {
          qWarning() << tr("No venv Python executable found under the provided "
              "venv base path %1. This plugin will not be able to function.").arg(virtualenvPath());
          venv_status_str = "Py bin not found after init";
          l_venv_status->setText(venv_status_str);
        } else {
          qDebug() << tr("Plugin %1 finished initializing Python venv, moving "
              "onto pip dependency installation.").arg(name());
          venv_pip();
        }
      });

  term_out(venv_process);
}

QString PluginEngine::pluginStatusStr()
{
  if (py_use_virtualenv && !venv_init_success) {
    return venv_status_str;
  }
  return "Ready";
}

QList<QStandardItem*> PluginEngine::standardItemRow(QList<StandardItemField> fields) const
{
  QList<QStandardItem*> row_si;

  for (StandardItemField field : fields) {
    QStandardItem *si = nullptr;
    switch (field) {
      case UniqueIdentifierField:
        si = new QStandardItem(QString::number(uniqueIdentifier()));
        break;
      case ServicesField:
        si = new QStandardItem(services().join(","));
        break;
      case NameField:
        si = new QStandardItem(name());
        break;
      case VersionField:
        si = new QStandardItem(version());
        break;
      case RootPathField:
        si = new QStandardItem(pluginRootPath());
        break;
      case BinaryPathField:
        si = new QStandardItem(binaryPath());
        break;
      case DependenciesPathField:
        si = new QStandardItem(dependenciesFilePath());
        break;;
      case DescriptionPathField:
        si = new QStandardItem(descriptionFilePath());
        break;
      case UserPresetDirectoryPathField:
        si = new QStandardItem(userPresetDirectoryPath());
        break;
      default:
        qWarning() << tr("Unknown StandardItemField encountered %1").arg(field);
        break;
    }

    if (si != nullptr) {
      si->setData(field, EnginePropertyFieldRole);
      row_si.append(si);
    }
  }

  return row_si;
}


QString PluginEngine::virtualenvPath()
{
  // TODO might be better to make this dev configurable in the physeng file
  QDir eng_preset_dir(userPresetDirectoryPath());
  return eng_preset_dir.filePath("venv");
}

QString PluginEngine::pythonBin()
{
  if (!py_use_virtualenv) {
    return gui::python_path;
  }

  QStringList venv_py_paths({
      "bin/python3",
      "bin/python",
      "Scripts/python.exe"
      });

  for (QString venv_py_path : venv_py_paths) {
    if (QDir(virtualenvPath()).exists(venv_py_path)) {
      return QDir(virtualenvPath()).filePath(venv_py_path);
    }
  }

  qWarning() << "No venv Python executable found.";
  return "";
}

QPushButton *PluginEngine::widgetVenvInitLog()
{
  // TODO make required connections for pop-op box creation
  pb_venv_init_log->setText("Venv Init Log");
  
  connect(pb_venv_init_log, &QPushButton::pressed,
      [this](){
        QWidget *wid = new QWidget();
        wid->setWindowFlag(Qt::Dialog);
        QPlainTextEdit *te_term_out = new QPlainTextEdit();
        te_term_out->setPlainText(venv_init_stdout);
        QVBoxLayout *vb = new QVBoxLayout;
        vb->addWidget(te_term_out);
        wid->setLayout(vb);
        wid->show();
      });
  return pb_venv_init_log;
}
