// @file:     afm_manager.h
// @author:   Samuel
// @created:  2018.01.18
// @editted:  2018.01.18 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Widget for editing AFM Path properties and adding/removing nodes

#ifndef _GUI_AFM_PANEL_H_
#define _GUI_AFM_PANEL_H_

// TODO includes

namespace gui{

  class AFMPanel : public QWidget
  {
    Q_OBJECT

  public:

    // constructor
    AFMManager(QWidget *parent = 0);

    // destructor
    ~AFMManager() {};


    // path being displayed in the panel
    void setAFMPathShowing(AFMPath *path_sh) {path_showing = path_sh;}
    AFMPath *getAFMPathShowing() {return path_showing;}

    // path being edited
    void setAFMPathEditing(AFMPath *path_ed) {path_editing = path_ed;}
    AFMPath *getAFMPathEditing() {return path_editing;}

  public slots:
    void 

  private:

    // VAR
    AFMPath *path_showing;
    AFMPath *path_editing;


  };

} // end of gui namespace

#endif
