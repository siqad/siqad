// @file:     pot_plot.cc
// @author:   Nathan
// @created:  2017.10.27
// @editted:  2017.10.27 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     pot_plot classes

#include <algorithm>
#include "pot_plot.h"
#include "src/settings/settings.h"

// Initialize statics
qreal prim::PotPlot::edge_width = -1;

QColor prim::PotPlot::edge_col;
QColor prim::PotPlot::fill_col;
QColor prim::PotPlot::selected_col; // edge colour, selected

// Draw on layer 0 for now.
prim::PotPlot::PotPlot(int lay_id, QPixmap potential_plot, QRectF graph_container):
  prim::Item(prim::Item::PotPlot)
{
  initPotPlot(lay_id, potential_plot, graph_container);
}

void prim::PotPlot::initPotPlot(int lay_id, QPixmap potential_plot_in, QRectF graph_container_in)
{
  layer_id = lay_id;
  potential_plot = potential_plot_in;
  graph_container = graph_container_in;
  constructStatics();
  setZValue(-1);
  setPos(graph_container.topLeft());
  // flags
  setFlag(QGraphicsItem::ItemIsSelectable, true);
}

QRectF prim::PotPlot::boundingRect() const
{
  qreal width = graph_container.width();
  qreal height = graph_container.height();
  return QRectF(0, 0, width, height);
}

//Actually draw the picture into the rectangle.
void prim::PotPlot::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  painter->setOpacity(0.5);
  QRectF graph_container_draw = QRectF(qreal(0),qreal(0),graph_container.width(), graph_container.height());
  painter->drawPixmap(graph_container_draw, potential_plot.scaled(graph_container.width(), graph_container.height()), graph_container_draw);
  painter->setOpacity(1);
}

prim::Item *prim::PotPlot::deepCopy() const
{
  prim::PotPlot *pp = new PotPlot(layer_id, potential_plot, graph_container);
  return pp;
}

void prim::PotPlot::constructStatics() //needs to be changed to look at electrode settings instead.
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  edge_width = gui_settings->get<qreal>("potplot/edge_width");
  edge_col= gui_settings->get<QColor>("potplot/edge_col");
  fill_col= gui_settings->get<QColor>("potplot/fill_col");
  selected_col= gui_settings->get<QColor>("potplot/selected_col");
}
