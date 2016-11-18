#ifndef _GUI_PR_DBDOT_H_
#define _GUI_PR_DBDOT_H_

#include <QObject>
#include <QPointF>
#include <QPainter>
#include <QGraphicsItem>

namespace prim{


class DBDot: public QGraphicsItem
{
public:

  // constructor
  DBDot(QPointF p_loc);

  // destructor
  ~DBDot();

  // geometry
  QRectF boundingRect() const;

  // painting
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);


protected:

  // events

  // void mousePressEvent(QGraphicsSceneMouseEvent *e);
  // void mouseReleaseEvent(QGraphicsSceneMouseEvent *e);


private:

  QPointF phys_loc;

  QColor edge_col;
  qreal fill_fact; // area proportion of dot filled
  QColor fill_col;

  // immutable parameters
  qreal scale_fact; // pixels per angstrom for dot locations
  qreal diameter;   // diameter of dot in pixels
  qreal edge_width; // edge pen width in pixels
};


} // end prim namespace


#endif
