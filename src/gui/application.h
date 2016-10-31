#ifndef _UI_APPLICATION_H_
#define _UI_APPLICATION_H_

#include <QMainWindow>

namespace Ui
{
    class ApplicationGUI;
}

// Main application window
class ApplicationGUI : public QMainWindow
{
    Q_OBJECT

public:

    // constructor
    explicit ApplicationGUI(QWidget *parant = 0);

    // deconstructor
    ~ApplicationGUI();

public slots:

protected:

private:


};
#endif
