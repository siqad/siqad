#ifndef _GUI_PR_DBDOT_H_
#define _GUI_PR_DBDOT_H_

#include <QObject>
#include <QPainter>
#include <QGraphicsItem>

namespace prim{


class DBDot: public QGraphicsItem
{
public:

  // constructor
  DBDot(qreal x, qreal y);

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


};


} // end prim namespace


#endif
