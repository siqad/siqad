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

qreal prim::Electrode::in_fill;
QColor prim::Electrode::in_fill_col;

// Draw on layer 0 for now.
prim::Electrode::Electrode(int lay_id, QPointF p1, QPointF p2):
  prim::Item(prim::Item::Electrode, lay_id), p1(p1), p2(p2)
{
  constructStatics();
  elec_width = (std::max(p1.x(), p2.x()) - std::min(p1.x(), p2.x()));
  elec_height = (std::max(p1.y(), p2.y()) - std::min(p1.y(), p2.y()));
  topLeft.setX(std::min(p1.x(), p2.x()));
  topLeft.setY(std::min(p1.y(), p2.y()));
  setPos(mapToScene(topLeft).toPoint());
  // flags
  setFlag(QGraphicsItem::ItemIsSelectable, true);
}

QRectF prim::Electrode::boundingRect() const
{
  qreal width = elec_width+edge_width;
  qreal height = elec_height+edge_width;
  return QRectF(0, 0, width, height);
  // return QRectF(-0.5*width, -0.5*height, width, height);
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

//clocking
//fixed


prim::Item *prim::Electrode::deepCopy() const
{
  prim::Electrode *elec = new Electrode(layer_id, p1, p2);
  elec->setPos(pos());
  return elec;
}

void prim::Electrode::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  qDebug() << QObject::tr("Electrode has seen the mousePressEvent");

  switch(e->buttons()){
    case Qt::RightButton:
      qDebug() << QObject::tr("Electrode: User right clicked.");
      qDebug() << QObject::tr("item_type: %1").arg(item_type);
      setpot(potential-1);
      break;
    default:
      prim::Item::mousePressEvent(e);
      break;
  }
}

void prim::Electrode::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e)
{
  qDebug() << QObject::tr("Electrode has seen the mouseDoubleClickEvent");
  //do something here to manipulate potential. Maybe dialog box?
  setpot(potential+1);
}

void prim::Electrode::setpot(double pot_given)
{
  if (pot_given == pot_given)//check for NULL argument
  {
    potential = pot_given;
    qDebug() << QObject::tr("potential = %1").arg(potential);
  }
}

void prim::Electrode::constructStatics() //needs to be changed to look at electrode settings instead.
{
  settings::GUISettings *gui_settings = settings::GUISettings::instance();
  edge_width = gui_settings->get<qreal>("dbdot/edge_width");
  edge_col= gui_settings->get<QColor>("dbdot/edge_col");
  selected_col= gui_settings->get<QColor>("dbdot/selected_col");
  // edge_col= gui_settings->get<QColor>("latdot/edge_col");
  // fill_col= gui_settings->get<QColor>("latdot/fill_col");
  // in_fill = gui_settings->get<qreal>("latdot/inner_fill");
  // in_fill_col = gui_settings->get<QColor>("latdot/inner_fill_col");
}
