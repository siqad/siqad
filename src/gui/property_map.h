// @file:     property_map.h
// @author:   Samuel
// @created:  2018.03.22
// @editted:  2018.03.28  - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Reads properties from XML resources, parses them and makes them
//            available as a map

#ifndef _GUI_PROPERTY_MAP_H_
#define _GUI_PROPERTY_MAP_H_

// Qt includes
#include <QtWidgets>
#include <QtCore>
#include <QMetaType>
#include <QMap>
#include <QXmlStreamReader>

// Global
#include "../global.h"


namespace gui{

  enum ValueSelectionType{LineEdit, Combo};

  //! A struct that stores a ComboOption
  struct ComboOption {
    ComboOption(const QVariant &val, const QString &label) : val(val), label(label) {};
    ComboOption() {};

    QVariant val;
    QString label;
  };

  //! A struct that stores the value selection type and values. If no type is
  //! specified then it's a line edit by default.
  struct ValueSelection {
    ValueSelection(const ValueSelectionType type,
      QList<ComboOption> combo_options=QList<ComboOption>())
      : type(type), combo_options(combo_options) {};
    ValueSelection() : type(LineEdit) {};

    ValueSelectionType type;
    QList<ComboOption> combo_options;
  };

  //! A struct that stores all information relevant to a property
  struct Property {
    Property(int index, const QVariant &val, const QString &f_label,
      const QString &f_tip, const ValueSelection &v_sel)
      : index(index), value(val), form_label(f_label), form_tip(f_tip),
        value_selection(v_sel) {};
    Property(const QVariant &val, const Property &p)
      : index(p.index), value(val), form_label(p.form_label), form_tip(p.form_tip),
        value_selection(p.value_selection) {};
    Property(const QVariant &val)
      : value(val) {};
    Property() {};

    int index;          // original index when read from file
    QVariant value;     // the value stored in this property
    QString form_label; // descriptive label when showing this in a form
    QString form_tip;   // tooltip when showing this in a form
    ValueSelection value_selection; // value selection method and options
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

    //! Read properties from XML stream until the end of the indicated tag.
    void readPropertiesFromXMLStream(QXmlStreamReader *rs);

    //! Read one property node from XML file.
    void readProperty(const QString &node_name, QXmlStreamReader *rs);

    //! Read combo_options for a combo box
    void readComboOptions(Property *prop, int type_id, QXmlStreamReader *rs);

    //! Update property values from provided XML file path. Keys that don't exist
    //! in this map are ignored.
    void updateValuesFromXML(const QString &fname);

    //! Write only property map values to XML stream
    static void writeValuesToXMLStream(const PropertyMap &map, QXmlStreamWriter *ws);

    //! Convert value to specified type and return a QVariant containing that
    //! converted value. Give the type_id in terms of QMetaType's enum.
    static QVariant string2Type2QVariant(const QString &val, int type_id);


  private:

    bool preserve_order=false;

    QString xml_path;

    static QMap<QString, int> string2type;
  };

} // end of gui namespace

#endif
