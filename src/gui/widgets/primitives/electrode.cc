// @file:     electrode.cc
// @author:   Nathan
// @created:  2017.10.27
// @editted:  2017.10.27 - Nathan
// @license:  GNU LGPL v3
//
// @desc:     Electrode classes

#include <algorithm>
#include "electrode.h"
#include "src/settings/settings.h"



// Initialize statics
qreal prim::Electrode::edge_width = -1;

QColor prim::Electrode::edge_col;
QColor prim::Electrode::fill_col;
QColor prim::Electrode::selected_col; // edge colour, selected

// Draw on layer 0 for now.
prim::Electrode::Electrode(int lay_id, QPointF point1, QPointF point2):
  prim::Item(prim::Item::Electrode, lay_id), point1(point1), point2(point2)
{
  constructStatics();
  elec_width = (std::max(point1.x(), point2.x()) - std::min(point1.x(), point2.x()));
  elec_height = (std::max(point1.y(), point2.y()) - std::min(point1.y(), point2.y()));
  top_left.setX(std::min(point1.x(), point2.x()));
  top_left.setY(std::min(point1.y(), point2.y()));
  setZValue(-1);
  setPos(mapToScene(top_left).toPoint());
  // flags
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setFlag(QGraphicsItem::ItemIsFocusable, true);
  // setFlag(QGraphicsItem::ItemIsMovable, true);
}

QRectF prim::Electrode::boundingRect() const
{
  qreal width = elec_width+edge_width;
  qreal height = elec_height+edge_width;
  return QRectF(0, 0, width, height);
}

// NOTE: nothing in this paint method changes... possibly cache background as
// pre-rendered bitma for speed.
void prim::Electrode::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  QRectF rect = boundingRect();
  qreal dxy = .5*edge_width;
  rect.adjust(dxy,dxy,-dxy,-dxy); //make the bounding rectangle, and trim off the edges.
  // draw outer circle
  painter->setPen(QPen(edge_col, edge_width));
  painter->setBrush(fill_col.isValid() ? fill_col : Qt::NoBrush);
  painter->drawRect(rect);

  // draw inner circle
  if(select_mode && isSelected()){

      // qDebug() << QObject::tr("Electrode selected");
    setPos(pos());
    QPointF center = rect.center();
    QSizeF size(elec_width+edge_width, elec_height+edge_width);
    rect.setSize(size);
    rect.moveCenter(center);

    painter->setPen(Qt::NoPen);
    painter->setBrush(selected_col);
    painter->drawRect(rect);
  }
}

prim::Item *prim::Electrode::deepCopy() const
{
  prim::Electrode *elec = new Electrode(layer_id, point1, point2);
  elec->setPos(pos());
  return elec;
}

void prim::Electrode::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  switch(e->buttons()){
    default:
      prim::Item::mousePressEvent(e);
      break;
  }
}

void prim::Electrode::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e)
{
  // qDebug() << QObject::tr("Electrode has seen the mouseDoubleClickEvent");
  //do something here to manipulate potential. Maybe dialog box?
  // setpot(potential+1);
  // qDebug() << QObject::tr("mouse pos = %1, %2").arg(e->pos().x()).arg(e->pos().y());
  qDebug() << QObject::tr("Electrode potential: %1").arg(potential);

}

void prim::Electrode::setPotential(double givenPotential)
{
  if (givenPotential == givenPotential)//check for NULL argument
  {
    potential = givenPotential;
  }
}

void prim::Electrode::constructStatics() //needs to be changed to look at electrode settings instead.
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  edge_width = gui_settings->get<qreal>("electrode/edge_width");
  edge_col= gui_settings->get<QColor>("electrode/edge_col");
  fill_col= gui_settings->get<QColor>("electrode/fill_col");
  selected_col= gui_settings->get<QColor>("electrode/selected_col");
}
