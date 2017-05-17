// @file:     emitter.h
// @author:   Jake
// @created:  2016.11.24
// @editted:  2017.05.09  - Jake
// @license:  GNU LGPL v3
//
// @desc:     Assistance class Emitter which allows non QWidget object to trigger
//            an interrupt to the DesignWidet when clicked.


#ifndef _PRIM_EMITTER_H_
#define _PRIM_EMITTER_H_


#include <QObject>
#include <QGraphicsItem>

namespace prim{

  // need a forward declaration of prim::Item
  class Item;

  // Emitter class to allow prim::Items to trigger an interrupt, singleton
  class Emitter : public QObject
  {
    Q_OBJECT

  public:

    // get or create static instance of Emitter object
    static Emitter *instance();

    // delete the static instance
    static void clear();

    // emit a signal indicating the given item has been clicked
    void selectClicked(Item *);

  signals:

    void sig_selectClicked(Item *);

  private:

    // private constructor, singleton
    Emitter() : QObject(){}
    static Emitter *inst;   // static pointer to private instance
  };

} // end prim namespace

#endif
