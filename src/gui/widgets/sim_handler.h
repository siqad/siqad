// @file:     sim_handler.h
// @author:   Samuel
// @created:  2017.09.27
// @editted:  2017.09.27 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     window that allows users to setup and dispatch new simulations,
//            as well as manage ongoing or completed simulations.

#ifndef _GUI_SIM_HANDLER_H_
#define _GUI_SIM_HANDLER_H_

#include <QtWidgets>

namespace gui{

class SimHandler : public QWidget
{
  Q_OBJECT

public:

  // constructor
  SimHandler(QWidget *parent = 0);

  // destructor
  ~SimHandler();

private:
  void initSimHandler();

  void simUserOptions();            // take user options for simulation parameters
  bool exportSimProblem();          // generate problem XML
  void invokeSimulator();           // call simulator to run problem XML
  bool checkSimCompletion();        // check whether simulation has been completed
};


} // end gui namespace

#endif
