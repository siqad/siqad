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

prim::PotPlot::PotPlot(QImage potential_plot, QRectF graph_container, QMovie *potential_animation):
  prim::Item(prim::Item::PotPlot)
{
  initPotPlot(potential_plot, graph_container, potential_animation);
}

prim::PotPlot::~PotPlot()
{
  // delete potential_animation;
  // delete gif_anim;
  // delete proxy;
}

void prim::PotPlot::initPotPlot(QImage potential_plot_in, QRectF graph_container_in, QMovie *potential_animation_in)
{
  potential_plot = potential_plot_in;
  graph_container = graph_container_in;
  potential_animation = potential_animation_in;
  potential_animation->setScaledSize(graph_container.size().toSize()/5.0);
  potential_animation->setSpeed(10);
  potential_animation->start();
  // gif_anim = new QLabel();
  gif_anim.setMovie(potential_animation);
  constructStatics();
  setZValue(-1);
  setPos(graph_container.topLeft());
  // flags
  setFlag(QGraphicsItem::ItemIsSelectable, false);
}

void prim::PotPlot::setProxy(QGraphicsProxyWidget *proxy_in)
{
  proxy = proxy_in;
  proxy->resize(graph_container.size()/5.0);
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
  painter->drawImage(graph_container_draw, potential_plot);
  // painter->drawPixmap(graph_container_draw, potential_plot.scaled(graph_container.width(), graph_container.height()), graph_container_draw);
  painter->setOpacity(1);
}

prim::Item *prim::PotPlot::deepCopy() const
{
  prim::PotPlot *pp = new PotPlot(potential_plot, graph_container, potential_animation);
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
