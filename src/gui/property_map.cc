// @file:     property_map.cc
// @author:   Samuel
// @created:  2018.03.28
// @editted:  2018.03.28  - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Reads properties from XML resources, parses them and makes them
//            available as a map

#include "property_map.h"

namespace gui{

// initialize static variables
QMap<QString, int> PropertyMap::string2type(std::map<QString, int> {
  {"int", QMetaType::Int},
  {"float", QMetaType::Float},
  {"double", QMetaType::Double},
  {"string", QMetaType::QString}
});


// constructor
PropertyMap::PropertyMap(const QString &fname)
  : xml_path(fname)
{
  readPropertiesFromXML(xml_path);
}


// read properties from XML file at indicated path
void PropertyMap::readPropertiesFromXML(const QString &fname)
{
  xml_path = fname;

  QFile file(xml_path);

  // test whether file can be opened to read
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    qFatal(QObject::tr("Error when opening properties file to read: %1")
        .arg(file.errorString()).toLatin1().constData(), 0);
    return;
  }

  // read from XML stream
  QXmlStreamReader rs(&file);
  qDebug() << QObject::tr("Beginning load from %1").arg(file.fileName());

  readPropertiesFromXMLStream(&rs, "properties");

  file.close();
  qDebug() << QObject::tr("Finished loading from %1").arg(file.fileName());
}


// read properties from XML stream until the end of the indicated tag
void PropertyMap::readPropertiesFromXMLStream(QXmlStreamReader *rs, const QString &enclosed_tag)
{
  while ( !rs->atEnd() && !(rs->name() == enclosed_tag && rs->isEndElement()) ) {
    if (!rs->readNextStartElement())
      continue;

    // root node
    if (rs->name() == enclosed_tag)
      continue;

    // read new property
    readProperty(rs->name().toString(), rs);
  }

  if (rs->hasError())
    qCritical() << QObject::tr("XML error: ") << rs->errorString().data();
}


// read one property in the XML stream, assuming that the entry point is already
// the beginning of the property
void PropertyMap::readProperty(const QString &node_name, QXmlStreamReader *rs)
{
  if (contains(node_name))
    qCritical() << QObject::tr("Property %1 has been repeated").arg(node_name);

  Property prop;
  int p_type_id=-1;
  QString p_val;
  qDebug() << QObject::tr("Reading content of property %1").arg(node_name);

  // keep reading until the end of this property node
  while ( !(rs->name() == node_name && rs->isEndElement())
          && rs->readNextStartElement()) {
    if (rs->name() == "T") {
      p_type_id = string2type[rs->readElementText()];
      qDebug() << QObject::tr("%1 type=%2").arg(node_name).arg(p_type_id);
    } else if (rs->name() == "val") {
      p_val = rs->readElementText();
      qDebug() << QObject::tr("%1 val=%2").arg(node_name).arg(p_val);
      //rs->skipCurrentElement();
    } else if (rs->name() == "label") {
      prop.form_label = rs->readElementText();
      qDebug() << QObject::tr("%1 label=%2").arg(node_name).arg(prop.form_label);
      //rs->skipCurrentElement();
    } else if (rs->name() == "tip") {
      prop.form_tip = rs->readElementText();
      qDebug() << QObject::tr("%1 tip=%2").arg(node_name).arg(prop.form_tip);
      //rs->skipCurrentElement();
    } else if (rs->name() == "value_selection") {
      if (rs->attributes().value("type") == "ComboBox") {
        readComboOptions(&prop, p_type_id, rs);
      }
      for (ComboOption combo_option : prop.value_selection.combo_options) {
        qDebug() << QObject::tr("Combo val is %1").arg(combo_option.val.toString());
      }
    } else {
      qDebug() << "else";
      rs->readNext();
    }
  }

  // convert the value to the intended type
  if (p_type_id == -1)
    qFatal("Unable to read type id of a property");

  prop.value = string2Type2QVariant(p_val, p_type_id);

  qDebug() << QObject::tr("Got value of %1 with type %2")
      .arg(prop.value.toString()).arg(p_type_id);

  // add this property to the map
  insert(node_name, prop);
}

// read combo options
void PropertyMap::readComboOptions(Property *prop, int type_id, QXmlStreamReader *rs)
{
  prop->value_selection = Combo;
  while ( !(rs->name() == "value_selection" && rs->isEndElement())
          && rs->readNextStartElement()) {
    prop->value_selection.combo_options.append(
        ComboOption(string2Type2QVariant(rs->name().toString(), type_id),
                    rs->readElementText()));
  }
}

QVariant PropertyMap::string2Type2QVariant(const QString &val, int type_id)
{
  switch (type_id) {
    case QMetaType::Int:
      return QVariant(val.toInt());
      break;
    case QMetaType::Float:
      return QVariant(val.toFloat());
      break;
    case QMetaType::Double:
      return QVariant(val.toDouble());
      break;
    case QMetaType::QString:
      return QVariant(val);
      break;
    default:
      qFatal("Trying to convert string to an unsupported type");
      return QVariant(0);
      break;
  }
}


} // end of gui namespace
