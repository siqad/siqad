// @file:     screenshot_manager.h
// @author:   Samuel
// @created:  2019.01.04
// @editted:  2019.01.04 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Implementation of screenshot manager.

#include "screenshot_manager.h"


namespace gui{

ScreenshotManager::ScreenshotManager(QWidget *parent)
  : QWidget(parent, Qt::Widget)
{
  initScreenshotManager();
}


// SIGNALS

void ScreenshotManager::sig_repeatLastRegion()
{

}

void ScreenshotManager::sig_selectClipArea()
{

}



// PRIVATE

void ScreenshotManager::initScreenshotManager()
{
  clip_area = new prim::ScreenshotClipArea();

  // init GUI

  // Visual Setting Group
  QGroupBox *group_visual = new QGroupBox(tr("Visual"));
  cb_sim_result_style = new QCheckBox(tr("Simulation Result Style"));
  cb_publish_style = new QCheckBox(tr("Publish Style"));

  QFormLayout *fl_visual = new QFormLayout();
  fl_visual->addRow(cb_sim_result_style);
  fl_visual->addRow(cb_publish_style);
  group_visual->setLayout(fl_visual);
  
  // Clip Setting Group
  QGroupBox *group_clip = new QGroupBox(tr("Clipping"));
  QPushButton *button_set_clip = new QPushButton(tr("Set Clip Area"));
  QPushButton *button_reset_clip = new QPushButton(tr("Reset"));
  QCheckBox *cb_preview_clip = new QCheckBox(tr("Preview Clip Area"));

  QFormLayout *fl_clip = new QFormLayout();
  fl_clip->addRow(button_set_clip, button_reset_clip);
  fl_clip->addRow(cb_preview_clip);
  group_clip->setLayout(fl_clip);

  // TODO Toggle publish style button (while at it, probably want to revamp the whole simulation mode and publish style mode thing to status flags since multiple things can be enabled concurrently. Additional flags can be added to enable preset colorschemes)

  // Action Group
  QGroupBox *group_screenshot = new QGroupBox(tr("Screenshot"));
  QLabel *label_name_format = new QLabel(tr("Filename Format"));
  le_name_format = new QLineEdit();
  // TODO tooltip for name formatting
  QCheckBox *cb_always_ask_name = new QCheckBox(tr("Ask for name every time"));
  QCheckBox *cb_overwrite = new QCheckBox(tr("Overwrite if name exists"));
  le_save_dir = new QLineEdit();  // TODO initialize to current working path
  // TODO browse button
  QPushButton *button_take_screenshot = new QPushButton(tr("Take Screenshot"));

  // TODO somewhere - check if the chosen save directory is writable before attempting screenshot
}




} // end gui namespace
