/** @file:     screenshot_clip_area.cc
 *  @author:   Samuel
 *  @created:  2019.01.04
 *  @editted:  2019.01.04  - Samuel
 *  @license:  GNU LGPL v3
 *
 *  @brief:    Screenshot clip area preview.
 */

#include "screenshot_clip_area.h"
#include "src/settings/settings.h"

qreal prim::ScreenshotClipArea::edge_width=0;
prim::ScreenshotClipArea::StateColors fill_col;
prim::ScreenshotClipArea::StateColors edge_col;

namespace prim{

ScreenshotClipArea::ScreenshotClipArea(QRectF t_scene_rect)
  : prim::Item(prim::Item::ScreenshotClipArea)
{
  initScreenshotClipArea(t_scene_rect);
}


void ScreenshotClipArea::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{

}

// PROTECTED

QRectF ScreenshotClipArea::boundingRect() const
{
  return scene_rect;
}

void ScreenshotClipArea::mousePressEvent(QGraphicsSceneMouseEvent *e)
{

}


// PRIVATE

void ScreenshotClipArea::initScreenshotClipArea(QRectF t_scene_rect)
{
  scene_rect = t_scene_rect;
}

void ScreenshotClipArea::constructStatics()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  edge_width = gui_settings->get<qreal>("screenshotcliparea/edge_width") * scale_factor;
  //fill_col = gui_settings->get<QColor>("screenshotcliparea/fill_col");
  //edge_col = gui_settings->get<QColor>("screenshotcliparea/edge_col");
}


}
