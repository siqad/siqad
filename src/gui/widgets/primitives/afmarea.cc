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
gui::PropertyMap AFMArea::default_class_properties;
qreal AFMArea::area_border_width=-1;
prim::Item::StateColors AFMArea::area_border_col;
prim::Item::StateColors AFMArea::area_fill_col;
qreal AFMArea::scan_path_width;
prim::Item::StateColors AFMArea::scan_path_fill_col;


// Normal constructor
AFMArea::AFMArea(int lay_id, const QRectF &scene_rect, bool orientation,
    float z_spd, float h_spd, float v_spd, float v_disp)
  : prim::ResizableRect(prim::Item::AFMArea)
{
  initAFMArea(lay_id, scene_rect, orientation, z_spd, h_spd, v_spd, v_disp);
}

// Command constructor
AFMArea::AFMArea(int lay_id, QStringList points, bool orientation,
    float z_spd, float h_spd, float v_spd, float v_disp)
  : prim::ResizableRect(prim::Item::AFMArea)
{
  float xmin = std::min(points[0].toFloat(), points[2].toFloat())*scale_factor;
  float xmax = std::max(points[0].toFloat(), points[2].toFloat())*scale_factor;
  float ymin = std::min(points[1].toFloat(), points[3].toFloat())*scale_factor;
  float ymax = std::max(points[1].toFloat(), points[3].toFloat())*scale_factor;
  QRectF scene_rect = QRectF(QPointF(xmin, ymin), QPointF(xmax,ymax));
  initAFMArea(lay_id, scene_rect, orientation, z_spd, h_spd, v_spd, v_disp);
}


// Load XML constructor
AFMArea::AFMArea(QXmlStreamReader *rs, QGraphicsScene *scene, int lay_id)
  : prim::ResizableRect(prim::Item::AFMArea)
{
  QPointF point1, point2;
  bool orientation;
  float z_spd, h_spd, v_spd, v_disp;

  while (!rs->atEnd()) {
    if (rs->isStartElement()) {
      if (rs->name() == "afmarea") {
        // do nothing
      } else if (rs->name() == "layer_id") {
        qDebug() << QObject::tr("The layer_id tag in designs are no longer used in loading. Using the lay_id supplied to the constructor instead.");
      } else if (rs->name() == "dimensions") {
        for (QXmlStreamAttribute &attr : rs->attributes()) {
          if (attr.name().toString() == QLatin1String("x1"))
            point1.setX(scale_factor*attr.value().toFloat());
          else if (attr.name().toString() == QLatin1String("y1"))
            point1.setY(scale_factor*attr.value().toFloat());
          else if (attr.name().toString() == QLatin1String("x2"))
            point2.setX(scale_factor*attr.value().toFloat());
          else if (attr.name().toString() == QLatin1String("y2"))
            point2.setY(scale_factor*attr.value().toFloat());
        }
      } else if (rs->name() == "h_orientation") {
        orientation = static_cast<bool>(rs->readElementText().toInt());
      } else if (rs->name() == "z_speed") {
        z_spd = rs->readElementText().toFloat();
      } else if (rs->name() == "h_speed") {
        h_spd = rs->readElementText().toFloat();
      } else if (rs->name() == "v_speed") {
        v_spd = rs->readElementText().toFloat();
      } else if (rs->name() == "v_displacement") {
        v_disp = rs->readElementText().toFloat();
      } else {
        qDebug() << QObject::tr("AFMArea: invalid element encountered on line "
            "%1 - %2").arg(rs->lineNumber()).arg(rs->name().toString());
      }
      rs->readNext();
    } else if (rs->isEndElement()) {
      // break out of stream if the end of the element has been reached
      if (rs->name() == "afmarea") {
        rs->readNext();
        break;
      }
      rs->readNext();
    } else {
      rs->readNext();
    }
  }

  if (rs->hasError())
    qCritical() << QObject::tr("XML error: ") << rs->errorString().data();

  initAFMArea(lay_id, QRectF(point1, point2), orientation, z_spd, h_spd, v_spd, v_disp);
  scene->addItem(this);
}

// Common initialization actions
void AFMArea::initAFMArea(int lay_id, const QRectF &scene_rect,
    bool orientation, float z_spd, float h_spd, float v_spd, float v_disp)
{
  layer_id = lay_id;

  // prepare static variables if they haven't been prepared
  if (area_border_width == -1)
    prepareStatics();

  createActions();

  h_orientation = orientation;
  z_speed = z_spd;
  h_speed = h_spd;
  v_speed = v_spd;
  v_displacement = v_disp;

  // GUI
  setSceneRect(scene_rect);
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setAcceptHoverEvents(true);
  setResizable(true);
}

// Save to XML
void AFMArea::saveItems(QXmlStreamWriter *ws) const
{
  ws->writeStartElement("afmarea");

  ws->writeTextElement("layer_id", QString::number(layer_id));

  // dimensions
  ws->writeEmptyElement("dimensions");
  ws->writeAttribute("x1", QString::number(sceneRect().topLeft().x()/scale_factor));
  ws->writeAttribute("y1", QString::number(sceneRect().topLeft().y()/scale_factor));
  ws->writeAttribute("x2", QString::number(sceneRect().bottomRight().x()/scale_factor));
  ws->writeAttribute("y2", QString::number(sceneRect().bottomRight().y()/scale_factor));

  // tip parameters
  ws->writeTextElement("h_orientation", QString::number(horizontalOrientation()));
  ws->writeTextElement("z_speed", QString::number(zSpeed()));
  ws->writeTextElement("h_speed", QString::number(horizontalSpeed()));
  ws->writeTextElement("v_speed", QString::number(verticalSpeed()));
  ws->writeTextElement("v_displacement", QString::number(
      verticalDisplacementBetweenScans()));

  // end of afmarea
  ws->writeEndElement();
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
  QPointF diag = sceneRect().bottomRight() - sceneRect().topLeft();
  return QRectF(0, 0, diag.x(), diag.y());
}

void AFMArea::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  // draw area border and background
  QRectF rect = boundingRect();
  qreal dxy = .5*area_border_width;
  rect.adjust(dxy, dxy, -dxy, -dxy);  // trim off the border from bounding rect
  painter->setPen(QPen(getCurrentStateColor(area_border_col), area_border_width));
  painter->setBrush(getCurrentStateColor(area_fill_col));
  painter->drawRect(rect);

  // TODO draw AFM scan path preview
}

Item *AFMArea::deepCopy() const
{
  return new prim::AFMArea(layer_id, sceneRect(),
      h_orientation, z_speed, h_speed, v_speed, v_displacement);
}


void AFMArea::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  prim::Item::mousePressEvent(e);
}


void prim::AFMArea::showProps()
{
  prim::Emitter::instance()->sig_showProperty(this);
}


void prim::AFMArea::performAction(QAction *action)
{
  //switch case doesnt work on non-ints, use if else.
  if (action->text() == action_show_prop->text()) {
    showProps();
  } else {
    qDebug() << QObject::tr("Matched no action.");
  }
}


void prim::AFMArea::createActions()
{
  action_show_prop = new QAction(QObject::tr("Show properties"));
  actions_list.append(action_show_prop);
}

void AFMArea::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
  setHovered(true);
  update();
}

void AFMArea::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
  setHovered(false);
  update();
}

void AFMArea::prepareStatics()
{
  // Read properties
  default_class_properties.readPropertiesFromXML(":/properties/afmarea.xml");

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
