// @file:     electrode.cc
// @author:   Nathan
// @created:  2017.10.27
// @editted:  2017.10.27 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     Electrode classes

#include <algorithm>
#include "electrode.h"
#include "settings/settings.h"



// Initialize statics
gui::PropertyMap prim::Electrode::default_class_properties;
qreal prim::Electrode::edge_width = -1;
QColor prim::Electrode::edge_col;
prim::Item::StateColors prim::Electrode::fill_col_def;
// QColor prim::Electrode::fill_col;
QColor prim::Electrode::selected_col; // edge colour, selected

// Draw on layer 0 for now.
prim::Electrode::Electrode(int lay_id, const QRectF &scene_rect)
  : prim::ResizeRotateRect(prim::Item::Electrode)
{
  if (edge_width == -1){
    constructStatics();
  }
  initElectrode(lay_id, scene_rect);
}

prim::Electrode::Electrode(int lay_id, QStringList points)
  : prim::ResizeRotateRect(prim::Item::Electrode)
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

prim::Electrode::Electrode(QXmlStreamReader *ls, QGraphicsScene *scene, int lay_id)
  : prim::ResizeRotateRect(prim::Item::Electrode)
{
  if(edge_width == -1){
    constructStatics();
  }
  QPointF ld_point1, ld_point2;
  qreal angle_in;
  QColor color;
  while(!ls->atEnd()){
    if(ls->isStartElement()){
      if(ls->name() == "electrode")
        ls->readNext();
      else if(ls->name() == "layer_id"){
        qDebug() << QObject::tr("The layer_id tag in designs are no longer used in loading. Using the lay_id supplied to the constructor instead.");
        ls->readNext();
        /*
        lay_id = ls->readElementText().toInt();
        ls->readNext();
        */
      }
      else if(ls->name() == "color"){
        color = QColor(ls->readElementText());
        ls->readNext();
      }
      else if(ls->name() == "angle"){
        angle_in = ls->readElementText().toFloat();
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
  if (color.isValid()) {
    setColor(color);
  } else {
    setColor(settings::GUISettings::instance()->get<QColor>("electrode/fill_col"));
  }
  setRotation(angle_in);
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
  } else if (action->text() == action_rotate_prop->text()) {
    requestRotation();
  } else if (action->text() == action_color_prop->text()) {
    colorChange();
  } else {
    qDebug() << QObject::tr("Matched no action.");
  }
}

void prim::Electrode::colorChange()
{
  QList<prim::Item*> target;
  target.append(this);
  prim::Emitter::instance()->sig_color_change(target);
}

void prim::Electrode::requestRotation()
{
  QList<prim::Item*> target;
  target.append(this);
  prim::Emitter::instance()->sig_rotate(target);
}

void prim::Electrode::createActions()
{
  action_show_prop = new QAction(QObject::tr("Show properties"));
  actions_list.append(action_show_prop);
  action_rotate_prop = new QAction(QObject::tr("Set rotation"));
  actions_list.append(action_rotate_prop);
  action_color_prop = new QAction(QObject::tr("Set color"));
  actions_list.append(action_color_prop);
}

void prim::Electrode::initElectrode(int lay_id, const QRectF &scene_rect)
{
  layer_id = lay_id;
  if(edge_width == -1){
    constructStatics();
  }
  setColor(fill_col_def.normal);
  createActions();
  setSceneRect(scene_rect);
  updatePolygon();
  setZValue(-1);
  // flags
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setFlag(QGraphicsItem::ItemIsFocusable, true);

}

QRectF prim::Electrode::boundingRect() const
{
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
  painter->setPen(QPen(edge_col, edge_width));
  QColor curr_fill_col = getCurrentStateColor(fill_col);
  // painter->setBrush(fill_col.normal.isValid() ? fill_col.normal : Qt::NoBrush);
  painter->setBrush(curr_fill_col.isValid() ? curr_fill_col : Qt::NoBrush);
  if(tool_type == gui::SelectTool && isSelected()){
    painter->setPen(Qt::NoPen);
    painter->setBrush(selected_col);
  }
  painter->drawPolygon(getPolygon());
}


prim::Item *prim::Electrode::deepCopy() const
{
  prim::Electrode *elec = new Electrode(layer_id, sceneRect());
  elec->setColor(fill_col.normal);
  elec->setRotation(getAngleDegrees());
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
  ss->writeTextElement("angle", QString::number(getAngleDegrees()));
  ss->writeTextElement("color", fill_col.normal.name(QColor::HexArgb));
  ss->writeStartElement("property_map");
  gui::PropertyMap::writeValuesToXMLStream(properties(), ss);
  // other attributes
  // ......
  ss->writeEndElement();
  ss->writeEndElement();
}

void prim::Electrode::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  // TODO Nathan do these need to be captured?
  switch(e->buttons()){
    case Qt::RightButton:
      //qDebug() << "Right clicked!";
      break;
    case Qt::LeftButton:
      qDebug() << "Left clicked!";
      ResizeRotateRect::mousePressEvent(e);
      break;
    default:
      prim::Item::mousePressEvent(e);
      break;
  }
}

void prim::Electrode::setColor(QColor color)
{
  //Change the color for this specific electrode
  fill_col.normal = color;
  //Change the default color used for later electrodes
  fill_col_def.normal = color;
  // qDebug() << color.name(QColor::HexArgb);
}

void prim::Electrode::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
  // TODO Nathan do these need to be captured?
  switch(e->buttons()){
    case Qt::RightButton:
      //qDebug() << "Right clicked!";
      break;
    case Qt::LeftButton:
      qDebug() << "Left clicked!";
      ResizeRotateRect::mouseReleaseEvent(e);
      break;
    default:
      prim::Item::mouseReleaseEvent(e);
      break;
  }
}

void prim::Electrode::constructStatics() //needs to be changed to look at electrode settings instead.
{

  default_class_properties.readPropertiesFromXML(":/properties/electrode.xml");

  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  edge_width = gui_settings->get<qreal>("electrode/edge_width");
  edge_col= gui_settings->get<QColor>("electrode/edge_col");
  // fill_col= gui_settings->get<QColor>("electrode/fill_col");
  fill_col_def.normal = gui_settings->get<QColor>("electrode/fill_col");

  selected_col= gui_settings->get<QColor>("electrode/selected_col");
}
