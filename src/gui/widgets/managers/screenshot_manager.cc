// @file:     screenshot_manager.h
// @author:   Samuel
// @created:  2019.01.04
// @editted:  2019.01.04 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Implementation of screenshot manager.

#include "screenshot_manager.h"


namespace gui{

ScreenshotManager::ScreenshotManager(int misc_layer_id, QWidget *parent)
  : QWidget(parent, Qt::Dialog), misc_layer_id(misc_layer_id)
{
  initScreenshotManager();
}

ScreenshotManager::~ScreenshotManager()
{
  // TODO safely remove clip_area and scale_bar
  if (clip_area->scene() != 0) 
    emit sig_removeVisualAidFromDP(clip_area);
  delete clip_area;

  if (scale_bar->scene() != 0) 
    emit sig_removeVisualAidFromDP(scale_bar);
  delete scale_bar;
}

void ScreenshotManager::prepareScreenshotMode(bool entering)
{
  setVisible(entering);
  setClipVisibility(entering);
  setScaleBarVisibility(entering);
}

void ScreenshotManager::setClipArea(QRectF area)
{
  clip_area->setSceneRect(area);
  if (area.isValid() && clip_area->scene() == 0) {
    // add clip area to scene if it isn't already in one
    emit sig_addVisualAidToDP(clip_area);
  } else if(area.isNull() && clip_area->scene() !=0) {
    // remove clip area from scene if it's in one
    emit sig_removeVisualAidFromDP(clip_area);
  }
}

void ScreenshotManager::setClipVisibility(const bool &visible)
{
  if (visible && !clip_area->scene())
    emit sig_addVisualAidToDP(clip_area);
  clip_area->setVisible(visible);
}

void ScreenshotManager::setScaleBar(float t_length, Unit::DistanceUnit unit)
{
  scale_bar->setScaleBar(t_length, unit);

  if (t_length > 0 && scale_bar->scene() == 0) {
    // add scale bar to scene if it isn't already in one
    emit sig_addVisualAidToDP(scale_bar);
  } else if (t_length <= 0 && scale_bar->scene() != 0) {
    // remove scale bar from scene if it's in one
    emit sig_removeVisualAidFromDP(scale_bar);
  }
}

void ScreenshotManager::setScaleBarVisibility(const bool &visible)
{
  if (visible && !scale_bar->scene())
    emit sig_addVisualAidToDP(scale_bar);
  scale_bar->setVisible(visible);
}


// PRIVATE

void ScreenshotManager::initScreenshotManager()
{
  clip_area = new prim::ScreenshotClipArea(misc_layer_id);
  scale_bar = new prim::ScaleBar(misc_layer_id);

  // init GUI

  // Scale Bar Setting Group
  QGroupBox *group_scale_bar = new QGroupBox(tr("Scale Bar"));
  // TODO implement style change
  //cb_sim_result_style = new QCheckBox(tr("Simulation Result Style"));
  //cb_publish_style = new QCheckBox(tr("Publish Style"));
  QCheckBox *cb_scale_bar = new QCheckBox(tr("Show Scale Bar"));
  QLabel *label_scale_bar_length = new QLabel(tr("Length"));
  QLineEdit *le_scale_bar_length = new QLineEdit("1");
  QComboBox *cbb_scale_bar_unit = new QComboBox();
  QPushButton *button_set_scale_bar_anchor = new QPushButton(tr("Set Scale Bar Anchor"));
  
  // populate scale bar length unit dropdown menu
  cbb_scale_bar_unit->addItems(Unit::distanceUnitStringList(Unit::pm,Unit::m));
  cbb_scale_bar_unit->setCurrentText(Unit::distanceUnitString(Unit::nm));

  // update scale bar properties from GUI options
  auto updateScaleBarFromOptions = [this, le_scale_bar_length, cbb_scale_bar_unit](){
    float sb_length = le_scale_bar_length->text().toFloat();
    Unit::DistanceUnit unit = Unit::stringToDistanceUnit(cbb_scale_bar_unit->currentText());
    setScaleBar(sb_length, unit);
  };
  connect(le_scale_bar_length, &QLineEdit::textChanged, 
          updateScaleBarFromOptions);
  connect(cbb_scale_bar_unit, &QComboBox::currentTextChanged, 
          updateScaleBarFromOptions);
  updateScaleBarFromOptions();

  // enable or disable scale bar options depending on the check state of cb_scale_bar
  auto enableScaleBarOptions = [this, le_scale_bar_length, cbb_scale_bar_unit, 
                                button_set_scale_bar_anchor, 
                                updateScaleBarFromOptions](int cb_state) {
    bool show_scale_bar_settings = cb_state == Qt::Checked;
    le_scale_bar_length->setEnabled(show_scale_bar_settings);
    cbb_scale_bar_unit->setEnabled(show_scale_bar_settings);
    button_set_scale_bar_anchor->setEnabled(show_scale_bar_settings);
    scale_bar->setVisible(show_scale_bar_settings);
    updateScaleBarFromOptions();
  };
  connect(cb_scale_bar, &QCheckBox::stateChanged, enableScaleBarOptions);
  enableScaleBarOptions(cb_scale_bar->checkState());


  // set scale bar anchor
  connect(button_set_scale_bar_anchor, &QAbstractButton::clicked,
          [this]() {emit sig_scaleBarAnchorTool();});

  QHBoxLayout *hl_scale_bar = new QHBoxLayout();
  hl_scale_bar->addWidget(label_scale_bar_length);
  hl_scale_bar->addWidget(le_scale_bar_length);
  hl_scale_bar->addWidget(cbb_scale_bar_unit);

  QVBoxLayout *vl_visual = new QVBoxLayout();
  //vl_visual->addWidget(cb_sim_result_style);
  //vl_visual->addWidget(cb_publish_style);
  vl_visual->addWidget(cb_scale_bar);
  vl_visual->addLayout(hl_scale_bar);
  vl_visual->addWidget(button_set_scale_bar_anchor);
  group_scale_bar->setLayout(vl_visual);


  // Clip Setting Group
  QGroupBox *group_clip = new QGroupBox(tr("Clipping"));
  QPushButton *button_set_clip = new QPushButton(tr("Set Clip Area"));
  QPushButton *button_reset_clip = new QPushButton(tr("Reset"));
  QCheckBox *cb_preview_clip = new QCheckBox(tr("Preview Clip Area"));

  connect(button_set_clip, &QAbstractButton::clicked,
          [this, cb_preview_clip]() {
            cb_preview_clip->setChecked(true);
            emit sig_clipSelectionTool();
          }
  );
  connect(button_reset_clip, &QAbstractButton::clicked,
          [this, cb_preview_clip]() {
            cb_preview_clip->setChecked(false);
            setClipArea();
          }
  );
  connect(cb_preview_clip, &QCheckBox::stateChanged,
          this, &gui::ScreenshotManager::setClipVisibility);
  setClipVisibility(cb_preview_clip->isChecked());  // init to check state

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
            if (!dir.isEmpty())
              le_save_dir->setText(dir);
          }
  );

  // disable relevant fields when user opts to ask for path and name every time
  auto disableFilePathOptions = [this, button_browse](int cb_state) {
    bool disable = cb_state == Qt::Checked;
    le_save_dir->setDisabled(disable);
    button_browse->setDisabled(disable);
    le_name->setDisabled(disable);
    cb_overwrite->setDisabled(disable);
  };
  connect(cb_always_ask_name, &QCheckBox::stateChanged, disableFilePathOptions);
  disableFilePathOptions(cb_always_ask_name->checkState());

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
  vl_widget->addWidget(group_scale_bar);
  vl_widget->addWidget(group_clip);
  vl_widget->addWidget(group_screenshot);
  setLayout(vl_widget);


  // TODO somewhere - check if the chosen save directory is writable before attempting screenshot
}




} // end gui namespace
