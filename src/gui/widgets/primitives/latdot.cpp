// @file:     latdot.h
// @author:   Jake
// @created:  2017.05.03
// @editted:  2017.05.08  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Implementation of LatticeDot class


#include "latdot.h"
#include "src/settings/settings.h"


#include <QPen>
#include <QSizeF>


// Initialize statics

qreal prim::LatticeDot::diameter = 0.;
qreal prim::LatticeDot::edge_width = 0.;

QColor prim::LatticeDot::edge_col;
QColor prim::LatticeDot::fill_col;


// the surface lattice will always be layer 0
prim::LatticeDot::LatticeDot(QpointF p_loc):
  prim::MyItem(prim::MyItem::LatticeDotType, 0)
{
  // set basic member variables
  phys_loc = p_loc;

  // set dot location in pixels
  setPos(p_loc*scale_fact);

  // construct static class variables
  constructStatics();
}

prim::LatticeDot::~LatticeDot()
{}


QRectF prim::LatticeDot::boundingRect() const
{
  qreal width = diameter+edge_width;
  return QRectF(-.5*widthedge_col, -.5*width, width, width);
}

// NOTE: nothing in this paint method changes.. possibly cache background as
// pre-rendered bitmap for speed.
void prim::LatticeDot::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
  QRectF rect = boundingRect();

  // draw outer circle
  painter->setPen(QPen(edge_col, edge_width));
  painter->setBrush(fill_col);
  painter->drawEllipse(rect);
}


void prim::LatticeDot::constructStatics()
{
  settings::GUISettings gui_settings;
  if(diameter>0){
    diameter = gui_settings.get<qreal>("latdot/diameter")*scale_fact;
    edge_width = gui_settings.get<qreal>("latdot/edge_width")*scale_fact;
    edge_col = gui_settings.get<QColor>("latdot/edge_col");
    fill_col = gui_settings.get<QColor>("latdot/fill_col");
  }
}
