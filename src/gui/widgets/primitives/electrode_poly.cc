// @file:     electrode_poly.cc
// @author:   Nathan
// @created:  2017.10.27
// @editted:  2017.10.27 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     ElectrodePoly class for the functionality of polygonal electrodes

// #include <algorithm>
#include "electrode_poly.h"
#include "src/settings/settings.h"

// Initialize statics
gui::PropertyMap prim::ElectrodePoly::default_class_properties;
qreal prim::ElectrodePoly::edge_width = -1;
QColor prim::ElectrodePoly::edge_col;
QColor prim::ElectrodePoly::fill_col;
QColor prim::ElectrodePoly::selected_col; // edge colour, selected

prim::ElectrodePoly::ElectrodePoly(const QPolygonF poly, int lay_id)
  : prim::ResizablePoly(prim::Item::ElectrodePoly, poly, lay_id)
{
  initElectrodePoly(lay_id, poly);
}




prim::ElectrodePoly::ElectrodePoly(QXmlStreamReader *ls, QGraphicsScene *scene)
  : prim::ResizablePoly(prim::Item::ElectrodePoly)
{
  // if(edge_width == -1){
  //   constructStatics();
  // }
  int lay_id=-1;
  QPointF ld_point;
  QPolygonF points;
  while(!ls->atEnd()){
    if(ls->isStartElement()){
      if(ls->name() == "electrode_poly")
        ls->readNext();
      else if(ls->name() == "layer_id"){
        lay_id = ls->readElementText().toInt();
        ls->readNext();
      }
      else if(ls->name() == "vertex"){
        for(QXmlStreamAttribute &attr : ls->attributes()){
          if(attr.name().toString() == QLatin1String("x"))
            ld_point.setX(attr.value().toFloat()*scale_factor); //convert from angstrom to pixel
          else if(attr.name().toString() == QLatin1String("y"))
            ld_point.setY(attr.value().toFloat()*scale_factor);
        }
        points.append(ld_point);
        ls->readNext();
      }
      // else if(ls->name() == "property_map"){
      //   propMapFromXml(ls);
      //   ls->readNext();
      // }
      // TODO the rest of the variables
      else{
        qDebug() << QObject::tr("ElectrodePoly: invalid element encountered on line %1 - %2").arg(ls->lineNumber()).arg(ls->name().toString());
        ls->readNext();
      }
    }
    else if(ls->isEndElement()){
      // break out of ls if the end of this element has been reached
      if(ls->name() == "electrode_poly"){
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
    qWarning() << "ElectrodePoly lay_id is at" << lay_id << ", should be at 2.";
  }
  if(ld_point.isNull()){
    qWarning() << "ld_point is null";
  }
  //load all read data into init_electrode
  // setPolygon(points);
  // qDebug() << points;
  // setLayerIndex(lay_id);
  // setPolygon(poly_in);
  // QRectF rect = points.boundingRect();
  // setRect(rect, true);
  // update();
  // setZValue(-1);
  // setFlag(QGraphicsItem::ItemIsSelectable, true);
  // createHandles();
  // initResizablePoly(lay_id);

  // initResizablePoly(lay_id, points);
  initElectrodePoly(lay_id, points);
  scene->addItem(this);
  qDebug() << "POLY MADE";
}





prim::ElectrodePoly::~ElectrodePoly()
{
}

void prim::ElectrodePoly::initElectrodePoly(int lay_id, QPolygonF poly_in)
{
  initResizablePoly(lay_id, poly_in);
  createActions();
  constructStatics();
  update();
  // setZValue(-1);

}

void prim::ElectrodePoly::saveItems(QXmlStreamWriter *ss) const
{
  ss->writeStartElement("electrode_poly");
  // layer id
  ss->writeTextElement("layer_id", QString::number(layer_id));

  QPolygonF poly_save = getTranslatedPolygon();

  for (QPointF vertex: poly_save) {
    ss->writeEmptyElement("vertex");
    ss->writeAttribute("x", QString::number(vertex.x()/scale_factor)); //convert to angstrom
    ss->writeAttribute("y", QString::number(vertex.y()/scale_factor));
  }
  ss->writeTextElement("pixel_per_angstrom", QString::number(scale_factor));
  ss->writeStartElement("property_map");
  gui::PropertyMap::writeValuesToXMLStream(properties(), ss);
  // other attributes
  // ......
  ss->writeEndElement();
  ss->writeEndElement();
}

QRectF prim::ElectrodePoly::boundingRect() const
{
  return QRectF(0,0, getPolygon().boundingRect().width(), getPolygon().boundingRect().height());
}

void prim::ElectrodePoly::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  painter->setPen(QPen(edge_col, edge_width));
  painter->setBrush(fill_col.isValid() ? fill_col : Qt::NoBrush);
  painter->drawPolygon(getPolygon());
  if(tool_type == gui::SelectTool && isSelected()){
    painter->setPen(Qt::NoPen);
    painter->setBrush(selected_col);
    painter->drawPolygon(getPolygon());
  }
}

void prim::ElectrodePoly::showProps()
{
  prim::Emitter::instance()->sig_showProperty(this);
}

void prim::ElectrodePoly::performAction(QAction *action)
{
  //switch case doesnt work on non-ints, use if else.
  if (action->text() == action_show_prop->text()) {
    showProps();
  } else {
    qDebug() << QObject::tr("Matched no action.");
  }
}

void prim::ElectrodePoly::createActions()
{
  action_show_prop = new QAction(QObject::tr("Show properties"));
  actions_list.append(action_show_prop);
}


prim::Item *prim::ElectrodePoly::deepCopy() const
{
  QPolygonF new_poly = getPolygon();
  //return fresh polygon with unshifted coordinates.
  new_poly.translate(sceneRect().topLeft());
  prim::ElectrodePoly *ep = new ElectrodePoly(new_poly, layer_id);
  return ep;
}

void prim::ElectrodePoly::constructStatics() //needs to be changed to look at electrode settings instead.
{
  default_class_properties.readPropertiesFromXML(":/properties/electrode.xml");
  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  edge_width = gui_settings->get<qreal>("electrodepoly/edge_width");
  edge_col= gui_settings->get<QColor>("electrodepoly/edge_col");
  fill_col= gui_settings->get<QColor>("electrodepoly/fill_col");
  selected_col= gui_settings->get<QColor>("electrodepoly/selected_col");
}
