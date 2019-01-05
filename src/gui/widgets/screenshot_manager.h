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
      QRectF clipArea() {return clip_area->sceneRect();}

      //! Set the screenshot clip area.
      void setClipArea(QRectF area) {clip_area->setSceneRect(area);}

      //! Reset the screenshot clip area to a null rectangle (use isNull() to
      //! check for QRect null state.)
      void resetClipArea() {clip_area->setSceneRect(QRectF());}

      //! Set the visibility of the clip area preview.
      void setClipVisibility(bool visible) {clip_area->setVisible(visible);}



    public slots:



    signals:

      //! Take a screenshot of the area defined in the provided QRectF. If the
      //! rect is null, take a screenshot of the entire scene.
      void sig_takeScreenshot(const QString &target_img_path, 
                              const QRectF &scene_rect=QRectF(),
                              bool always_overwrite=false);

      //! Tell display panel to allow user to set screenshot clip area. The next
      //! rubberband selection should become the clip area.
      void sig_clipSelectionTool();

      // TODO include display options such as disabling publish mode, inclusion
      // of scale bar, etc.

    private:

      //! Initialize the screenshot manager.
      void initScreenshotManager();



      // Variables
      prim::ScreenshotClipArea *clip_area=nullptr;  //! Clip area for region screenshot.

      QLineEdit *le_name=nullptr;            //! Text field for file name format.
      QLineEdit *le_save_dir=nullptr;               //! Path to target directory.
      QCheckBox *cb_sim_result_style=nullptr;       //! Toggle simulation result style.
      QCheckBox *cb_publish_style=nullptr;          //! Toggle publish style.
      QCheckBox *cb_overwrite=nullptr;              //! Overwrite file without asking.
      // TODO pointer to scale bar item
  };


} // end gui namespace


#endif
