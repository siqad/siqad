/** @file:     dialog_panel.h
 *  @author:   Jake
 *  @created:  2016.11.02
 *  @editted:  2017.05.11  - Jake
 *  @license:  GNU LGPL v3
 *
 *  @brief:     PlainTextEdit field in which to display stdout when instructed.
 */

#ifndef _GUI_DIALOG_PANEL_H_
#define _GUI_DIALOG_PANEL_H_


#include <QtWidgets>


namespace gui{

  class DialogPanel : public QPlainTextEdit
  {
    Q_OBJECT

  public:

    //! constructor
    explicit DialogPanel(QWidget *parent=0);

    //! destructor
    ~DialogPanel();

    // public methods
    //! Write QString s into the QPlainTextEdit widget
    void echo(const QString& s);

  private:

    //! Purge oldest log files if log file count > log/keepcount.
    void purgeOldLogs();

    QDir log_dir;         // log directory
    QFile *file=nullptr;  // target file for logging
    QString filename;     // log file name (excluding dir path)
  };

} // end gui namespace

#endif
