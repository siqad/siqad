#ifndef _PRIM_EMITTER_H_
#define _PRIM_EMITTER_H_

#include <QObject>
#include <QGraphicsItem>

namespace prim{

class Emitter : public QObject
{
  Q_OBJECT

public:

  //constructor
  Emitter() : QObject(){}

  //destructor
  ~Emitter() {}

  void selectClicked(QGraphicsItem *item)
  {
    emit sig_selectClicked(item);
  }

signals:

  void sig_selectClicked(QGraphicsItem *item);

private:

};


} // end prim namespace

#endif
