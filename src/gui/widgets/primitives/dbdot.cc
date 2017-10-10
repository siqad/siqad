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

qreal prim::DBDot::diameter = -1;
qreal prim::DBDot::edge_width = -1;

QColor prim::DBDot::edge_col;
QColor prim::DBDot::selected_col;




prim::DBDot::DBDot(int lay_id, prim::LatticeDot *src, int elec_in)
  : prim::Item(prim::Item::DBDot)
{
  initDBDot(lay_id, src, elec_in);
  // TODO might want to add a struct or something that stores all properties of the db, this way copies can be much easier
}


prim::DBDot::DBDot(QXmlStreamReader *stream, QGraphicsScene *scene)
  : prim::Item(prim::Item::DBDot)
{
  QPointF scene_loc; // physical location from file
  int lay_id; // layer id from file
  int elec_in;

  while(!stream->atEnd()){
    if(stream->isStartElement()){
      if(stream->name() == "layer_id"){
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
  //fill_col = fill_col_default;
  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  fill_col_drv = gui_settings->get<QColor>("dbdot/fill_col_drv");
  fill_col_drv_sel = gui_settings->get<QColor>("dbdot/fill_col_drv_sel");

  setLayerIndex(lay_id);

  // construct static class variables
  if(diameter<0)
    constructStatics();

  // set dot location in pixels
  setSource(src);

  // set electron occupation
  setElec(elec_in);

  // flags
  setFlag(QGraphicsItem::ItemIsSelectable, true);
}


void prim::DBDot::toggleElec() {
  if(elec) 
    setElec(0); 
  else 
    setElec(1);
}


void prim::DBDot::setElec(int e_in) {
  elec = e_in;
  if(elec){
    // set to 1
    setFill(1);
    setFillCol(fill_col_drv);
  }
  else{
    // set to 0
    setFill(0);
    setFillCol(fill_col_default);
  }
  update();
}


void prim::DBDot::setSource(prim::LatticeDot *src)
{
  if(src){
    // unset the previous LatticeDot
    if(source)
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
  return QRectF(-.5*width, -.5*width, width, width);
}


void prim::DBDot::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  QRectF rect = boundingRect();
  qreal dxy = .5*edge_width;
  rect.adjust(dxy,dxy,-dxy,-dxy);

  // draw inner fill
  if(fill_fact>0){
    QPointF center = rect.center();
    QSizeF size(diameter, diameter);
    rect.setSize(size*fill_fact);
    rect.moveCenter(center);

    painter->setPen(Qt::NoPen);
    painter->setBrush(fill_col);
    painter->drawEllipse(rect);
  }

  // draw outer circle
  painter->setPen(QPen((select_mode && upSelected()) ? selected_col : edge_col, edge_width));
  painter->drawEllipse(rect);

}


prim::Item *prim::DBDot::deepCopy() const
{
  prim::DBDot *cp = new DBDot(layer_id, 0, elec);
  cp->setPos(pos());
  return cp;
}


void prim::DBDot::saveItems(QXmlStreamWriter *stream) const
{
  /*stream->writeEmptyElement("dbdot");
  stream->writeAttribute("x", QString::number(getPhysLoc().x()));
  stream->writeAttribute("y", QString::number(getPhysLoc().y()));
  stream->writeAttribute("layer_id", QString::number(layer_id));
  stream->writeAttribute("elec", QString::number(elec));*/
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

  diameter = gui_settings->get<qreal>("dbdot/diameter")*scale_factor;
  edge_width = gui_settings->get<qreal>("dbdot/edge_width")*diameter;
  edge_col= gui_settings->get<QColor>("dbdot/edge_col");
  selected_col= gui_settings->get<QColor>("dbdot/selected_col");
  fill_col_default = gui_settings->get<QColor>("dbdot/fill_col");
  fill_col_drv = gui_settings->get<QColor>("dbdot/fill_col_drv");
  fill_col_drv_sel = gui_settings->get<QColor>("dbdot/fill_col_drv_sel");

}

void prim::DBDot::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  qDebug() << QObject::tr("DBDot has seen the mousePressEvent");

  switch(e->buttons()){
    case Qt::RightButton:
      toggleElec(); // for now, right click toggles electron. In the future, show context menu with electron toggle being one option
      break;
    default:
      prim::Item::mousePressEvent(e);
      break;
  }
}
