// @file:     info_panel.h
// @author:   Jake
// @created:  2016.11.02
// @editted:  2017.05.11  - Jake
// @license:  GNU LGPL v3
//
// @desc:     unimplemented: Container for discribing information about
//            current selection/simulations/settings/etc.


#ifndef _GUI_INFO_PANEL_H_
#define _GUI_INFO_PANEL_H_

#include <QtWidgets>

namespace gui{

class InfoPanel : public QWidget
{
  Q_OBJECT

public:

  // constructor
  InfoPanel(QWidget *parent = 0);

  // destructor
  ~InfoPanel();

private:
};


} // end gui namespace


#endif
