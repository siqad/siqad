// @file:     completer.h
// @author:   Nathan
// @created:  2018.08.07
// @editted:  2018.08.07  - Nathan
// @license:  GNU LGPL v3
//
// @desc:     QCompleter with overrriden eventFilter.

#ifndef _GUI_COMPLETER_H
#define _GUI_COMPLETER_H

#include <QWidget>
#include <QString>
#include <QDebug>
#include <QKeyEvent>
#include <QtCore>
#include <QCompleter>

namespace gui{

// customized Completer object with an event filter.
class Completer : public QCompleter
{
  Q_OBJECT
public:
  // constructor
  Completer(QWidget *parent=0);
  Completer(QStringList list, QWidget *parent=0);

  // destructor
  ~Completer(){};
protected:
  bool eventFilter(QObject *obj, QEvent *event);

private:
  void initCompleter();
};

} // end gui namespace


#endif
