// @file:     property_map.h
// @author:   Samuel
// @created:  2018.03.22
// @editted:  2018.03.28  - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Reads properties from XML resources, parses them and makes them
//            available as a map

#ifndef _GUI_PROPERTIES_H_
#define _GUI_PROPERTIES_H_

// Qt includes
#include <QtWidgets>
#include <QtCore>
#include <QMetaType>
#include <QMap>
#include <QXmlStreamReader>

// Global
#include "../global.h"


namespace gui{

  //! A struct that stores all information relevant to a property
  struct Property {
    QVariant value;
    QString form_label; // descriptive label when showing this in a form
    QString form_tip;   // tooltip when showing this in a form
  };

  //! Read properties from XML resources, parses them and makes them accessible
  //! as a map. Kind of similar to QSettings in principle, just made to serve
  //! different needs. Properties objects are designed to be owned by prim::Item
  //! sub-classes as class variables, but they can be used wherever appropriate.
  class PropertyMap : public QMap<QString, Property>
  {

  public:

    //! Constructor
    PropertyMap() {};

    //! Constructor taking in XML file name to load properties from.
    PropertyMap(const QString &fname);

    //! Destructor
    ~PropertyMap() {}


    //! Read properties from XML file at provided path.
    void readPropertiesFromXML(const QString &fname);

    //! Read one property node from XML file.
    void readProperty(const QString &node_name, QXmlStreamReader *rs);

    //! Convert value to specified type and return a QVariant containing that
    //! converted value. Give the type_id in terms of QMetaType's enum.
    QVariant string2Type2QVariant(const QString &val, int type_id);


  private:
    
    QString xml_path;

    static QMap<QString, int> string2type;
  };

} // end of gui namespace

#endif
