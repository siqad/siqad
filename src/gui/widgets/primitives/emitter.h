/** @file:     emitter.h
 *  @author:   Jake
 *  @created:  2016.11.24
 *  @editted:  2017.05.09  - Jake
 *  @license:  GNU LGPL v3
 *
 *  @brief:     Assistance class Emitter which allows non QWidget object to trigger
 *             an interrupt to the DesignWidet when clicked.
 */

#ifndef _PRIM_EMITTER_H_
#define _PRIM_EMITTER_H_


#include <QObject>
#include <QGraphicsItem>
#include "lattice.h"

namespace prim{

  // need a forward declaration of prim::Item
  class Item;

  // Emitter class to allow prim::Items to trigger an interrupt, singleton
  class Emitter : public QObject
  {
    Q_OBJECT

  public:

    //! get or create static instance of Emitter object
    static Emitter *instance();

    //! delete the static instance
    static void clear();

    //! emit a signal indicating the given item has been clicked
    void selectClicked(Item *);

    //! emit a signal telling DesignPanel to add item to scene
    void addItemToScene(Item *);

    //! emit a signal telling DesignPanel to remove item from scene
    void removeItemFromScene(Item *);

    //! emit a signal teeling DesignPanel to move item to lattice coordinate
    void moveItemToLatticeCoord(Item *, prim::LatticeCoord coord);

  signals:

    void sig_selectClicked(Item *);

    void sig_showProperty(Item *);

    void sig_addItemToScene(Item *);

    void sig_removeItemFromScene(Item *);

    void sig_moveItemToLatticeCoord(Item *, prim::LatticeCoord coord);

    void sig_resizeBegin();

    void sig_resizeFinalize(Item *, QRectF orig_rect, QRectF final_rect);

  private:

    // private constructor, singleton
    Emitter() : QObject(){}
    static Emitter *inst;   // static pointer to private instance
  };

} // end prim namespace

#endif
