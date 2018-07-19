// @file:     electrode_poly.cc
// @author:   Nathan
// @created:  2017.10.27
// @editted:  2017.10.27 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     ElectrodePoly class for the functionality of polygonal electrodes

#include <algorithm>
#include "electrode_poly.h"
#include "src/settings/settings.h"

// Initialize statics
// gui::PropertyMap prim::ElectrodePoly::default_class_properties;
qreal prim::ElectrodePoly::edge_width = -1;
QColor prim::ElectrodePoly::edge_col;
QColor prim::ElectrodePoly::fill_col;
QColor prim::ElectrodePoly::selected_col; // edge colour, selected

prim::ElectrodePoly::ElectrodePoly(const QPolygonF poly, const QRectF scene_rect, int lay_id)
  : prim::Item(prim::Item::ElectrodePoly), poly(poly), scene_rect(scene_rect)
{
  initElectrodePoly(lay_id);
}

void prim::ElectrodePoly::initElectrodePoly(int lay_id)
{
  layer_id = lay_id;
  constructStatics();
  setPos(scene_rect.topLeft());
  poly.translate(-scene_rect.topLeft());
  update();
  setZValue(-1);
  setFlag(QGraphicsItem::ItemIsSelectable, true);
}

void prim::ElectrodePoly::test()
{
  for (QPointF point: poly){
    qDebug() << point;
  }
}

QRectF prim::ElectrodePoly::boundingRect() const
{
  return QRectF(0,0, poly.boundingRect().width(), poly.boundingRect().height());
}

void prim::ElectrodePoly::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  painter->setPen(QPen(edge_col, edge_width));
  painter->setBrush(fill_col.isValid() ? fill_col : Qt::NoBrush);
  painter->drawPolygon(poly);
  if(tool_type == gui::SelectTool && isSelected()){
    painter->setPen(Qt::NoPen);
    painter->setBrush(selected_col);
    painter->drawPolygon(poly);
  }
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
