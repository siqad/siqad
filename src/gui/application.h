#ifndef _UI_APPLICATION_H_
#define _UI_APPLICATION_H_

// Qt inclusions
#include <QMainWindow>
#include <QObject>

// Widget inclusions
#include "widgets/design_widget.h"
#include "widgets/dialog_panel.h"
#include "widgets/info_panel.h"

namespace Ui
{
  class ApplicationGUI;
} // end namespace Ui

namespace gui{

// Main application window
class ApplicationGUI : public QMainWindow
{
  Q_OBJECT

public:

  // constructor
  explicit ApplicationGUI(QWidget *parent = 0);

  // deconstructor
  ~ApplicationGUI();

  // static declaration of DialogPanel for dialogstream
  static gui::DialogPanel *dialog_wg;

public slots:

protected:

private:

  void initGui();
  void initMenuBar();
  void initTopBar();
  void initSideBar();

  void loadSettings();
  void saveSettings();

  QString global_settings_fname_;
  QToolBar *top_bar;
  QToolBar *side_bar;

  // functional widgets
  gui::DesignWidget *design_wg;
  gui::InfoPanel *info_wg;
};

} // end namespace gui



#endif
