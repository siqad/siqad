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
    //! Returns unenclosed numbers found in input's QString.
    QStringList cleanNumbers(QString* input);
    //! Returns non-numbers found in input's QString.
    QStringList cleanAlphas(QString* input);
    //! Returns ()-enclosed numbers found in input's QString.
    QStringList cleanBrackets(QString* input);

  private:
    bool performCommand();
    bool commandAddItem();
    bool commandRemoveItem();
    bool commandEcho();
    bool commandHelp();
    bool commandRun();
    bool commandMoveItem();

    DesignPanel *design_pan;
    DialogPanel *dialog_pan;
    QStringList alphas;        // alphabetical inputs
    QStringList brackets;      // number inputs enclosed in brackets
    QStringList numericals;    // number inputs without bracket enclosure
    QStringList input_kws;     // white-listed keywords
    QString input_orig;        // original input before processing
  };

} // end gui namespace

#endif
