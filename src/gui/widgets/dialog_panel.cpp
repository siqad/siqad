#include "dialog_panel.h"
#include "src/settings/settings.h"


gui::DialogPanel::DialogPanel(QWidget *parent)
  : QScrollArea(parent)
{
  settings::GUISettings gui_settings;

  setMaximumHeight(gui_settings.value("Panel/maxh").toInt());
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

gui::DialogPanel::~DialogPanel()
{}
