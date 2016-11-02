#ifndef _GUI_INFO_PANEL_H_
#define _GUI_INFO_PANEL_H_

#include <QObject>
#include <QWidget>

namespace gui{

class InfoPanel : public QWidget
{
  Q_OBJECT
  
public:

  // constructor
  InfoPanel(QWidget *parent = 0);

  // deconstructor
  ~InfoPanel();

private:
};


} // end gui namespace


#endif
