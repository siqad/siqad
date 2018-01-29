// @file:     afmnode.cc
// @author:   Samuel
// @created:  2018.01.17
// @editted:  2018.01.17 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Node in AFM travel path

#include "afmnode.h"

namespace prim{

// Initialise statics
QColor AFMNode::fill_col_default;
QColor AFMNode::fill_col_hovered;
QColor AFMNode::fill_col_sel;
QColor AFMNode::bd_col_default;
QColor AFMNode::bd_col_hovered;
QColor AFMNode::bd_col_sel;

qreal AFMNode::diameter = -1;
qreal AFMNode::edge_width = -1;

// constructors
AFMNode::AFMNode(int lay_id, QPointF scenepos, float z_offset)
  : prim::Item(prim::Item::AFMNode)
{
  initAFMNode(lay_id, scenepos, z_offset);
}

AFMNode::AFMNode(QXmlStreamReader *rs, QGraphicsScene *scene)
  : prim::Item(prim::Item::AFMNode)
{
  int lay_id = -1;
  QPointF scenepos;
  float z_offset;

  while (!rs->atEnd()) {
    if (rs->isStartElement()) {
      if (rs->name() == "layer_id") {
        lay_id = rs->readElementText().toInt();
        rs->readNext();
      } else if (rs->name() == "scenepos") {
        for (QXmlStreamAttribute &attr : rs->attributes()) {
          if (attr.name().toString() == QLatin1String("x")) {
            scenepos.setX(attr.value().toFloat());
          } else if (attr.name().toString() == QLatin1String("y")) {
            scenepos.setY(attr.value().toFloat());
          }
        }
        qDebug() << QObject::tr("Found x=%1 and y=%2 for node").arg(scenepos.x()).arg(scenepos.y());
        rs->readNext();
      } else if (rs->name() == "zoffset") {
        z_offset = rs->readElementText().toFloat();
        qDebug() << QObject::tr("Found node zoffset=%1").arg(z_offset);
        rs->readNext();
      } else {
        rs->readNext();
      }
    } else if (rs->isEndElement()) {
      // break out of read stream if the end of this element has been reached
      if (rs->name() == "afmnode") {
        rs->readNext();
        break;
      }
      rs->readNext();
    } else {
      rs->readNext();
    }
  }

  if (rs->hasError()) {
    qCritical() << QObject::tr("XML error: ") << rs->errorString().data();
  }

  initAFMNode(lay_id, scenepos, z_offset);
}

void AFMNode::initAFMNode(int lay_id, QPointF scenepos, float z_offset)
{
  // initialise static variables if not already
  if (diameter < 0)
    prepareStatics();

  layer_id = lay_id;
  setZOffset(z_offset);
  setPos(scenepos);

  // GUI properties
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setAcceptHoverEvents(true);
  setZValue(2);
}


// Save to XML
void AFMNode::saveItems(QXmlStreamWriter *ws) const
{
  ws->writeStartElement("afmnode");

  ws->writeTextElement("layer_id", QString::number(layer_id));

  ws->writeEmptyElement("scenepos"); // TODO consider changing to saving physloc
  ws->writeAttribute("x", QString::number(scenePos().x()));
  ws->writeAttribute("y", QString::number(scenePos().y()));

  ws->writeTextElement("zoffset", QString::number(zOffset()));

  // TODO other properties

  ws->writeEndElement();
}


// Node manipulation
void AFMNode::setZOffset(float z_offset)
{
  zoffset = z_offset;
  // Check whether the z_offset is within bounds of the layer. If not, expand
  // the layer's Z-Offset or Z-Height to make it fit, or alert the user if the
  // desired z_offset is not valid, e.g. runs into the surface.

  // TODO
}


// Graphics
QRectF AFMNode::boundingRect() const
{
  qreal width = diameter+edge_width;
  return QRectF(-.5*width, -.5*width, width, width);
}

void AFMNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  if (select_mode && upSelected()) {
    fill_col = fill_col_sel;
    bd_col = bd_col_sel;
  } else if (isHovered()) {
    fill_col = fill_col_hovered;
    bd_col = bd_col_hovered;
  } else {
    fill_col = fill_col_default;
    bd_col = bd_col_default;
  }

  QRectF rect = boundingRect();
  qreal dxy = .5*edge_width;
  rect.adjust(dxy,dxy,-dxy,-dxy);
  
  painter->setPen(Qt::NoPen);
  painter->setBrush(fill_col);
  painter->drawEllipse(rect);
}

Item *AFMNode::deepCopy() const
{
  // TODO
}


// PRIVATE

void AFMNode::prepareStatics()
{
  // Initialize statics
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  fill_col_default = gui_settings->get<QColor>("afmnode/fill_col_default");
  fill_col_hovered = gui_settings->get<QColor>("afmnode/fill_col_hovered");
  fill_col_sel = gui_settings->get<QColor>("afmnode/fill_col_sel");
  bd_col_default = gui_settings->get<QColor>("afmnode/bd_col_default");
  bd_col_hovered = gui_settings->get<QColor>("afmnode/bd_col_hovered");
  bd_col_sel = gui_settings->get<QColor>("afmnode/bd_col_sel");
  diameter = gui_settings->get<qreal>("afmnode/diameter")*scale_factor;
  edge_width = gui_settings->get<qreal>("afmnode/edge_width")*diameter;
}


// Mouse events

void AFMNode::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  // Show path config dialog when selected, with this node highlighted on the list
  // TODO select whole path if the whole path wasn't selected; select node if the parent path already is selected
  if (parentItem() != 0)
    parentItem()->setSelected(true);
  else
    prim::Item::mousePressEvent(e);
}

void AFMNode::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
  setHovered(true);
  update();
}

void AFMNode::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
  setHovered(false);
  update();
}



} // end of prim namespace
