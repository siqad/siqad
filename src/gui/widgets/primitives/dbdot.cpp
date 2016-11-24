#include "dbdot.h"
#include "src/settings/settings.h"

#include <cmath>
#include <QSizeF>


prim::DBDot::DBDot(prim::Emitter *em, QPointF p_loc, bool lat, DBDot *src):
  prim::MyItem(em)
{
  settings::GUISettings gui_settings;

  scale_fact = gui_settings.get<qreal>("dbdot/scale_fact");
  diameter = gui_settings.get<qreal>("dbdot/diameter")*scale_fact;

  setPos(p_loc*scale_fact);

  phys_loc = p_loc;
  source = src;
  lattice = lat;

  if(lattice){
    edge_width = gui_settings.get<qreal>("dbdot/lattice_edge_width")*diameter;
    edge_col = gui_settings.get<QColor>("dbdot/lattice_edge_col");
    selected_col = edge_col;
    fill_fact = 0.;
  }
  else{
    edge_width = gui_settings.get<qreal>("dbdot/edge_width")*diameter;
    edge_col = gui_settings.get<QColor>("dbdot/edge_col");
    selected_col = gui_settings.get<QColor>("dbdot/selected_col");
    fill_fact = 0.;
  }

  fill_col = gui_settings.get<QColor>("dbdot/fill_col");
}

prim::DBDot::~DBDot()
{
}


prim::DBDot *prim::DBDot::clone() const
{
  prim::DBDot *dot = new DBDot(emitter, phys_loc, lattice, 0);

  dot->setFlags(flags());
  dot->setPos(pos());
  dot->setZValue(zValue());

  return dot;
}



QRectF prim::DBDot::boundingRect() const
{
  settings::GUISettings gui_settings;

  qreal width = diameter+edge_width; // full width of rect

  // outer most edges
  return QRectF(-.5*width, -.5*width, width, width);
}


void prim::DBDot::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  settings::GUISettings gui_settings;

  // draw outer circle
  QRectF rect = boundingRect();

  if(isSelected()){
    painter->setPen(QPen(selected_col, edge_width));
  }
  else{
    painter->setPen(QPen(edge_col, edge_width));
  }
  //painter->setBrush(Qt::NoBrush);
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





// INTERRUPTS
