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
prim::Electrode::Electrode(int lay_id, const QRectF &scene_rect)
  : prim::ResizableRect(prim::Item::Electrode)
{
  if (edge_width == -1){
    constructStatics();
  }
  initElectrode(lay_id, scene_rect);
}

prim::Electrode::Electrode(int lay_id, QStringList points)
  : prim::ResizableRect(prim::Item::Electrode)
{
  if (edge_width == -1){
    constructStatics();
  }
  float xmin = std::min(points[0].toFloat(), points[2].toFloat())*scale_factor;
  float xmax = std::max(points[0].toFloat(), points[2].toFloat())*scale_factor;
  float ymin = std::min(points[1].toFloat(), points[3].toFloat())*scale_factor;
  float ymax = std::max(points[1].toFloat(), points[3].toFloat())*scale_factor;
  QRectF scene_rect = QRectF(QPointF(xmin, ymin), QPointF(xmax,ymax));
  initElectrode(lay_id, scene_rect);
}

prim::Electrode::Electrode(QXmlStreamReader *ls, QGraphicsScene *scene) :
  prim::ResizableRect(prim::Item::Electrode)
{
  if(edge_width == -1){
    constructStatics();
  }
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
            ld_point1.setX(attr.value().toFloat()*scale_factor); //convert from angstrom to pixel
          else if(attr.name().toString() == QLatin1String("y1"))
            ld_point1.setY(attr.value().toFloat()*scale_factor);
          else if(attr.name().toString() == QLatin1String("x2"))
            ld_point2.setX(attr.value().toFloat()*scale_factor);
          else if(attr.name().toString() == QLatin1String("y2"))
            ld_point2.setY(attr.value().toFloat()*scale_factor);
        }
        ls->readNext();
      }
      else if(ls->name() == "property_map"){
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
  QRectF rect(ld_point1, ld_point2);
  initElectrode(lay_id, rect.normalized());
  scene->addItem(this);
}

void prim::Electrode::showProps()
{
  prim::Emitter::instance()->sig_showProperty(this);
}

void prim::Electrode::performAction(QAction *action)
{
  //switch case doesnt work on non-ints, use if else.
  if (action->text() == action_show_prop->text()) {
    showProps();
  } else {
    qDebug() << QObject::tr("Matched no action.");
  }
}

void prim::Electrode::createActions()
{
  action_show_prop = new QAction(QObject::tr("Show properties"));
  actions_list.append(action_show_prop);
}

void prim::Electrode::initElectrode(int lay_id, const QRectF &scene_rect)
{
  layer_id = lay_id;
  if(edge_width == -1){
    constructStatics();
  }
  createActions();
  setSceneRect(scene_rect);
  setZValue(-1);
  // flags
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setFlag(QGraphicsItem::ItemIsFocusable, true);

}

QPolygon prim::Electrode::getPolygon() const
{
  QRect rect = sceneRect().toRect();
  rect.moveTo(0,0);
  QTransform t;
  float angle = getProperty("angle").value.toFloat();
  t.translate(sceneRect().width()*0.5, sceneRect().height()*0.5);
  t.rotate(angle);
  t.translate(-sceneRect().width()*0.5, -sceneRect().height()*0.5);
  return t.mapToPolygon(rect);
}

QRectF prim::Electrode::boundingRect() const
{
  // QPolygon test = getRotatedPolygon();
  // QRectF rect_f = QRectF(getRotatedPolygon().boundingRect());
  // rect_f.moveTo(0,0);
  // qDebug() << "Poly: " << getRotatedPolygon().boundingRect();
  // qDebug() << "Rect: " << sceneRect();
  // return rect_f;
  // qreal width = sceneRect().width()+edge_width;
  // qreal height = sceneRect().height()+edge_width;
  // return QRectF(0, 0, width, height);
    // return QRectF(0,0, getPolygon().boundingRect().width(), getPolygon().boundingRect().height());
  return QRectF(getPolygon().boundingRect());
}

QPainterPath prim::Electrode::shape() const
{
  QPainterPath path;
  path.addPolygon(getPolygon());
  path.closeSubpath();
  return path;
}

// NOTE: nothing in this paint method changes... possibly cache background as
// pre-rendered bitma for speed.
void prim::Electrode::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  QRectF rect = boundingRect();
  qreal dxy = .5*edge_width;
  // rect.adjust(dxy,dxy,-dxy,-dxy); //make the bounding rectangle, and trim off the edges.
  painter->setPen(QPen(edge_col, edge_width));
  painter->setBrush(fill_col.isValid() ? fill_col : Qt::NoBrush);
  // painter->drawRect(rect);
  // float angle = getProperty("angle").value.toFloat();
  //
  // // qDebug() << "TRANSFORM" << getProperty("angle").value.toString();
  // QTransform t;
  // t.translate(sceneRect().width()*0.5, sceneRect().height()*0.5);
  // t.rotate(angle);
  // // t.rotate(45);
  // t.translate(-sceneRect().width()*0.5, -sceneRect().height()*0.5);
  // QPolygon poly = t.mapToPolygon(rect.toRect());
  painter->drawPolygon(getPolygon());
  // painter->translate(sceneRect().width()*0.5,sceneRect().height()*0.5);
  // painter->rotate(0);
  // rect.translate(-sceneRect().width()*0.5,-sceneRect().height()*0.5);
  // painter->drawRect(rect);
  if(tool_type == gui::SelectTool && isSelected()){
    // setPos(pos());
    // QPointF center = rect.center();
    // QSizeF size(sceneRect().width()+edge_width, sceneRect().height()+edge_width);
    // rect.setSize(size);
    // rect.moveCenter(center);
    painter->setPen(Qt::NoPen);
    painter->setBrush(selected_col);
    painter->drawPolygon(getPolygon());
    // painter->drawRect(rect);
  }
}


prim::Item *prim::Electrode::deepCopy() const
{
  prim::Electrode *elec = new Electrode(layer_id, sceneRect());
  return elec;
}

void prim::Electrode::saveItems(QXmlStreamWriter *ss) const
{
  ss->writeStartElement("electrode");
  // layer id
  ss->writeTextElement("layer_id", QString::number(layer_id));

  // top left and bottom right locations
  ss->writeEmptyElement("dim");
  ss->writeAttribute("x1", QString::number(sceneRect().topLeft().x()/scale_factor)); //convert to angstrom
  ss->writeAttribute("y1", QString::number(sceneRect().topLeft().y()/scale_factor));
  ss->writeAttribute("x2", QString::number(sceneRect().bottomRight().x()/scale_factor));
  ss->writeAttribute("y2", QString::number(sceneRect().bottomRight().y()/scale_factor));
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
    case Qt::RightButton:
      qDebug() << "Right clicked!";
      break;
    case Qt::LeftButton:
      qDebug() << "Left clicked!";
      break;
    default:
      prim::Item::mousePressEvent(e);
      break;
  }
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
