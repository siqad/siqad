// @file:     emitter.h
// @author:   Jake
// @created:  2016.11.24
// @editted:  2017.05.01  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Assistance class Emitter which allows non QWidget object to trigger
//            an interrupt to the DesignWidet when clicked.

#ifndef _PRIM_EMITTER_H_
#define _PRIM_EMITTER_H_

#include <QObject>
#include <QGraphicsItem>



namespace prim{

// singleton class
class Emitter : public QObject
{
  Q_OBJECT

public:
  // get or create static instance of Emitter object
  static Emitter *instance();

  ~Emitter() {}

  void selectClicked(QGraphicsItem *item)
  {
    emit sig_selectClicked(item);
  }

signals:

  void sig_selectClicked(QGraphicsItem *item);

private:

  // constructor private: singleton
  Emitter() : QObject(){}

  static Emitter *inst;   // static pointer to instance

};


} // end prim namespace

#endif
