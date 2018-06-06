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
prim::Item::StateColors prim::DBDot::fill_col_electron;  // contains electron
prim::Item::StateColors prim::DBDot::edge_col;           // edge of the dbdot
prim::Item::StateColors prim::DBDot::edge_col_electron;  // edge of the dbdot


prim::DBDot::DBDot(prim::LatticeCoord l_coord, int lay_id, bool cp)
  : prim::Item(prim::Item::DBDot), show_elec(0)
{
  initDBDot(l_coord, lay_id, cp);
}


prim::DBDot::DBDot(QXmlStreamReader *rs, QGraphicsScene *)
  : prim::Item(prim::Item::DBDot)
{
  prim::LatticeCoord read_coord(0,0,-1);
  QPointF loc;
  int lay_id=-1;      // layer id from file

  while (rs->readNextStartElement()) {
    if (rs->name() == "layer_id") {
      lay_id = rs->readElementText().toInt();
    } else if (rs->name() == "latcoord") {
      read_coord.n = rs->attributes().value("n").toInt();
      read_coord.m = rs->attributes().value("m").toInt();
      read_coord.l = rs->attributes().value("l").toInt();
      rs->skipCurrentElement();
    } else if (rs->name() == "physloc") {
      loc.setX(rs->attributes().value("x").toFloat());
      loc.setY(rs->attributes().value("y").toFloat());
      qDebug() << QObject::tr("Read physloc of DB: (%1, %2)").arg(loc.x()).arg(loc.y());
      rs->skipCurrentElement();
    } else {
      qDebug() << QObject::tr("DBDot: invalid element encountered on line %1 - %2").arg(rs->lineNumber()).arg(rs->name().toString());
      rs->skipCurrentElement();
    }
  }

  // if no layer id is available, something is wrong
  if (lay_id == -1)
    qFatal("No layer id found for DBDot, aborting");

  // if lattice coord not available (legacy saves), use the physloc
  if (read_coord.l == -1) {
    if (!loc.isNull()) {
      int n,m,l;
      prim::Emitter::instance()->physLoc2LatticeCoord(loc, n, m, l);
      read_coord = prim::LatticeCoord(n,m,l); // TODO magic function
    } else {
      qFatal("Neither physical location nor lattice coordinates available when loading DB");
    }
  }

  // initialize
  initDBDot(read_coord, lay_id, false);
  prim::Emitter::instance()->addItemToScene(this);
}


void prim::DBDot::setLatticeCoord(prim::LatticeCoord l_coord)
{
  lat_coord = l_coord;
  prim::Emitter::instance()->moveDBToLatticeCoord(this, lat_coord.n, lat_coord.m, lat_coord.l);
}


void prim::DBDot::initDBDot(prim::LatticeCoord coord, int lay_id, bool cp)
{
  // construct static class variables
  if(diameter_m<0)
    constructStatics();

  if(cp)
    lat_coord = coord;
  else
    setLatticeCoord(coord);

  setLayerIndex(lay_id);
  fill_fact = 0.;
  diameter = diameter_m;

  // flags
  setFlag(QGraphicsItem::ItemIsSelectable, true);
}


void prim::DBDot::setShowElec(float se_in)
{
  show_elec = se_in;
  update();
}


QRectF prim::DBDot::boundingRect() const
{
  qreal width = diameter+2*edge_width;
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
    setFill(show_elec);
    diameter = diameter_l;
    fill_col_state = getCurrentStateColor(fill_col_electron);
    edge_col_state = getCurrentStateColor(edge_col_electron);
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
  qreal dxy = 0.5 * edge_width_paint;
  rect.adjust(dxy,dxy,-dxy,-dxy);

  // draw outer circle
  painter->setPen(QPen(edge_col_state, edge_width_paint));
  painter->setBrush(Qt::NoBrush);
  painter->drawEllipse(rect);

  // draw inner fill
  if(fill_fact>0){
    QRectF rect = boundingRect();
    QPointF center = rect.center();
    QSizeF size(diameter_paint, diameter_paint);
    rect.setSize(size*qSqrt(fill_fact));
    rect.moveCenter(center);

    painter->setPen(Qt::NoPen);
    painter->setBrush(fill_col_state);
    painter->drawEllipse(rect);
  }
}


prim::Item *prim::DBDot::deepCopy() const
{
  prim::DBDot *cp = new DBDot(lat_coord, layer_id, true);
  cp->setPos(pos());
  return cp;
}


void prim::DBDot::saveItems(QXmlStreamWriter *ws) const
{
  ws->writeStartElement("dbdot");

  // layer id
  ws->writeTextElement("layer_id", QString::number(layer_id));

  // physical location
  ws->writeEmptyElement("latcoord");
  ws->writeAttribute("n", QString::number(lat_coord.n));
  ws->writeAttribute("m", QString::number(lat_coord.m));
  ws->writeAttribute("l", QString::number(lat_coord.l));

  ws->writeEmptyElement("physloc");
  ws->writeAttribute("x", QString::number(physloc.x()));
  ws->writeAttribute("y", QString::number(physloc.y()));

  ws->writeEndElement();
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



// DBDotPreview Class

// Static variables
QColor prim::DBDotPreview::fill_col;
QColor prim::DBDotPreview::edge_col;

qreal prim::DBDotPreview::diameter=-1;
qreal prim::DBDotPreview::edge_width;
qreal prim::DBDotPreview::fill_fact;

prim::DBDotPreview::DBDotPreview(prim::LatticeCoord l_coord)
  : prim::Item(prim::Item::DBDotPreview)
{
  if (diameter == -1)
    constructStatics();

  lat_coord = l_coord;
}

QRectF prim::DBDotPreview::boundingRect() const
{
  qreal width = diameter + edge_width;
  return QRectF(-.5*width, -.5*width, width, width);
}

void prim::DBDotPreview::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  QRectF rect = boundingRect();
  // draw inner fill
  if (fill_fact > 0) {
    QPointF center = rect.center();
    QSizeF size(diameter, diameter);
    rect.setSize(size*fill_fact);
    rect.moveCenter(center);

    painter->setPen(Qt::NoPen);
    painter->setBrush(fill_col);
    painter->drawEllipse(rect);
  }

  // draw outer ring
  painter->setPen(QPen(edge_col, edge_width));
  painter->setBrush(Qt::NoBrush);
  painter->drawEllipse(rect);
}

void prim::DBDotPreview::constructStatics()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  fill_col = gui_settings->get<QColor>("dbdotprev/fill_col");
  edge_col = gui_settings->get<QColor>("dbdotprev/edge_col");

  diameter = gui_settings->get<qreal>("dbdotprev/diameter")*prim::Item::scale_factor;
  edge_width = gui_settings->get<qreal>("dbdotprev/edge_width")*diameter;
  fill_fact = gui_settings->get<qreal>("dbdotprev/fill_fact");
}
