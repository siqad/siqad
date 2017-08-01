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




prim::DBDot::DBDot(int lay_id, prim::LatticeDot *src)
  : prim::Item(prim::Item::DBDot, lay_id), source(src)
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  // construct static class variables
  if(diameter<0)
    constructStatics();

  // set dot location in pixels
  if(src){
    phys_loc = src->getPhysLoc();
    setPos(src->pos());
    src->setDBDot(this);
  }

  fill_fact = 0.;
  fill_col = gui_settings->get<QColor>("dbdot/fill_col");

  // flags
  setFlag(QGraphicsItem::ItemIsSelectable, true);
}


void prim::DBDot::setSource(prim::LatticeDot *src)
{
  // unset the previous LatticeDot
  if(source)
    source->setDBDot(0);

  // move to new LatticeDot
  src->setDBDot(this);
  source=src;
  phys_loc = src->getPhysLoc();
  setPos(src->pos());
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

  // draw outer circle
  painter->setPen(QPen((select_mode && upSelected()) ? selected_col : edge_col, edge_width));
  painter->drawEllipse(rect);

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

}


prim::Item *prim::DBDot::deepCopy() const
{
  prim::DBDot *cp = new DBDot(layer_id, 0);
  cp->setPos(pos());
  return cp;
}


void prim::DBDot::saveToFile(QXmlStreamWriter *stream) const
{
  stream->writeStartElement("dbdot");

  // TODO layer id
  // TODO parent id (maybe not)
  // TODO self id

  // physical location
  stream->writeEmptyElement("physloc");
  stream->writeAttribute("x", QString::number(getPhysLoc().x()));
  stream->writeAttribute("y", QString::number(getPhysLoc().y()));

  stream->writeEndElement();
}


void prim::DBDot::loadFromFile(QXmlStreamReader *stream)
{
  QPointF p_loc; // physical location from file
  

  while(!stream->atEnd()){
    if(stream->isStartElement()){
      if(stream->name() == "parentid"){
        // TODO
        // figure out the layer pointer from here?
      }
      else if(stream->name() == "itemid"){
        // TODO
      }
      else if(stream->name() == "physloc"){
        for(QXmlStreamAttribute &attr : stream->attributes()){
          if(attr.name().toString() == QLatin1String("x")){
            p_loc.setX(attr.value().toFloat()); // TODO probably issue with type
          }
          else if(attr.name().toString() == QLatin1String("y")){
            p_loc.setY(attr.value().toFloat()); // TODO probably issue with type
          }
        }
      }
    }
    else if(stream->isEndElement())
      stream->readNext();
  }

  if(stream->hasError()){
    qCritical() << QObject::tr("XML error: ") << stream->errorString().data();
  }

  // TODO find the lattice dot located at p_loc
  // prim::LatticeDot latdot = ??

  //return new prim::DBDot(layer, latdot);
}


void prim::DBDot::constructStatics()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  diameter = gui_settings->get<qreal>("dbdot/diameter")*scale_factor;
  edge_width = gui_settings->get<qreal>("dbdot/edge_width")*diameter;
  edge_col= gui_settings->get<QColor>("dbdot/edge_col");
  selected_col= gui_settings->get<QColor>("dbdot/selected_col");

}
