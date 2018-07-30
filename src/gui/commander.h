/** @file:     dialog_panel.h
 *  @author:   Nathan
 *  @created:  2018.06.26
 *  @editted:  2018.06.26  - Nathan
 *  @license:  GNU LGPL v3
 *
 *  @brief:     Handles commands from dialog_panel, and asks design_panel to perform the appropriate tasks.
 */

#ifndef _GUI_COMMANDER_H_
#define _GUI_COMMANDER_H_

#include <QtWidgets>
#include <QDir>
#include "widgets/design_panel.h"
#include "widgets/dialog_panel.h"

namespace gui{

  class Commander
  {
  public:

    //! constructor
    Commander();

    //! destructor
    ~Commander(){};

    // public methods
    //! Set design panel
    void setDesignPanel(DesignPanel *des_pan){design_pan = des_pan;}
    //! Set dialog panel
    void setDialogPanel(DialogPanel *dia_pan){dialog_pan = dia_pan;}
    //! Adds a keyword into the whitelist
    void addKeyword(QString keyword);
    //! Clears all keywords from the whitelist.
    void clearKeywords();
    //! Further parses input after application determines that input is non-empty.
    void parseInputs(QString input);

    QStringList cleanNumbers(QString* input);
    QStringList cleanAlphas(QString* input);
    QStringList cleanBrackets(QString* input);

  private:

    bool performCommand();
    // bool performCommand(QStringList cmds);
    void commandAddItem(QStringList args);
    void commandRemoveItem(QStringList args);
    void commandEcho(QStringList args);
    void commandHelp(QStringList args);
    void commandRun(QStringList args);
    void commandMoveItem(QStringList args);

    void commandAddItem();
    void commandRemoveItem();
    void commandEcho();
    void commandHelp();
    void commandRun();
    void commandMoveItem();

    DesignPanel *design_pan;
    DialogPanel *dialog_pan;
    QStringList brackets;      // number inputs enclosed in brackets
    QStringList numericals;    // number inputs without bracket enclosure
    QStringList alphas;        // alphabetical inputs
    QStringList input_kws;     // white-listed keywords
  };

} // end gui namespace

#endif
