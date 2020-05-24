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

#include "../../../global.h"
#include "../primitives/items.h"


namespace gui{

  class ScreenshotManager : public QWidget
  {
    Q_OBJECT

    public:

      //! Constructor, the parent should normally be design panel. The layer
      //! id of the misc layer must be provided as that's the only layer which
      //! ScreenshotManager will add items to.
      ScreenshotManager(int misc_layer_id, QWidget *parent = 0);

      //! Destructor.
      ~ScreenshotManager();

      //! Actions performed when entering or exiting screenshot mode.
      void prepareScreenshotMode(bool entering);

      //! Return the screenshot clip area as a QRect.
      QRectF clipArea() const {return clip_area->sceneRect();}

      //! Set the screenshot clip area. If a null rectangle is received, then
      //! clipping is disabled.
      void setClipArea(QRectF area=QRectF());

      //! Return the clip visibility.
      bool clipVisible() {return clip_area->isVisible();}

      //! Set the visibility of the clip area preview.
      void setClipVisibility(const bool &visible, const bool &cb_update=false);

      //! Set the scale bar length, or hide it if the length is less than 0.
      void setScaleBar(float t_length, Unit::DistanceUnit unit);

      //! Set the scale bar anchor position in screen coordinates.
      void setScaleBarAnchor(QPointF anchor) {scale_bar->setScenePos(anchor);}

      //! Set the visibility of the scale bar.
      void setScaleBarVisibility(const bool &visible, const bool &cb_update=false);



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

      //! Tell display panel to switch to scale bar position anchor tool.
      void sig_scaleBarAnchorTool();

      //! Add visual aid (scale bar, clip area preview) to display panel.
      void sig_addVisualAidToDP(prim::Item *t_item);

      //! Remove visual aid (scale bar, clip area preview) from display panel.
      void sig_removeVisualAidFromDP(prim::Item *t_item);

      //! Inform design panel of closing.
      void sig_closeEventTriggered();

      // TODO include display options such as disabling publish mode, inclusion
      // of scale bar, etc.

    private:

      //! Initialize the screenshot manager.
      void initScreenshotManager();

      //! Override close event to also end screenshot mode.
      void closeEvent(QCloseEvent *bar);

      // Variables
      int misc_layer_id=-1;                         //! layer id of the misc layer

      prim::ScreenshotClipArea *clip_area=nullptr;  //! Clip area for region screenshot.
      prim::ScaleBar *scale_bar=nullptr;            //! Scale bar in screenshots.

      QLineEdit *le_name=nullptr;            //! Text field for file name format.
      QLineEdit *le_save_dir=nullptr;               //! Path to target directory.
      QCheckBox *cb_sim_result_style=nullptr;       //! Toggle simulation result style.
      QCheckBox *cb_publish_style=nullptr;          //! Toggle publish style.
      QCheckBox *cb_overwrite=nullptr;              //! Overwrite file without asking.
      QCheckBox *cb_scale_bar=nullptr;              //! Checkbox for showing scale bar
      QCheckBox *cb_preview_clip=nullptr;           //! Checkbox for showing preview clip
      // TODO pointer to scale bar item
  };


} // end gui namespace


#endif
