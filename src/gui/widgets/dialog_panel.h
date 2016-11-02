#ifndef _GUI_DIALOG_PANEL_H_
#define _GUI_DIALOG_PANEL_H_

#include <QWidget>
#include <QScrollArea>

namespace gui{

class DialogPanel : public QScrollArea
{
public:

  // constructor
  DialogPanel(QWidget *parent = 0);

  // deconstructor
  ~DialogPanel();

private:

};


} // end gui namespace

#endif
