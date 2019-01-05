// @file:     screenshot_manager.h
// @author:   Samuel
// @created:  2019.01.04
// @editted:  2019.01.04  - Samuel
// @license:  GNU LGPL v3
//
// @desc:     A manager for screenshot related options.


#ifndef _GUI_SCREENSHOT_MANAGER_H_
#define _GUI_SCREENSHOT_MANAGER_H_

#include <QtWidgets>
#include <QGraphicsRectItem>

#include "primitives/items.h"


namespace gui{

  class ScreenshotManager : public QWidget
  {
    Q_OBJECT

    public:

      //! Constructor, the parent should normally be design panel.
      ScreenshotManager(QWidget *parent = 0);

      //! Destructor.
      ~ScreenshotManager();

      //! Return the screenshot clip area as a QRect.
      QRectF clipArea() {return clip_area->rect();}

      //! Set the screenshot clip area.
      void setClipArea(QRectF area) {clip_area->setRect(area);}

      //! Reset the screenshot clip area to a null rectangle (use isNull() to
      //! check for QRect null state.)
      void resetClipArea() {clip_area->setRect(QRectF());}


    public slots:



    signals:

      //! Take a screenshot of the same region as before.
      void sig_repeatLastRegion();  

      //! Tell display panel to allow user to set screenshot clip area. The next
      //! rubberband selection should become the clip area.
      void sig_selectClipArea();
      
      // TODO include display options such as disabling publish mode, inclusion
      // of scale bar, etc.

    private:

      //! Initialize the screenshot manager.
      void initScreenshotManager();



      // Variables
      prim::ScreenshotClipArea *clip_area=nullptr;  //! Clip area for region screenshot.

      QLineEdit *le_name_format=nullptr;            //! Text field for file name format.
      QLineEdit *le_save_dir=nullptr;               //! Path to target directory.
      QCheckBox *cb_sim_result_style=nullptr;       //! Toggle simulation result style.
      QCheckBox *cb_publish_style=nullptr;          //! Toggle publish style.
      // TODO pointer to scale bar item
  };


} // end gui namespace


#endif
