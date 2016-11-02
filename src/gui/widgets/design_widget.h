#ifndef _GUI_DESIGN_WIDGET_H_
#define _GUI_DESIGN_WIDGET_H_

#include <QWidget>


namespace gui{

class DesignWidget : public QWidget
{
public:

  // constructor
  DesignWidget(QWidget *parent = 0);

  // deconstructor
  ~DesignWidget();

private:
};

} // end gui namespace


#endif
