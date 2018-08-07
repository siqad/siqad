// @file:     completer.cc
// @author:   Nathan
// @created:  2018.08.07
// @editted:  2018.08.07  - Nathan
// @license:  GNU LGPL v3
//
// @desc:     Completer definitions

#include "completer.h"

gui::Completer::Completer(QWidget *parent)
  : QCompleter(parent)
{
  initCompleter();
}

gui::Completer::Completer(QStringList list, QWidget *parent)
  : QCompleter(list, parent)
{
  initCompleter();
}

void gui::Completer::initCompleter()
{
  installEventFilter(this);
  setCaseSensitivity(Qt::CaseSensitive);
  setCompletionMode(QCompleter::InlineCompletion);
}
//Only job is to eat the tab press so the the completer doesn't lose focus.
bool gui::Completer::eventFilter(QObject *obj, QEvent *event)
{
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if (keyEvent->key() == Qt::Key_Tab) {
      //throw the event to the widgets eventfilter to deal with.
      widget()->eventFilter(obj, event);
      return true;
    }
    return QCompleter::eventFilter(obj, event);
  }
  return QCompleter::eventFilter(obj, event);
}
