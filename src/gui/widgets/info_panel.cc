// @file:     info_panel.cc
// @author:   Jake
// @created:  2016.11.02
// @editted:  2017.05.11  - Jake
// @license:  GNU LGPL v3
//
// @desc:     InfoPanel definitions

#include "info_panel.h"

namespace gui{

InfoPanel::InfoPanel(QWidget *parent)
  : QWidget(parent)
{
  initInfoPanel();
}

InfoPanel::~InfoPanel()
{}


// public slots

void InfoPanel::updateCursorPhysLoc(const QPointF cursor_pos)
{
  disp_cursor_coords->setText(tr("(%1, %2)")
      .arg(cursor_pos.x(),0,'f',2)
      .arg(cursor_pos.y(),0,'f',2));
}

void InfoPanel::updateZoom(float zoom)
{
  disp_zoom->setText(QString::number(zoom,'f',3));
}

void InfoPanel::updateSelItemCount(QList<prim::Item*> items)
{
  int db_count = 0;
  QRectF bounding_rect;
  for (prim::Item *item : items) {
    // accumulate DB count
    if (item->item_type == prim::Item::DBDot)
      db_count++;
    // get total bounding rect
    bounding_rect |= item->boundingRect().translated(item->pos());
  }

  // update label content
  disp_sel_db_count->setText(QString::number(db_count));
  disp_sel_bounding_rect->setText(tr("%1 nm x %2 nm")
      .arg(bounding_rect.width() / prim::Item::scale_factor_nm,0,'f',2)
      .arg(bounding_rect.height() / prim::Item::scale_factor_nm,0,'f',2));
}



// private

void InfoPanel::initInfoPanel()
{
  QLabel *l_cursor_coords = new QLabel(tr("Cursor (nm)"));
  disp_cursor_coords = new QLabel(tr("(0,0)"));

  QLabel *l_zoom = new QLabel(tr("Zoom"));
  disp_zoom = new QLabel(tr("0"));

  QLabel *l_sel_db_count = new QLabel(tr("Selected DBs"));
  disp_sel_db_count = new QLabel(tr("0"));

  QLabel *l_sel_bounding_rect = new QLabel(tr("Selected rect"));
  l_sel_bounding_rect->setToolTip(tr("Size of bounding rectangle containing all selected graphical items (WxH). The same selection might not result in the same dimensions at different zoom levels or viewing modes since graphical items may be at different sizes."));
  disp_sel_bounding_rect = new QLabel(tr("0 nm x 0 nm"));

  QHBoxLayout *hl_cursor_coords = new QHBoxLayout;
  hl_cursor_coords->addWidget(l_cursor_coords);
  hl_cursor_coords->addWidget(disp_cursor_coords);

  QHBoxLayout *hl_zoom = new QHBoxLayout;
  hl_zoom->addWidget(l_zoom);
  hl_zoom->addWidget(disp_zoom);

  QHBoxLayout *hl_sel_db_count = new QHBoxLayout;
  hl_sel_db_count->addWidget(l_sel_db_count);
  hl_sel_db_count->addWidget(disp_sel_db_count);

  QHBoxLayout *hl_sel_bounding_rect = new QHBoxLayout;
  hl_sel_bounding_rect->addWidget(l_sel_bounding_rect);
  hl_sel_bounding_rect->addWidget(disp_sel_bounding_rect);

  QVBoxLayout *vl_infos = new QVBoxLayout;
  vl_infos->addLayout(hl_cursor_coords);
  vl_infos->addLayout(hl_zoom);
  vl_infos->addLayout(hl_sel_db_count);
  vl_infos->addLayout(hl_sel_bounding_rect);
  vl_infos->addStretch();

  setLayout(vl_infos);
}

}
