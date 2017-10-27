// @file:     electrode.cc
// @author:   Nathan
// @created:  2017.10.27
// @editted:  2017.10.27 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     Electrode classes

#include "electrode.h"
#include "src/settings/settings.h"



// Initialize statics

qreal prim::Electrode::diameter = -1;
qreal prim::Electrode::edge_width = -1;

QColor prim::Electrode::edge_col;
QColor prim::Electrode::fill_col;

qreal prim::Electrode::in_fill;
QColor prim::Electrode::in_fill_col;


// Draw on layer 0 for now.
prim::Electrode::Electrode(int lay_id, QPoint p1, QPoint p2):
  prim::Item(prim::Item::Electrode, lay_id), QPoint p1, QPoint p2,
{
  // construct static class variables
  if(diameter < 0)
    constructStatics();

  // set dot location in pixels
  setPos(p_loc*scale_factor);

  // flags
  setFlag(QGraphicsItem::ItemIsSelectable, true);
}

void prim::LatticeDot::setDBDot(prim::DBDot *dot)
{
  setFlag(QGraphicsItem::ItemIsSelectable, dot==0);
  dbdot=dot;
}


QRectF prim::LatticeDot::boundingRect() const
{
  qreal width = diameter+edge_width;
  return QRectF(-.5*width, -.5*width, width, width);
}


// NOTE: nothing in this paint method changes... possibly cache background as
// pre-rendered bitma for speed.
void prim::LatticeDot::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  if(dbdot==0){
    QRectF rect = boundingRect();
    qreal dxy = .5*edge_width;
    rect.adjust(dxy,dxy,-dxy,-dxy);

    // draw outer circle
    painter->setPen(QPen(edge_col, edge_width));
    painter->setBrush(fill_col.isValid() ? fill_col : Qt::NoBrush);
    painter->drawEllipse(rect);

    // draw inner circle
    if(!select_mode && isSelected()){
        QPointF center = rect.center();
        QSizeF size(diameter, diameter);
        rect.setSize(size*in_fill);
        rect.moveCenter(center);

        painter->setPen(Qt::NoPen);
        painter->setBrush(in_fill_col);
        painter->drawEllipse(rect);
    }
  }
}

prim::Item *prim::LatticeDot::deepCopy() const
{
  return new prim::LatticeDot(layer_id, phys_loc);
}


void prim::LatticeDot::constructStatics()
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();

  diameter = gui_settings->get<qreal>("latdot/diameter")*scale_factor;
  edge_width = gui_settings->get<qreal>("latdot/edge_width")*diameter;
  edge_col= gui_settings->get<QColor>("latdot/edge_col");
  fill_col= gui_settings->get<QColor>("latdot/fill_col");
  in_fill = gui_settings->get<qreal>("latdot/inner_fill");
  in_fill_col = gui_settings->get<QColor>("latdot/inner_fill_col");
}
