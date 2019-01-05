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
  : QWidget(parent, Qt::Dialog)
{
  initScreenshotManager();
}

ScreenshotManager::~ScreenshotManager()
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

  QVBoxLayout *vl_visual = new QVBoxLayout();
  vl_visual->addWidget(cb_sim_result_style);
  vl_visual->addWidget(cb_publish_style);
  group_visual->setLayout(vl_visual);

  group_visual->setDisabled(true);

  // TODO connect
  

  // Clip Setting Group
  QGroupBox *group_clip = new QGroupBox(tr("Clipping"));
  QPushButton *button_set_clip = new QPushButton(tr("Set Clip Area"));
  QPushButton *button_reset_clip = new QPushButton(tr("Reset"));
  QCheckBox *cb_preview_clip = new QCheckBox(tr("Preview Clip Area"));

  connect(button_set_clip, &QAbstractButton::clicked,
          [this]() {emit sig_clipSelectionTool();});
  connect(button_reset_clip, &QAbstractButton::clicked,
          this, &gui::ScreenshotManager::resetClipArea);
  connect(cb_preview_clip, &QCheckBox::stateChanged,
          this, &gui::ScreenshotManager::setClipVisibility);

  QFormLayout *fl_clip = new QFormLayout();
  fl_clip->addRow(button_set_clip, button_reset_clip);
  fl_clip->addRow(cb_preview_clip);
  group_clip->setLayout(fl_clip);
        

  // TODO Toggle publish style button (while at it, probably want to revamp the whole simulation mode and publish style mode thing to status flags since multiple things can be enabled concurrently. Additional flags can be added to enable preset colorschemes)

  // Action Group
  QGroupBox *group_screenshot = new QGroupBox(tr("Screenshot"));
  QLabel *label_save_dir = new QLabel(tr("Directory"));
  le_save_dir = new QLineEdit(QDir::currentPath());  // TODO initialize to current working path
  QPushButton *button_browse = new QPushButton(tr("..."));
  QLabel *label_name = new QLabel(tr("Name"));
  le_name = new QLineEdit(tr("siqad-screenshot.svg"));
  // TODO tooltip for name formatting
  cb_overwrite = new QCheckBox(tr("Overwrite without asking"));
  QCheckBox *cb_always_ask_name = new QCheckBox(tr("Browse for file path every time"));
  QPushButton *button_take_screenshot = new QPushButton(tr("Take Screenshot"));

  // browse for path and fill into le_save_dir
  connect(button_browse, &QAbstractButton::clicked,
          [this]() {
            QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
            le_save_dir->setText(dir);
          }
  );
  // disable relevant fields when user opts to ask for path and name every time
  connect(cb_always_ask_name, &QCheckBox::stateChanged, 
          [this, button_browse](int cb_state) {
            bool disable = cb_state == Qt::Checked;
            le_save_dir->setDisabled(disable);
            button_browse->setDisabled(disable);
            le_name->setDisabled(disable);
            cb_overwrite->setDisabled(disable);
          }
  );
  // emit screenshot signal with relevant settings
  connect(button_take_screenshot, &QAbstractButton::clicked,
          [this]() {
            QString fpath = QDir(le_save_dir->text()).absoluteFilePath(le_name->text());
            emit sig_takeScreenshot(fpath, clip_area->sceneRect(), cb_overwrite->isChecked());
          }
  );
      

  QHBoxLayout *hl_save_dir = new QHBoxLayout();
  hl_save_dir->addWidget(label_save_dir);
  hl_save_dir->addWidget(le_save_dir);
  hl_save_dir->addWidget(button_browse);

  QFormLayout *fl_screenshot = new QFormLayout();
  fl_screenshot->addRow(hl_save_dir);
  fl_screenshot->addRow(label_name, le_name);
  fl_screenshot->addRow(cb_overwrite);
  fl_screenshot->addRow(cb_always_ask_name);
  fl_screenshot->addRow(button_take_screenshot);
  group_screenshot->setLayout(fl_screenshot);


  // Add them to this widget
  QVBoxLayout *vl_widget = new QVBoxLayout();
  vl_widget->addWidget(group_visual);
  vl_widget->addWidget(group_clip);
  vl_widget->addWidget(group_screenshot);
  setLayout(vl_widget);


  // TODO somewhere - check if the chosen save directory is writable before attempting screenshot
}




} // end gui namespace
