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
  {"bool", QMetaType::Bool},
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

  // enter the root node and read relevant attributes
  rs.readNextStartElement();
  if (rs.attributes().value("preserve_order").toString() == "true")
    preserve_order = true;

  // start reading properties
  if (rs.name().toString() == "properties") {
    map_label = rs.attributes().value("map_label").toString();
    readPropertiesFromXMLStream(&rs);
  }

  file.close();
  qDebug() << QObject::tr("Finished loading from %1").arg(file.fileName());
}


// read properties from XML stream until the end of the indicated tag
void PropertyMap::readPropertiesFromXMLStream(QXmlStreamReader *rs)
{
  while (rs->readNextStartElement())
    readProperty(rs->name().toString(), rs);

  if (rs->hasError())
    qCritical() << QObject::tr("XML error: ") << rs->errorString().data();
}

void PropertyMap::readValsFromXML(QXmlStreamReader *rs)
{
  // traverse through properties
  while (rs->readNextStartElement()) {
    QString key = rs->name().toString();
    if (!contains(key)) {
      qDebug() << QObject::tr("Encountered undefined key: %1").arg(key);
      rs->skipCurrentElement();
      continue;
    }
    // traverse through property content
    while (rs->readNextStartElement()) {
      if (rs->name().toString() == "val") {
        QVariant new_val = string2Type2QVariant(rs->readElementText(),
                                                value(key).value.userType());
        (*this)[key].value = new_val;
      }
    }
  }
}


// read one property in the XML stream, assuming that the entry point is already
// the beginning of the property
void PropertyMap::readProperty(const QString &node_name, QXmlStreamReader *rs)
{
  if (contains(node_name))
    qCritical() << QObject::tr("Property %1 has been repeated").arg(node_name);

  Property prop;
  prop.index = size();
  prop.dp = -1;
  int p_type_id=-1;
  QString p_val;
  //qDebug() << QObject::tr("Reading content of property %1").arg(node_name);

  // keep reading until the end of this property node
  while (rs->readNextStartElement()) {
    QString elem_name = rs->name().toString();
    if (elem_name == "T") {
      p_type_id = string2type[rs->readElementText()];
      //qDebug() << QObject::tr("%1 type=%2").arg(node_name).arg(p_type_id);
    } else if (elem_name == "val") {
      p_val = rs->readElementText();
      //qDebug() << QObject::tr("%1 val=%2").arg(node_name).arg(p_val);
    } else if (elem_name == "dp") {
      prop.dp = rs->readElementText().toInt();
    } else if (elem_name == "label") {
      prop.form_label = rs->readElementText();
      //qDebug() << QObject::tr("%1 label=%2").arg(node_name).arg(prop.form_label);
    } else if (elem_name == "tip") {
      prop.form_tip = rs->readElementText();
      //qDebug() << QObject::tr("%1 tip=%2").arg(node_name).arg(prop.form_tip);
    } else if (elem_name == "value_selection") {
      QString sel_type = rs->attributes().value("type").toString();
      if (sel_type == "ComboBox") {
        readComboOptions(&prop, p_type_id, rs);
      } else if (sel_type == "CheckBox") {
        prop.value_selection.type = CheckBox;
        rs->skipCurrentElement();
      }
    } else if (elem_name == "meta") {
      readMeta(&prop, rs);
    } else {
      qDebug() << QObject::tr("Unknown element '%1' when reading property map from XML").arg(elem_name);
      rs->skipCurrentElement();
    }
  }

  // convert the value to the intended type
  if (p_type_id < 0)
    qFatal("Unable to read type id of a property");

  prop.value = string2Type2QVariant(p_val, p_type_id);

  //qDebug() << QObject::tr("Got value of %1 with type %2")
      //.arg(prop.value.toString()).arg(p_type_id);

  // add this property to the map
  insert(node_name, prop);
}

// read combo options
void PropertyMap::readComboOptions(Property *prop, int type_id, QXmlStreamReader *rs)
{
  prop->value_selection.type = Combo;
  while (rs->readNextStartElement()) {
    prop->value_selection.combo_options.append(
        ComboOption(string2Type2QVariant(rs->name().toString(), type_id),
                    rs->readElementText()));
  }
}

// read meta data
void PropertyMap::readMeta(Property *prop, QXmlStreamReader *rs)
{
  while (rs->readNextStartElement()) {
    prop->meta.insert(rs->name().toString(), rs->readElementText());
    //qDebug() << QObject::tr("Added meta data '%1' with value '%2'").arg(rs->name().toString()).arg(prop->meta[rs->name().toString()]);
  }
}

// read property values and update the current map
void PropertyMap::updateValuesFromXML(const QString &fname)
{
  QFile file(fname);

  // test whether file can be opened to read
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    qFatal(QObject::tr("Error when opening properties file to read: %1")
        .arg(file.errorString()).toLatin1().constData(), 0);
    return;
  }

  // read from XML stream
  QXmlStreamReader rs(&file);
  qDebug() << QObject::tr("Beginning load from %1").arg(file.fileName());

  // enter the root node and read relevant attributes
  rs.readNextStartElement();

  // traverse through properties
  while (rs.readNextStartElement()) {
    QString key = rs.name().toString();
    if (!contains(key)) {
      qDebug() << QObject::tr("Encountered undefined key: %1").arg(key);
      rs.skipCurrentElement();
      continue;
    }
    // traverse through property content
    while (rs.readNextStartElement()) {
      if (rs.name().toString() == "val") {
        QVariant new_val = string2Type2QVariant(rs.readElementText(),
                                                value(key).value.userType());
        (*this)[key].value = new_val;
      }
    }
  }

  file.close();
  qDebug() << QObject::tr("Finished loading from %1").arg(file.fileName());
}

void PropertyMap::writeValuesToXMLStream(const PropertyMap &map, QXmlStreamWriter *ws)
{
  for (const QString &key : map.keys()) {
    ws->writeStartElement(key);
    ws->writeTextElement("val", map[key].value.toString());
    ws->writeEndElement();
  }
}


QVariant PropertyMap::string2Type2QVariant(const QString &val, int type_id)
{
  switch (type_id) {
    case QMetaType::Bool:
      return QVariant(static_cast<bool>(val.toInt()));
      break;
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
