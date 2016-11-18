#include "dbdot.h"
#include "src/settings/settings.h"

#include <cmath>
#include <QSizeF>


prim::DBDot::DBDot(QPointF p_loc)
{
  settings::GUISettings gui_settings;

  scale_fact = gui_settings.get<qreal>("dbdot/scale_fact");
  diameter = gui_settings.get<qreal>("dbdot/diameter")*scale_fact;
  edge_width = gui_settings.get<qreal>("dbdot/edge_width")*diameter;

  setPos(p_loc);

  phys_loc = p_loc;

  edge_col = gui_settings.get<QColor>("dbdot/edge_col");

  fill_fact = 0;
  fill_col = gui_settings.get<QColor>("dbdot/fill_col");
}

prim::DBDot::~DBDot()
{}


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

  painter->setPen(QPen(edge_col, edge_width));
  //painter->setBrush(Qt::NoBrush);
  painter->drawEllipse(rect);

  // draw inner fill
  if(fill_fact>0){
    QPointF center = rect.center();
    QSizeF size(diameter, diameter);
    rect.setSize(size*sqrt(fill_fact));
    rect.moveCenter(center);

    painter->setPen(Qt::NoPen);
    painter->setBrush(fill_col);
    painter->drawEllipse(rect);
  }
}
