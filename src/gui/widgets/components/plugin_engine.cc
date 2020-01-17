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
  QFileInfo desc_file_info(desc_file_path);
  plugin_root_path = desc_file_info.absolutePath();

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
    QDir preset_root_dir(settings::AppSettings::instance()->getPath("phys/preset_root_path"));
    QDir eng_preset_dir(preset_root_dir.filePath(name()));
    if (!eng_preset_dir.mkpath(".")) {
      qWarning() << tr("Unable to create engine preset directory %1").arg(eng_preset_dir.path());
    }
    preset_dir_path = eng_preset_dir.path();
  }
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
