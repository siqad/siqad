// @file:     afmseg.cc
// @author:   Samuel
// @created:  2018.01.17
// @editted:  2018.01.17 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Segment in AFM travel path

#include "afmseg.h"

namespace prim{

// Initialise statics
QColor AFMSeg::line_col_default;
QColor AFMSeg::line_col_hovered;
QColor AFMSeg::line_col_sel;
qreal AFMSeg::line_width = -1;


// constructors
AFMSeg::AFMSeg(int lay_id, prim::AFMNode *orig_node, prim::AFMNode *dest_node)
  : prim::Item(prim::Item::AFMSeg)
{
  initAFMSeg(lay_id, orig_node, dest_node);
}

AFMSeg::AFMSeg(QXmlStreamReader *rs, QGraphicsScene *scene)
  : prim::Item(prim::Item::AFMSeg)
{
  // TODO call initAFMSeg from loaded values
}

void AFMSeg::initAFMSeg(int lay_id, prim::AFMNode *orig_node, prim::AFMNode *dest_node)
{
  if (line_width == -1)
    prepareStatics();

  layer_id = lay_id;
  setOriginNode(orig_node);
  setDestinationNode(dest_node);

  updatePoints();

  setZValue(1);
}


// Save to XML
void AFMSeg::saveItems(QXmlStreamWriter *ws) const
{

}


// Segment manipulation
void AFMSeg::setOriginNode(prim::AFMNode *orig_node)
{
  origin_node = orig_node;

  if (orig_node)
    updatePoints();
}

void AFMSeg::setDestinationNode(prim::AFMNode *dest_node)
{
  destination_node = dest_node;

  if (dest_node)
    updatePoints();
}


// Graphics
void AFMSeg::updatePoints()
{
  if (!segmentIsValid())
    return;

  prepareGeometryChange();
  origin_loc = originNode()->scenePos();
  destination_loc = destinationNode()->scenePos();
}


QRectF AFMSeg::boundingRect() const
{
  if (!segmentIsValid())
    return QRectF();

  qreal x_min = qMin(origin_loc.x(), destination_loc.x()) - line_width*0.5;
  qreal y_min = qMin(origin_loc.y(), destination_loc.y()) - line_width*0.5;
  qreal dx = qMax(origin_loc.x(), destination_loc.x()) - x_min + line_width;
  qreal dy = qMax(origin_loc.y(), destination_loc.y()) - y_min + line_width;

  // TODO expand a little to account for line width

  return QRectF(x_min, y_min, dx, dy);
}

void AFMSeg::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  if (!segmentIsValid())
    return;

  if (tool_type == gui::SelectTool && upSelected()) {
    line_col = line_col_sel;
  } else if (isHovered()) {
    line_col = line_col_hovered;
  } else {
    line_col = line_col_default;
  }

  QPen paint_pen = QPen();
  paint_pen.setColor(line_col);
  paint_pen.setCapStyle(Qt::RoundCap);
  paint_pen.setJoinStyle(Qt::RoundJoin);
  paint_pen.setWidth(line_width);


  painter->setPen(paint_pen);
  //painter->setBrush(QColor(255,255,255));
  painter->drawLine(QLineF(originNode()->scenePos(), destinationNode()->scenePos()));
}

Item *AFMSeg::deepCopy() const
{

}


// PRIVATE

void AFMSeg::prepareStatics()
{
  // Initialize statics
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  line_col_default = gui_settings->get<QColor>("afmseg/line_col_default");
  line_col_hovered = gui_settings->get<QColor>("afmseg/line_col_hovered");
  line_col_sel = gui_settings->get<QColor>("afmseg/line_col_sel");
  line_width = gui_settings->get<qreal>("afmseg/line_width");
}


void AFMSeg::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  // Show path config dialog when selected, with the two connected nodes highlighted
  // TODO
}

void AFMSeg::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
  qDebug() << QObject::tr("AFMSeg has seen the hoverEnterEvent");
  setHovered(true);
  update();
}

void AFMSeg::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
  qDebug() << QObject::tr("AFMSeg has seen the hoverLeaveEvent");
  setHovered(false);
  update();
}


} // end of prim namespace
