// @file:     design_assistant.h
// @author:   Samuel
// @created:  2019.03.13
// @license:  GNU LGPL v3
//
// @desc:     A class which assists the creation of desired DB logic circuits.

#ifndef _GUI_DESIGN_ASSISTANT_H_
#define _GUI_DESIGN_ASSISTANT_H_

#include <QtWidgets>
#include "gui/widgets/primitives/items.h"
#include "gui/widgets/components/sim_engine.h"

namespace gui{

  class DesignAssistant : public QWidget
  {
    Q_OBJECT

  public:

    //! Constructor.
    DesignAssistant(QWidget *parent=nullptr);

    //! Destructor.
    ~DesignAssistant() {};

    // TODO automatic job invocation

    // TODO area of interest (only show design aids for items within that area


  private:


  };

  class DBDesignAssistant : public DesignAssistant
  {
    Q_OBJECT

  public:

    //! Constructor.
    DBDesignAssistant(QWidget *parent=nullptr);

    //! Destructor.
    ~DBDesignAssistant() {};

    // TODO select which plugin to use as the supplier of certain aids

    // TODO this also means that plugins need to be able to declare what they're 
    // able to receive and supply

    // TODO DB structure (pair, fan-out, etc.) recognition. A selection/filter 
    // of what type of recognition to do should be present in the GUI

    //! Recognize specified features in the selected structure using the 
    //! chosen DB feature recognition plugins. Returns the QProcess to the 
    //! plugin runtime process. Assumes that a DB feature recognition plugin has 
    //! already been chosen in the GUI.
    QProcess *recognizeDBFeatures(QList<prim::DBDot> *dbs, QStringList features);

    // TODO Desired logic setup

    // TODO logic correctness evaluation

    // TODO E diff between simulated ground state and intended correct logic state

  private:

    //! A list of plugins which provide DB feature recognition.
    QList<prim::SimEngine*> plugins_db_feature_rec;

    //! A list of plugins which provide logic correctness evaluation.
    QList<prim::SimEngine*> plugins_logic_eval;

    //! A list of plugins which provide DB electron ground state simulation.
    QList <prim::SimEngine*> plugins_electron_ground;

  };

} // end of gui namespace

#endif
