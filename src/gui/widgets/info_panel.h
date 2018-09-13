// @file:     info_panel.h
// @author:   Jake
// @created:  2016.11.02
// @editted:  2017.05.11  - Jake
// @license:  GNU LGPL v3
//
// @desc:     unimplemented: Container for discribing information about
//            current selection/simulations/settings/etc.


#ifndef _GUI_INFO_PANEL_H_
#define _GUI_INFO_PANEL_H_

#include <QtWidgets>

#include "primitives/items.h"


namespace gui{

  class InfoPanel : public QWidget
  {
    Q_OBJECT

    public:

      // constructor
      InfoPanel(QWidget *parent = 0);

      // destructor
      ~InfoPanel();

    public slots:

      //! Update scene coordinates of cursor on screen
      void updateCursorPhysLoc(const QPointF curser_pos);

      //! Update zoom level
      void updateZoom(float zoom);

      /*
      //! Update the total DB count
      void updateTotalDBCount(int db_count);
      */

      //! Update count of selected items
      void updateSelItemCount(QList<prim::Item*> items);

      /*
      //! Update bounding rect dimensions of selected items
      void updateSelBoundingRect(const QRectF b_rect);
      */

    private:

      //! Initialize the info panel with blank fields
      void initInfoPanel();

      // TODO eventually would like to add options for users to pick what stats
      // to show and what not to.

      // Variables to be shown in fields
      QLabel *disp_cursor_coords;       // Cursor physical coordinates in nm
      QLabel *disp_zoom;                // Zoom level
      QLabel *disp_sel_db_count;        // Number of selected DBs
      QLabel *disp_sel_bounding_rect;   // Total bounding rect of selection
  };


} // end gui namespace


#endif
