/** @file:     screenshot_clip_area.cc
 *  @author:   Samuel
 *  @created:  2019.01.04
 *  @editted:  2019.01.04  - Samuel
 *  @license:  GNU LGPL v3
 *
 *  @brief:    Screenshot clip area preview.
 */

#include "screenshot_clip_area.h"
#include "settings/settings.h"

qreal prim::ScreenshotClipArea::edge_width=-1;
//prim::ScreenshotClipArea::StateColors fill_col;
QColor prim::ScreenshotClipArea::edge_col;

namespace prim{

ScreenshotClipArea::ScreenshotClipArea(int t_layer_id, QRectF t_scene_rect)
  : prim::Item(prim::Item::ScreenshotClipArea, t_layer_id), scene_rect(t_scene_rect)
{
  // initialize static variables
  if (edge_width < 0)
    constructStatics();
}


void ScreenshotClipArea::setSceneRect(QRectF t_scene_rect)
{
  prepareGeometryChange();
  scene_rect = t_scene_rect;
}


void ScreenshotClipArea::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  if (scene_rect.isNull())
    return;

  qreal adj = edge_width*.5;
  QRectF edge_rect = scene_rect.adjusted(-adj,-adj,adj,adj);
  painter->setPen(QPen(edge_col, edge_width));
  painter->setBrush(Qt::NoBrush);
  painter->drawRect(edge_rect);
}

// PROTECTED

QRectF ScreenshotClipArea::boundingRect() const
{
  return scene_rect.adjusted(-edge_width,-edge_width,edge_width,edge_width);
}

//void ScreenshotClipArea::mousePressEvent(QGraphicsSceneMouseEvent *e)
//{
//
//}


// PRIVATE

void ScreenshotClipArea::constructStatics()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  edge_width = gui_settings->get<qreal>("screenshotcliparea/edge_width") * scale_factor;
  //fill_col = gui_settings->get<QColor>("screenshotcliparea/fill_col");
  edge_col = gui_settings->get<QColor>("screenshotcliparea/edge_col");
}


}
