// @file:     electrode_poly.cc
// @author:   Nathan
// @created:  2017.10.27
// @editted:  2017.10.27 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     ElectrodePoly class for the functionality of polygonal electrodes

// #include <algorithm>
#include "electrode_poly.h"
#include "src/settings/settings.h"

// Initialize statics
// gui::PropertyMap prim::ElectrodePoly::default_class_properties;
qreal prim::ElectrodePoly::edge_width = -1;
QColor prim::ElectrodePoly::edge_col;
QColor prim::ElectrodePoly::fill_col;
QColor prim::ElectrodePoly::selected_col; // edge colour, selected

prim::ElectrodePoly::ElectrodePoly(const QPolygonF poly, const QRectF scene_rect, int lay_id)
  : prim::ResizablePoly(prim::Item::ElectrodePoly, poly, scene_rect, lay_id)
{
  initElectrodePoly(lay_id);
  qDebug() << "Creating ElectrodePoly";
}

prim::ElectrodePoly::~ElectrodePoly()
{
}

void prim::ElectrodePoly::initElectrodePoly(int lay_id)
{
  constructStatics();
  update();
  // setZValue(-1);

}

void prim::ElectrodePoly::saveItems(QXmlStreamWriter *ss) const
{
  ss->writeStartElement("electrode_poly");
  // layer id
  ss->writeTextElement("layer_id", QString::number(layer_id));

  QPolygonF poly_save = getTranslatedPolygon();

  for (QPointF vertex: poly_save) {
    ss->writeEmptyElement("vertex");
    ss->writeAttribute("x", QString::number(vertex.x()/scale_factor)); //convert to angstrom
    ss->writeAttribute("y", QString::number(vertex.y()/scale_factor));
  }
  ss->writeTextElement("pixel_per_angstrom", QString::number(scale_factor));
  // ss->writeStartElement("property_map");
  // gui::PropertyMap::writeValuesToXMLStream(properties(), ss);
  // other attributes
  // ......
  // ss->writeEndElement();
  ss->writeEndElement();
}

QRectF prim::ElectrodePoly::boundingRect() const
{
  return QRectF(0,0, getPolygon().boundingRect().width(), getPolygon().boundingRect().height());
}

void prim::ElectrodePoly::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  painter->setPen(QPen(edge_col, edge_width));
  painter->setBrush(fill_col.isValid() ? fill_col : Qt::NoBrush);
  painter->drawPolygon(getPolygon());
  if(tool_type == gui::SelectTool && isSelected()){
    painter->setPen(Qt::NoPen);
    painter->setBrush(selected_col);
    painter->drawPolygon(getPolygon());
  }
}

prim::Item *prim::ElectrodePoly::deepCopy() const
{
  QPolygonF new_poly = getPolygon();
  //return fresh polygon with unshifted coordinates.
  new_poly.translate(sceneRect().topLeft());
  prim::ElectrodePoly *ep = new ElectrodePoly(new_poly, sceneRect(), layer_id);
  return ep;
}

void prim::ElectrodePoly::constructStatics() //needs to be changed to look at electrode settings instead.
{
  // default_class_properties.readPropertiesFromXML(":/properties/electrode.xml");
  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  edge_width = gui_settings->get<qreal>("electrodepoly/edge_width");
  edge_col= gui_settings->get<QColor>("electrodepoly/edge_col");
  fill_col= gui_settings->get<QColor>("electrodepoly/fill_col");
  selected_col= gui_settings->get<QColor>("electrodepoly/selected_col");
}
