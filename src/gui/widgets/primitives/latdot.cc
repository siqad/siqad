// @file:     latdot.cc
// @author:   Jake
// @created:  2017.05.03
// @editted:  2017.06.07  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Implementation of LatticeDot class


#include "latdot.h"
#include "src/settings/settings.h"



// Initialize statics

qreal prim::LatticeDot::diameter = -1;
qreal prim::LatticeDot::edge_width = -1;

prim::Item::StateColors prim::LatticeDot::edge_col;
prim::Item::StateColors prim::LatticeDot::fill_col;

qreal prim::LatticeDot::in_fill;
prim::Item::StateColors prim::LatticeDot::in_fill_col;


// the surface lattice will always be layer 0
prim::LatticeDot::LatticeDot(int lay_id, QPointF p_loc):
  prim::Item(prim::Item::LatticeDot, lay_id), phys_loc(p_loc), dbdot(0)
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
    painter->setPen(QPen(getCurrentStateColor(edge_col), edge_width));
    painter->setBrush(getCurrentStateColor(fill_col));
    painter->drawEllipse(rect);

    // draw inner circle
    if(tool_type == gui::ToolType::DBGenTool && isSelected()){
        QPointF center = rect.center();
        QSizeF size(diameter, diameter);
        rect.setSize(size*in_fill);
        rect.moveCenter(center);

        painter->setPen(Qt::NoPen);
        painter->setBrush(getCurrentStateColor(in_fill_col));
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

  edge_col.normal = gui_settings->get<QColor>("latdot/edge_col");
  edge_col.selected = gui_settings->get<QColor>("latdot/edge_col");
  edge_col.hovered = gui_settings->get<QColor>("latdot/edge_col");
  edge_col.high_contrast = gui_settings->get<QColor>("latdot/edge_col_hc");

  fill_col.normal = gui_settings->get<QColor>("latdot/fill_col");
  fill_col.selected = gui_settings->get<QColor>("latdot/fill_col");
  fill_col.hovered = gui_settings->get<QColor>("latdot/fill_col");
  fill_col.high_contrast = gui_settings->get<QColor>("latdot/fill_col_hc");

  in_fill = gui_settings->get<qreal>("latdot/inner_fill");
  in_fill_col.normal = gui_settings->get<QColor>("latdot/inner_fill_col");
  in_fill_col.selected = gui_settings->get<QColor>("latdot/inner_fill_col");
  in_fill_col.hovered = gui_settings->get<QColor>("latdot/inner_fill_col");
  in_fill_col.high_contrast = gui_settings->get<QColor>("latdot/inner_fill_col_hc");
}
