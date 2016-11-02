#ifndef _GUI_DIALOG_PANEL_H_
#define _GUI_DIALOG_PANEL_H_

#include <QObject>
#include <QWidget>
#include <QScrollArea>
#include <QPlainTextEdit>
#include <QString>
#include <QMouseEvent>

namespace gui{

class DialogPanel : public QPlainTextEdit
{
  Q_OBJECT

public:

  // constructor
  explicit DialogPanel(QWidget *parent = 0);

  // deconstructor
  ~DialogPanel();

  // public methods
  void echo(const QString& s);

private:

  // interrupts
  void mousePressEvent(QMouseEvent *e);

};


} // end gui namespace

#endif
