// @file:     dbdot.cc
// @author:   Jake
// @created:  2016.11.15
// @editted:  2017.06.07  - Jake
// @license:  GNU LGPL v3
//
// @desc:     DBDot implementation


#include "dbdot.h"
#include "src/settings/settings.h"

// Initialize statics

qreal prim::DBDot::diameter_m = -1;
qreal prim::DBDot::diameter_l;
qreal prim::DBDot::edge_width;
qreal prim::DBDot::publish_scale;

prim::Item::StateColors prim::DBDot::fill_col;           // normal dbdot
prim::Item::StateColors prim::DBDot::fill_col_driver;    // driver (forced polarization)
prim::Item::StateColors prim::DBDot::fill_col_electron;  // contains electron
prim::Item::StateColors prim::DBDot::edge_col;           // edge of the dbdot
prim::Item::StateColors prim::DBDot::edge_col_driver;    // edge of the dbdot
prim::Item::StateColors prim::DBDot::edge_col_electron;  // edge of the dbdot


prim::DBDot::DBDot(int lay_id, prim::LatticeDot *src, int elec_in)
  : prim::Item(prim::Item::DBDot), show_elec(0)
{
  initDBDot(lay_id, src, elec_in);
}


prim::DBDot::DBDot(QXmlStreamReader *stream, QGraphicsScene *scene)
  : prim::Item(prim::Item::DBDot)
{
  QPointF scene_loc; // physical location from file
  int lay_id=-1; // layer id from file
  int elec_in=0;

  while(!stream->atEnd()){
    if(stream->isStartElement()){
      if(stream->name() == "dbdot")
        stream->readNext();
      else if(stream->name() == "layer_id"){
        lay_id = stream->readElementText().toInt();
        stream->readNext();
      }
      else if(stream->name() == "elec"){
        elec_in = stream->readElementText().toInt();
        stream->readNext();
      }
      else if(stream->name() == "physloc"){
        for(QXmlStreamAttribute &attr : stream->attributes()){
          if(attr.name().toString() == QLatin1String("x"))
            scene_loc.setX(scale_factor*attr.value().toFloat());
          else if(attr.name().toString() == QLatin1String("y"))
            scene_loc.setY(scale_factor*attr.value().toFloat());
        }
        stream->readNext();
      }
      else{
        qDebug() << QObject::tr("DBDot: invalid element encountered on line %1 - %2").arg(stream->lineNumber()).arg(stream->name().toString());
        stream->readNext();
      }
    }
    else if(stream->isEndElement()){
      // break out of stream if the end of this element has been reached
      if(stream->name() == "dbdot"){
        stream->readNext();
        break;
      }
      stream->readNext();
    }
    else
      stream->readNext();
  }

  if(stream->hasError())
    qCritical() << QObject::tr("XML error: ") << stream->errorString().data();

  // find the lattice dot located at scene_loc
  prim::LatticeDot *src_latdot = static_cast<prim::LatticeDot*>(scene->itemAt(scene_loc, QTransform()));
  if(!src_latdot){
    qCritical() << QObject::tr("No lattice dot at %1, %2").arg(scene_loc.x()).arg(scene_loc.y());
    // TODO error alert dialog?
  }

  // initialize
  initDBDot(lay_id, src_latdot, elec_in);
  scene->addItem(this);
}


void prim::DBDot::initDBDot(int lay_id, prim::LatticeDot *src, int elec_in)
{
  fill_fact = 0.;

  setLayerIndex(lay_id);

  // construct static class variables
  if(diameter_m<0)
    constructStatics();

  diameter = diameter_m;

  // set dot location in pixels
  setSource(src);

  // set electron occupation
  setElec(elec_in);

  // flags
  setFlag(QGraphicsItem::ItemIsSelectable, true);
}


void prim::DBDot::toggleElec()
{
  if(elec)
    setElec(0);
  else
    setElec(1);
}


void prim::DBDot::setElec(int e_in)
{
  elec = e_in;
  update();
}


void prim::DBDot::setShowElec(float se_in)
{
  show_elec = se_in;
  update();
}


void prim::DBDot::setSource(prim::LatticeDot *src)
{
  if(src){
    // unset the previous LatticeDot
    if(source && source->getDBDot() == this)
      source->setDBDot(0);

    // move to new LatticeDot
    src->setDBDot(this);
    source=src;
    phys_loc = src->getPhysLoc();
    setPos(src->pos());
  }
}


QRectF prim::DBDot::boundingRect() const
{
  qreal width = diameter+edge_width;
  if (display_mode == gui::ScreenshotMode)
    width *= publish_scale;
  return QRectF(-.5*width, -.5*width, width, width);
}


void prim::DBDot::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  QColor fill_col_state;
  QColor edge_col_state;
  if (display_mode == gui::SimDisplayMode ||
      display_mode == gui::ScreenshotMode) {
  //if ( (display_mode == gui::SimDisplayMode ||
  //      display_mode == gui::ScreenshotMode) &&
  //      show_elec > 0) {
    setFill(show_elec);
    diameter = diameter_l;
    fill_col_state = getCurrentStateColor(fill_col_electron);
    edge_col_state = getCurrentStateColor(edge_col_electron);
  } else if (elec > 0) {
    setFill(1);
    diameter = diameter_m;
    fill_col_state = getCurrentStateColor(fill_col_driver);
    edge_col_state = getCurrentStateColor(edge_col_driver);
  } else {
    setFill(1);
    diameter = diameter_m;
    fill_col_state = getCurrentStateColor(fill_col);
    edge_col_state = getCurrentStateColor(edge_col);
  }

  qreal edge_width_paint = edge_width;
  qreal diameter_paint = diameter;
  if (display_mode == gui::ScreenshotMode) {
    edge_width_paint *= publish_scale;
    diameter_paint *= publish_scale;
  }


  QRectF rect = boundingRect();
  //qreal dxy = .45*edge_width_paint;
  //rect.adjust(dxy,dxy,-dxy,-dxy);

  // draw inner fill
  if(fill_fact>0){
    QPointF center = rect.center();
    QSizeF size(diameter_paint, diameter_paint);
    rect.setSize(size*qSqrt(fill_fact));
    //rect.setSize(size*qSqrt(fill_fact));
    rect.moveCenter(center);

    painter->setPen(Qt::NoPen);
    painter->setBrush(fill_col_state);
    painter->drawEllipse(rect);
  }

  // draw outer circle
  painter->setPen(QPen(edge_col_state, edge_width_paint));
  painter->setBrush(Qt::NoBrush);
  //painter->drawEllipse(rect);
  painter->drawEllipse(boundingRect());
}


prim::Item *prim::DBDot::deepCopy() const
{
  prim::DBDot *cp = new DBDot(layer_id, 0, elec);
  cp->setPos(pos());
  return cp;
}


void prim::DBDot::saveItems(QXmlStreamWriter *stream) const
{
  stream->writeStartElement("dbdot");

  // layer id
  stream->writeTextElement("layer_id", QString::number(layer_id));

  // elec
  stream->writeTextElement("elec", QString::number(elec));

  // physical location
  stream->writeEmptyElement("physloc");
  stream->writeAttribute("x", QString::number(getPhysLoc().x()));
  stream->writeAttribute("y", QString::number(getPhysLoc().y()));

  stream->writeEndElement();
}


void prim::DBDot::constructStatics()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  diameter_m = gui_settings->get<qreal>("dbdot/diameter_m")*scale_factor;
  diameter_l = gui_settings->get<qreal>("dbdot/diameter_l")*scale_factor;
  edge_width = gui_settings->get<qreal>("dbdot/edge_width")*diameter_l;
  publish_scale = gui_settings->get<qreal>("dbdot/publish_scale");

  edge_col.normal = gui_settings->get<QColor>("dbdot/edge_col");
  edge_col.selected = gui_settings->get<QColor>("dbdot/edge_col_sel");
  edge_col.hovered = gui_settings->get<QColor>("dbdot/edge_col_hovered");
  edge_col.publish = gui_settings->get<QColor>("dbdot/edge_col_pb");

  fill_col.normal = gui_settings->get<QColor>("dbdot/fill_col");
  fill_col.selected = gui_settings->get<QColor>("dbdot/fill_col_sel");
  fill_col.hovered = gui_settings->get<QColor>("dbdot/fill_col_hovered");
  fill_col.publish = gui_settings->get<QColor>("dbdot/fill_col_pb");

  edge_col_driver.normal = gui_settings->get<QColor>("dbdot/edge_col_drv");
  edge_col_driver.selected = gui_settings->get<QColor>("dbdot/edge_col_drv_sel");
  edge_col_driver.hovered = gui_settings->get<QColor>("dbdot/edge_col_drv_hovered");
  edge_col_driver.publish = gui_settings->get<QColor>("dbdot/edge_col_drv_pb");

  fill_col_driver.normal = gui_settings->get<QColor>("dbdot/fill_col_drv");
  fill_col_driver.selected = gui_settings->get<QColor>("dbdot/fill_col_drv_sel");
  fill_col_driver.hovered = gui_settings->get<QColor>("dbdot/fill_col_drv_hovered");
  fill_col_driver.publish = gui_settings->get<QColor>("dbdot/fill_col_drv_pb");

  edge_col_electron.normal = gui_settings->get<QColor>("dbdot/edge_col_elec");
  edge_col_electron.selected = gui_settings->get<QColor>("dbdot/edge_col_elec_sel");
  edge_col_electron.hovered = gui_settings->get<QColor>("dbdot/edge_col_elec_hovered");
  edge_col_electron.publish = gui_settings->get<QColor>("dbdot/edge_col_elec_pb");

  fill_col_electron.normal = gui_settings->get<QColor>("dbdot/fill_col_elec");
  fill_col_electron.selected = gui_settings->get<QColor>("dbdot/fill_col_elec_sel");
  fill_col_electron.hovered = gui_settings->get<QColor>("dbdot/fill_col_elec_hovered");
  fill_col_electron.publish = gui_settings->get<QColor>("dbdot/fill_col_elec_pb");
}

void prim::DBDot::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  qDebug() << QObject::tr("DBDot (%1, %2) has seen the mousePressEvent").arg(x()).arg(y());
  //qDebug() << QObject::tr("Latdot of this DB (%1, %2)").arg(source->x()).arg(source->y());
  qDebug() << QObject::tr("lay_id: %1").arg(layer_id);
  switch(e->buttons()){
    // toggling currently handled by context menu.
    // case Qt::RightButton:
    //   if(designMode())
    //     // toggleElec(); // for now, right click toggles electron. In the future, show context menu with electron toggle being one option
    //   break;
    default:
      prim::Item::mousePressEvent(e);
      break;
  }
}
