// @file:     electrode.cc
// @author:   Nathan
// @created:  2017.10.27
// @editted:  2017.10.27 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     Electrode classes

#include <algorithm>
#include "electrode.h"
#include "src/settings/settings.h"



// Initialize statics
gui::PropertyMap prim::Electrode::default_class_properties;
qreal prim::Electrode::edge_width = -1;
QColor prim::Electrode::edge_col;
QColor prim::Electrode::fill_col;
QColor prim::Electrode::selected_col; // edge colour, selected

// Draw on layer 0 for now.
prim::Electrode::Electrode(int lay_id, QPointF point1, QPointF point2):
  prim::Item(prim::Item::Electrode)
{
  initElectrode(lay_id, point1, point2);
}

prim::Electrode::Electrode(QXmlStreamReader *ls, QGraphicsScene *scene) :
  prim::Item(prim::Item::Electrode)
{
  int lay_id=-1;
  QPointF ld_point1, ld_point2;
  while(!ls->atEnd()){
    if(ls->isStartElement()){
      if(ls->name() == "electrode")
        ls->readNext();
      else if(ls->name() == "layer_id"){
        lay_id = ls->readElementText().toInt();
        ls->readNext();
      }
      else if(ls->name() == "dim"){
        for(QXmlStreamAttribute &attr : ls->attributes()){
          if(attr.name().toString() == QLatin1String("x1"))
            ld_point1.setX(attr.value().toFloat());
          else if(attr.name().toString() == QLatin1String("y1"))
            ld_point1.setY(attr.value().toFloat());
          else if(attr.name().toString() == QLatin1String("x2"))
            ld_point2.setX(attr.value().toFloat());
          else if(attr.name().toString() == QLatin1String("y2"))
            ld_point2.setY(attr.value().toFloat());
        }
        ls->readNext();
      }
      else if(ls->name() == "property_map"){
        qDebug() << QObject::tr("FOUND PROPMAP");
        propMapFromXml(ls);
        ls->readNext();
      }
      // TODO the rest of the variables
      else{
        qDebug() << QObject::tr("Electrode: invalid element encountered on line %1 - %2").arg(ls->lineNumber()).arg(ls->name().toString());
        ls->readNext();
      }
    }
    else if(ls->isEndElement()){
      // break out of ls if the end of this element has been reached
      if(ls->name() == "electrode"){
        ls->readNext();
        break;
      }
      ls->readNext();
    }
    else
      ls->readNext();
  }
  if(ls->hasError())
    qCritical() << QObject::tr("XML error: ") << ls->errorString().data();
  if(lay_id != 2){
    qWarning() << "Electrode lay_id is at" << lay_id << ", should be at 2.";
  }
  if(ld_point1.isNull()){
    qWarning() << "ld_point1 is null";
  }
  if(ld_point2.isNull()){
    qWarning() << "ld_point2 is null";
  }
  //load all read data into init_electrode
  initElectrode(lay_id, ld_point1, ld_point2);
  scene->addItem(this);
}

// Resize according to given coordinates
void prim::Electrode::resize(qreal dx1, qreal dy1, qreal dx2, qreal dy2,
    bool update_handles)
{
  //setPos(scenePos() + QPointF(dx1, dy1));
  prepareGeometryChange();
  top_left += QPointF(dx1, dy1);
  bot_right += QPointF(dx2, dy2);

  // reverse change if user resizes past the other edge
  if (top_left.x() > bot_right.x() ||
      top_left.y() > bot_right.y()) {
    top_left -= QPointF(dx1, dy1);
    bot_right -= QPointF(dx2, dy2);
  }

  setPos(getTopLeft());
  update();

  if (update_handles && resize_frame)
    resize_frame->updateHandlePositions();
}

QVariant prim::Electrode::itemChange(GraphicsItemChange change, const QVariant &value)
{
  if (change == QGraphicsItem::ItemSelectedChange) {
    if (value == true) {
      if (!resize_frame) {
        resize_frame = new prim::ResizeFrame(this);
      }
      resize_frame->setVisible(true);
    } else {
      if (resize_frame) {
        resize_frame->setVisible(false);
      }
    }
  }

  return QGraphicsItem::itemChange(change, value);
}


void prim::Electrode::initElectrode(int lay_id, QPointF point1_in, QPointF point2_in)
{
  layer_id = lay_id;
  QPointF point1 = point1_in;
  QPointF point2 = point2_in;
  if(edge_width == -1){
    constructStatics();
  }
  qDebug() << QObject::tr("%1 %2").arg(getWidth()).arg(getHeight());

  top_left.setX(std::min(point1.x(), point2.x()));
  top_left.setY(std::min(point1.y(), point2.y()));
  bot_right.setX(std::max(point1.x(), point2.x()));
  bot_right.setY(std::max(point1.y(), point2.y()));
  setZValue(-1);
  setPos(mapToScene(top_left).toPoint());
  // flags
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setFlag(QGraphicsItem::ItemIsFocusable, true);
}

QRectF prim::Electrode::boundingRect() const
{
  qreal width = getWidth()+edge_width;
  qreal height = getHeight()+edge_width;
  return QRectF(0, 0, width, height);
}

// NOTE: nothing in this paint method changes... possibly cache background as
// pre-rendered bitma for speed.
void prim::Electrode::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  QRectF rect = boundingRect();
  qreal dxy = .5*edge_width;
  rect.adjust(dxy,dxy,-dxy,-dxy); //make the bounding rectangle, and trim off the edges.
  painter->setPen(QPen(edge_col, edge_width));
  painter->setBrush(fill_col.isValid() ? fill_col : Qt::NoBrush);
  painter->drawRect(rect);

  // draw inner circle
  if(tool_type == gui::SelectTool && isSelected()){

      // qDebug() << QObject::tr("Electrode selected");
    setPos(pos());
    QPointF center = rect.center();
    QSizeF size(getWidth()+edge_width, getHeight()+edge_width);
    rect.setSize(size);
    rect.moveCenter(center);

    painter->setPen(Qt::NoPen);
    painter->setBrush(selected_col);
    painter->drawRect(rect);
  }
}

prim::Item *prim::Electrode::deepCopy() const
{
  prim::Electrode *elec = new Electrode(layer_id, top_left, bot_right);
  elec->setPos(pos());
  return elec;
}

void prim::Electrode::saveItems(QXmlStreamWriter *ss) const
{
  ss->writeStartElement("electrode");
  // layer id
  ss->writeTextElement("layer_id", QString::number(layer_id));

  // top left and bottom right locations
  ss->writeEmptyElement("dim");
  ss->writeAttribute("x1", QString::number(std::min(top_left.x(), bot_right.x())));
  ss->writeAttribute("y1", QString::number(std::min(top_left.y(), bot_right.y())));
  ss->writeAttribute("x2", QString::number(std::max(top_left.x(), bot_right.x())));
  ss->writeAttribute("y2", QString::number(std::max(top_left.y(), bot_right.y())));
  ss->writeTextElement("pixel_per_angstrom", QString::number(scale_factor));
  ss->writeStartElement("property_map");
  gui::PropertyMap::writeValuesToXMLStream(properties(), ss);
  // other attributes
  // ......
  ss->writeEndElement();
  ss->writeEndElement();
}

void prim::Electrode::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  switch(e->buttons()){
    default:
      qDebug() << "should be showing property form.";
      prim::Emitter::instance()->sig_showProperty(this);
      prim::Item::mousePressEvent(e);
      break;
  }
}

// void prim::Electrode::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
// {
//   // qDebug() << QObject::tr("Electrode has seen the mouseDoubleClickEvent");
// }

void prim::Electrode::updatePoints(QPointF offset)
{
  //use after moving electrode with mouse. Graphic in correct place, but points need updating.
  top_left += offset;
  bot_right += offset;
}

void prim::Electrode::constructStatics() //needs to be changed to look at electrode settings instead.
{

  default_class_properties.readPropertiesFromXML(":/properties/electrode.xml");

  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  edge_width = gui_settings->get<qreal>("electrode/edge_width");
  edge_col= gui_settings->get<QColor>("electrode/edge_col");
  fill_col= gui_settings->get<QColor>("electrode/fill_col");
  selected_col= gui_settings->get<QColor>("electrode/selected_col");
}
