// @file:     afmarea.cc
// @author:   Samuel
// @created:  2018.03.15
// @editted:  2018.03.15 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Class that contains information for 2D AFM scans

#include "afmarea.h"

namespace prim {

// Initialise statics
qreal AFMArea::area_border_width=-1;
prim::Item::StateColors AFMArea::area_border_col;
prim::Item::StateColors AFMArea::area_fill_col;
qreal AFMArea::scan_path_width;
prim::Item::StateColors AFMArea::scan_path_fill_col;

// Normal constructor
AFMArea::AFMArea(int lay_id, QPointF point1, QPointF point2, bool h_orientation,
    float z_spd, float h_spd, float v_spd, float v_disp)
  : prim::Item(prim::Item::AFMArea)
{
  initAFMArea(lay_id, point1, point2, h_orientation, z_spd, h_spd, v_spd, v_disp);
}

// Load XML constructor
AFMArea::AFMArea(QXmlStreamReader *rs, QGraphicsScene *scene)
  : prim::Item(prim::Item::AFMArea)
{
  // TODO
}

// Common initialization actions
void AFMArea::initAFMArea(int lay_id, QPointF point1, QPointF point2,
    bool h_orientation, float z_spd, float h_spd, float v_spd, float v_disp)
{
  layer_id = lay_id;

  // prepare static variables if they haven't been prepared
  if (area_border_width == -1)
    prepareStatics();

  QPointF top_left = QPointF(qMin(point1.x(), point2.x()),
      qMin(point1.y(), point2.y()));
  QPointF bot_right = QPointF(qMax(point1.x(), point2.x()),
      qMax(point1.y(), point2.y()));

  point_top_left = top_left;
  point_bot_right = bot_right;

  orientation = h_orientation;
  z_speed = z_spd;
  h_speed = h_spd;
  v_speed = v_spd;
  v_displacement = v_disp;

  // GUI flags
  setFlag(QGraphicsItem::ItemIsSelectable, true);

  setAcceptHoverEvents(true);
}

// Save to XML
void AFMArea::saveItems(QXmlStreamWriter *ws) const
{
  // TODO
}

// Generate path used for simulation
/*QList<global::AFMPathTimed> AFMArea::generateSimulationPath()
{
  // TODO
}*/


QRectF AFMArea::boundingRect() const
{
  //QPointF border_margin(area_border_width, area_border_width);
  //return QRectF(point_top_left-border_margin, point_bot_right+border_margin);
  return QRectF(point_top_left, point_bot_right);
}

void AFMArea::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  // draw area border and background
  QRectF rect = boundingRect();
  qreal dxy = .5*area_border_width;
  rect.adjust(dxy, dxy, -dxy, -dxy);  // trim off the border from bounding rect
  qDebug() << QObject::tr("Current area fill color: %1, %2, %3")
      .arg(getCurrentStateColor(area_fill_col).red())
      .arg(getCurrentStateColor(area_fill_col).green())
      .arg(getCurrentStateColor(area_fill_col).blue());
  painter->setPen(QPen(getCurrentStateColor(area_border_col), area_border_width));
  painter->setBrush(getCurrentStateColor(area_fill_col));
  painter->drawRect(rect);

  // TODO draw AFM scan path preview
}

Item *AFMArea::deepCopy() const
{
  return new prim::AFMArea(layer_id, point_top_left, point_bot_right,
      orientation, z_speed, h_speed, v_speed, v_displacement);
}


void AFMArea::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  switch(e->buttons()) {
    default:
      prim::Item::mousePressEvent(e);
      break;
  }
}

void AFMArea::hoverEnterEvent(QGraphicsSceneHoverEvent *e)
{
  qDebug() << "AFMArea has seen the hoverEnterEvent";
  setHovered(true);
  update();
}

void AFMArea::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
  qDebug() << "AFMArea has seen the hoverLeaveEvent";
  setHovered(false);
  update();
}

void AFMArea::prepareStatics()
{
  // Initialize static variables
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  // AFM area related
  area_border_width = gui_settings->
      get<qreal>("afmarea/area_border_width");
  area_border_col.normal = gui_settings->
      get<QColor>("afmarea/afm_border_normal");
  area_border_col.hovered = gui_settings->
      get<QColor>("afmarea/afm_border_hovered");
  area_border_col.selected = gui_settings->
      get<QColor>("afmarea/afm_border_selected");
  area_fill_col.normal = gui_settings->
      get<QColor>("afmarea/area_fill_normal");
  area_fill_col.hovered = gui_settings->
      get<QColor>("afmarea/area_fill_hovered");
  area_fill_col.selected = gui_settings->
      get<QColor>("afmarea/area_fill_selected");

  // Scan path related
  scan_path_width = gui_settings->
      get<qreal>("afmarea/scan_path_width");
  scan_path_fill_col.normal = gui_settings->
      get<QColor>("afmarea/scan_path_fill_normal");
  scan_path_fill_col.hovered = gui_settings->
      get<QColor>("afmarea/scan_path_fill_hovered");
  scan_path_fill_col.selected = gui_settings->
      get<QColor>("afmarea/scan_path_fill_selected");
}


} // end of prim namespace
