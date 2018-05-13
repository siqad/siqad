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


prim::DBDot::DBDot(prim::LatticeCoord l_coord, int lay_id)
  : prim::Item(prim::Item::DBDot), show_elec(0)
{
  initDBDot(l_coord, lay_id);
}


prim::DBDot::DBDot(QXmlStreamReader *rs, QGraphicsScene *)
  : prim::Item(prim::Item::DBDot)
{
  prim::LatticeCoord read_coord;
  int lay_id=-1;      // layer id from file

  while(!rs->atEnd()){
    if(rs->isStartElement()){
      if(rs->name() == "dbdot")
        rs->readNext();
      else if(rs->name() == "layer_id"){
        lay_id = rs->readElementText().toInt();
        rs->readNext();
      }
      else if(rs->name() == "latcoord"){
        for(QXmlStreamAttribute &attr : rs->attributes()){
          if (attr.name().toString() == QLatin1String("n"))
            read_coord.n = attr.value().toInt();
          else if (attr.name().toString() == QLatin1String("m"))
            read_coord.m = attr.value().toInt();
          else if (attr.name().toString() == QLatin1String("l"))
            read_coord.l = attr.value().toInt();
        }
        rs->readNext();
      }
      else{
        qDebug() << QObject::tr("DBDot: invalid element encountered on line %1 - %2").arg(rs->lineNumber()).arg(rs->name().toString());
        rs->readNext();
      }
    }
    else if(rs->isEndElement()){
      // break out of rs if the end of this element has been reached
      if(rs->name() == "dbdot"){
        rs->readNext();
        break;
      }
      rs->readNext();
    }
    else
      rs->readNext();
  }

  if(rs->hasError())
    qCritical() << QObject::tr("XML error: ") << rs->errorString().data();

  // initialize
  initDBDot(read_coord, lay_id);
  prim::Emitter::instance()->addItemToScene(this);
}


QPointF prim::DBDot::getPhysLoc() const
{
  // TODO implement
  return QPointF();
}


void prim::DBDot::setLatticeCoord(prim::LatticeCoord l_coord)
{
  lat_coord = l_coord;
  prim::Emitter::instance()->moveDBToLatticeCoord(this, lat_coord.n, lat_coord.m, lat_coord.l);
}


void prim::DBDot::initDBDot(prim::LatticeCoord coord, int lay_id)
{
  // construct static class variables
  if(diameter_m<0)
    constructStatics();

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
  //qreal dxy = .45*edge_width_paint;
  //rect.adjust(dxy,dxy,-dxy,-dxy);

  // draw inner fill
  if(fill_fact>0){
    QPointF center = rect.center();
    QSizeF size(diameter_paint, diameter_paint);
    rect.setSize(size*qSqrt(fill_fact));
    rect.moveCenter(center);

    painter->setPen(Qt::NoPen);
    painter->setBrush(fill_col_state);
    painter->drawEllipse(rect);
  }

  // draw outer circle
  painter->setPen(QPen(edge_col_state, edge_width_paint));
  painter->setBrush(Qt::NoBrush);
  painter->drawEllipse(rect);
}


prim::Item *prim::DBDot::deepCopy() const
{
  prim::DBDot *cp = new DBDot(lat_coord, layer_id);
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
