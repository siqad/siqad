#include "dialog_panel.h"
#include "src/settings/settings.h"

#include <iostream>
#include <QDebug>


gui::DialogPanel::DialogPanel(QWidget *parent)
  : QPlainTextEdit(parent)
{
  settings::GUISettings gui_settings;

  setMaximumHeight(gui_settings.value("Panel/maxh").toInt());
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  setReadOnly(true);
  setLineWrapMode(QPlainTextEdit::NoWrap);
  //setMinimumWidth(gui_settings.value("Panel/logw").toInt());
}

gui::DialogPanel::~DialogPanel()
{}


// Add given text as a new line in the dialog panel
void gui::DialogPanel::echo(const QString& s)
{
  appendPlainText(s);
  //verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}


void gui::DialogPanel::mousePressEvent(QMouseEvent *e)
{
  qWarning("mouse clicked");
}
