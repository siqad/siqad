/** @file:     scale_bar.cc
 *  @author:   Samuel
 *  @created:  2019.01.06
 *  @editted:  2019.01.06  - Samuel
 *  @license:  GNU LGPL v3
 *
 *  @brief:    Scale bar.
 */

#include "scale_bar.h"

namespace prim{

qreal ScaleBar::bar_thickness=-1;
qreal ScaleBar::text_height;
QColor ScaleBar::bar_col;

ScaleBar::ScaleBar(int t_layer_id, float t_bar_length, gui::Unit::DistanceUnit t_bar_unit, 
                   QPointF scene_pos)
  : prim::Item(prim::Item::ScaleBar, t_layer_id)
{
  if (bar_thickness == -1)
    constructStatics();

  setScaleBar(t_bar_length, t_bar_unit);
  setPos(scene_pos);
}


void ScaleBar::setScaleBar(float t_bar_length, gui::Unit::DistanceUnit t_bar_unit)
{
  prepareGeometryChange();
  bar_length = t_bar_length;
  bar_unit = t_bar_unit;
  bar_length_px = scale_factor * gui::Unit::valueConvertDistanceUnit(bar_length, t_bar_unit, gui::Unit::ang);
  qDebug() << QObject::tr("set to %1 %2, px %3").arg(bar_length).arg(gui::Unit::distanceUnitString(t_bar_unit)).arg(bar_length_px);
}

void ScaleBar::setScenePos(const QPointF &scene_pos)
{
  prepareGeometryChange();
  setPos(scene_pos);
}

void ScaleBar::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  // TODO for now the scale text is not added. When added update this function.
  painter->setPen(QPen(bar_col, bar_thickness));
  painter->setBrush(Qt::NoBrush);
  painter->drawLine(QLineF(QPointF(.5*bar_thickness,0), 
                    QPointF(bar_length_px-.5*bar_thickness,0)));

  QString scale_label = QString::number(bar_length) + " " 
    + gui::Unit::distanceUnitString(bar_unit);
  QFont font = painter->font();
  font.setPixelSize(text_height);
  painter->setFont(font);
  painter->drawText(boundingRect(), Qt::AlignCenter, scale_label);
}


QRectF ScaleBar::boundingRect() const
{
  return QRectF(0, -.5*bar_thickness, bar_length_px, bar_thickness + 2 * text_height);
}


// PRIVATE

void ScaleBar::constructStatics()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  bar_thickness = gui_settings->get<qreal>("scalebar/edge_width") * scale_factor;
  text_height = gui_settings->get<qreal>("scalebar/text_height") * scale_factor;
  bar_col = gui_settings->get<QColor>("scalebar/edge_col");
}



} // end of prim namespace
