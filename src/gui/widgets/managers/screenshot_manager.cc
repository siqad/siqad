// @file:     screenshot_manager.h
// @author:   Samuel
// @created:  2019.01.04
// @editted:  2019.01.04 - Samuel
// @license:  GNU LGPL v3
//
// @desc:     Implementation of screenshot manager.

#include "screenshot_manager.h"
#include "layer_manager.h"


namespace gui{

ScreenshotManager::ScreenshotManager(int misc_layer_id, LayerManager *layer_manager, QWidget *parent)
  : QWidget(parent, Qt::Dialog), misc_layer_id(misc_layer_id), layer_manager(layer_manager)
{
  if (layer_manager != nullptr) {
    lattice_layer = layer_manager->getLattice(true);
    if (lattice_layer != nullptr) {
      connect(lattice_layer, &prim::Layer::sig_visibilityChanged,
              this, &ScreenshotManager::updateLatticeClipControls);
    }
  }
  initScreenshotManager();
}

ScreenshotManager::~ScreenshotManager()
{
  // safely remove clip_area and scale_bar
  if (clip_area->scene() != 0) 
    emit sig_removeVisualAidFromDP(clip_area);
  delete clip_area;

  if (lattice_clip_area != nullptr) {
    if (lattice_clip_area->scene() != 0)
      emit sig_removeVisualAidFromDP(lattice_clip_area);
    delete lattice_clip_area;
  }

  if (scale_bar->scene() != 0) 
    emit sig_removeVisualAidFromDP(scale_bar);
  delete scale_bar;
}

void ScreenshotManager::prepareScreenshotMode(bool entering)
{
  setVisible(entering);
  setClipVisibility(cb_preview_clip->isChecked() && entering);
  setScaleBarVisibility(cb_scale_bar->isChecked() && entering);
  if (cb_preview_lattice_clip != nullptr)
    setLatticeClipVisibility(cb_preview_lattice_clip->isChecked() && entering);
}

void ScreenshotManager::setClipArea(QRectF area)
{
  area = area.normalized();
  clip_area->setSceneRect(area);

  const bool has_clip = area.isValid() && !area.isNull();
  if (has_clip) {
    if (clip_area->scene() == nullptr) {
      // add clip area to scene if it isn't already in one
      emit sig_addVisualAidToDP(clip_area);
    }
    clip_area->setVisible(cb_preview_clip->isChecked());
  } else {
    if (clip_area->scene() != nullptr) {
      // remove clip area from scene if it's in one
      emit sig_removeVisualAidFromDP(clip_area);
    }
    clip_area->setVisible(false);
  }
}

void ScreenshotManager::setClipVisibility(const bool &visible, const bool &cb_update)
{
  const bool has_clip = clip_area->sceneRect().isValid() && !clip_area->sceneRect().isNull();
  if (visible && has_clip && !clip_area->scene())
    emit sig_addVisualAidToDP(clip_area);
  clip_area->setVisible(visible);
  if (cb_update)
    cb_preview_clip->setChecked(visible);
}

void ScreenshotManager::setLatticeClipArea(QRectF area)
{
  if (lattice_clip_area == nullptr)
    return;

  area = area.normalized();
  lattice_clip_area->setSceneRect(area);

  const bool has_clip = area.isValid() && !area.isNull();
  if (has_clip && lattice_clip_area->scene() == nullptr) {
    if (lattice_layer != nullptr && lattice_layer->isVisible())
      emit sig_addVisualAidToDP(lattice_clip_area);
  } else if (!has_clip && lattice_clip_area->scene() != nullptr) {
    emit sig_removeVisualAidFromDP(lattice_clip_area);
  }

  if (!has_clip) {
    lattice_clip_preview_requested = false;
    lattice_clip_area->setVisible(false);
    if (cb_preview_lattice_clip != nullptr)
      cb_preview_lattice_clip->setChecked(false);
  } else if (cb_preview_lattice_clip != nullptr && cb_preview_lattice_clip->isChecked()
             && lattice_layer != nullptr && lattice_layer->isVisible()) {
    lattice_clip_area->setVisible(true);
  }
}

void ScreenshotManager::setLatticeClipVisibility(const bool &visible, const bool &cb_update)
{
  if (lattice_clip_area == nullptr)
    return;

  const bool has_clip = lattice_clip_area->sceneRect().isValid() && !lattice_clip_area->sceneRect().isNull();
  const bool lattice_visible = lattice_layer != nullptr && lattice_layer->isVisible();

  if (visible && has_clip && lattice_visible && !lattice_clip_area->scene())
    emit sig_addVisualAidToDP(lattice_clip_area);

  lattice_clip_area->setVisible(visible && has_clip && lattice_visible);

  if (cb_update && cb_preview_lattice_clip != nullptr)
    cb_preview_lattice_clip->setChecked(visible && has_clip && lattice_visible);
}

void ScreenshotManager::updateLatticeClipControls(bool visible)
{
  const bool enable = lattice_layer != nullptr && visible;

  if (cb_preview_lattice_clip != nullptr) {
    if (!enable) {
      lattice_clip_preview_requested = cb_preview_lattice_clip->isChecked() || lattice_clip_preview_requested;
      cb_preview_lattice_clip->setChecked(false);
    } else if (lattice_clip_preview_requested) {
      cb_preview_lattice_clip->setChecked(true);
      lattice_clip_preview_requested = false;
    }
    cb_preview_lattice_clip->setEnabled(enable);
  }

  if (pb_set_lattice_clip != nullptr)
    pb_set_lattice_clip->setEnabled(enable);
  if (pb_reset_lattice_clip != nullptr)
    pb_reset_lattice_clip->setEnabled(enable);

  if (!enable && lattice_clip_area != nullptr) {
    lattice_clip_area->setVisible(false);
  } else if (enable && cb_preview_lattice_clip != nullptr && cb_preview_lattice_clip->isChecked()) {
    setLatticeClipVisibility(true, false);
  }
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

void ScreenshotManager::setScaleBarVisibility(const bool &visible, const bool &cb_update)
{
  if (visible && !scale_bar->scene())
    emit sig_addVisualAidToDP(scale_bar);
  scale_bar->setVisible(visible);

  if (cb_update)
    cb_scale_bar->setChecked(visible);
}


// PRIVATE

void ScreenshotManager::initScreenshotManager()
{
  clip_area = new prim::ScreenshotClipArea(misc_layer_id);
  lattice_clip_area = new prim::ScreenshotClipArea(misc_layer_id);
  scale_bar = new prim::ScaleBar(misc_layer_id);

  // init GUI

  // Scale Bar Setting Group
  QGroupBox *group_scale_bar = new QGroupBox(tr("Scale Bar"));
  // TODO implement style change
  //cb_sim_result_style = new QCheckBox(tr("Simulation Result Style"));
  //cb_publish_style = new QCheckBox(tr("Publish Style"));
  cb_scale_bar = new QCheckBox(tr("Show Scale Bar"));
  QLabel *label_scale_bar_length = new QLabel(tr("Length"));
  QLineEdit *le_scale_bar_length = new QLineEdit("1");
  QComboBox *cbb_scale_bar_unit = new QComboBox();
  QPushButton *pb_set_scale_bar_anchor = new QPushButton(tr("Set Scale Bar Anchor"));
  
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
                                pb_set_scale_bar_anchor, 
                                updateScaleBarFromOptions](int cb_state) {
    bool show_scale_bar_settings = cb_state == Qt::Checked;
    le_scale_bar_length->setEnabled(show_scale_bar_settings);
    cbb_scale_bar_unit->setEnabled(show_scale_bar_settings);
    pb_set_scale_bar_anchor->setEnabled(show_scale_bar_settings);
    scale_bar->setVisible(show_scale_bar_settings);
    updateScaleBarFromOptions();
  };
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
  connect(cb_scale_bar, &QCheckBox::checkStateChanged, enableScaleBarOptions);
#else
  connect(cb_scale_bar, &QCheckBox::stateChanged, enableScaleBarOptions);
#endif
  enableScaleBarOptions(cb_scale_bar->isChecked());

  // set scale bar anchor
  connect(pb_set_scale_bar_anchor, &QAbstractButton::clicked,
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
  vl_visual->addWidget(pb_set_scale_bar_anchor);
  group_scale_bar->setLayout(vl_visual);


  // Clip Setting Group
  QGroupBox *group_clip = new QGroupBox(tr("Clipping"));
  QPushButton *pb_set_clip = new QPushButton(tr("Set Clip Area"));
  QPushButton *pb_reset_clip = new QPushButton(tr("Reset"));
  cb_preview_clip = new QCheckBox(tr("Preview Clip Area"));

  pb_set_lattice_clip = new QPushButton(tr("Set Lattice Clip Area"));
  pb_reset_lattice_clip = new QPushButton(tr("Reset"));
  cb_preview_lattice_clip = new QCheckBox(tr("Preview Lattice Clip Area"));

  connect(pb_set_clip, &QAbstractButton::clicked,
          [this]() {
            cb_preview_clip->setChecked(true);
            emit sig_clipSelectionTool();
          }
  );
  connect(pb_reset_clip, &QAbstractButton::clicked,
          [this]() {
            cb_preview_clip->setChecked(false);
            setClipArea();
          }
  );
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
  connect(cb_preview_clip, &QCheckBox::checkStateChanged,
          [this](bool state) {
            setClipVisibility(state, false);
          });
#else
  connect(cb_preview_clip, &QCheckBox::stateChanged,
          [this](bool state) {
            setClipVisibility(state, false);
          });
#endif
  setClipVisibility(cb_preview_clip->isChecked(), false);  // init to check state

  connect(pb_set_lattice_clip, &QAbstractButton::clicked,
          [this]() {
            if (lattice_layer == nullptr || !lattice_layer->isVisible())
              return;
            lattice_clip_preview_requested = true;
            cb_preview_lattice_clip->setChecked(true);
            emit sig_latticeClipSelectionTool();
          }
  );
  connect(pb_reset_lattice_clip, &QAbstractButton::clicked,
          [this]() {
            lattice_clip_preview_requested = false;
            cb_preview_lattice_clip->setChecked(false);
            setLatticeClipArea();
          }
  );
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
  connect(cb_preview_lattice_clip, &QCheckBox::checkStateChanged,
          [this](bool state) {
            setLatticeClipVisibility(state, false);
          });
#else
  connect(cb_preview_lattice_clip, &QCheckBox::stateChanged,
          [this](bool state) {
            setLatticeClipVisibility(state, false);
          });
#endif
  setLatticeClipVisibility(cb_preview_lattice_clip->isChecked(), false);

  QGroupBox *group_main_clip = new QGroupBox(tr("Screenshot Clip Area"));
  QGridLayout *grid_main_clip = new QGridLayout();
  grid_main_clip->addWidget(pb_set_clip, 0, 0);
  grid_main_clip->addWidget(pb_reset_clip, 0, 1);
  grid_main_clip->addWidget(cb_preview_clip, 1, 0, 1, 2);
  grid_main_clip->setColumnStretch(0, 1);
  grid_main_clip->setColumnStretch(1, 1);
  group_main_clip->setLayout(grid_main_clip);

  QGroupBox *group_lattice_clip = new QGroupBox(tr("Lattice Clip Area"));
  QGridLayout *grid_lattice_clip = new QGridLayout();
  grid_lattice_clip->addWidget(pb_set_lattice_clip, 0, 0);
  grid_lattice_clip->addWidget(pb_reset_lattice_clip, 0, 1);
  grid_lattice_clip->addWidget(cb_preview_lattice_clip, 1, 0, 1, 2);
  grid_lattice_clip->setColumnStretch(0, 1);
  grid_lattice_clip->setColumnStretch(1, 1);
  group_lattice_clip->setLayout(grid_lattice_clip);

  QVBoxLayout *vl_clip = new QVBoxLayout();
  vl_clip->addWidget(group_main_clip);
  vl_clip->addWidget(group_lattice_clip);
  group_clip->setLayout(vl_clip);
  updateLatticeClipControls(lattice_layer != nullptr && lattice_layer->isVisible());
        

  // TODO Toggle publish style button (while at it, probably want to revamp the whole simulation mode and publish style mode thing to status flags since multiple things can be enabled concurrently. Additional flags can be added to enable preset colorschemes)

  // Action Group
  QGroupBox *group_screenshot = new QGroupBox(tr("Screenshot"));
  QLabel *label_save_dir = new QLabel(tr("Directory"));
  le_save_dir = new QLineEdit(QDir::currentPath());  // TODO initialize to current working path
  QPushButton *pb_browse = new QPushButton(tr("..."));
  QLabel *label_name = new QLabel(tr("Name"));
  le_name = new QLineEdit(tr("siqad-screenshot.svg"));
  // TODO tooltip for name formatting
  cb_overwrite = new QCheckBox(tr("Overwrite without asking"));
  QCheckBox *cb_always_ask_name = new QCheckBox(tr("Browse for file path every time"));
  QPushButton *pb_screenshot = new QPushButton(tr("Take Screenshot"));
  QPushButton *pb_close = new QPushButton(tr("Close"));
  QDialogButtonBox *dbb_ss_buttons = new QDialogButtonBox();
  dbb_ss_buttons->addButton(pb_screenshot, QDialogButtonBox::AcceptRole);
  dbb_ss_buttons->addButton(pb_close, QDialogButtonBox::RejectRole);

  pb_screenshot->setShortcut(Qt::Key_Return);
  pb_close->setShortcut(Qt::Key_Escape);

  // browse for path and fill into le_save_dir
  connect(pb_browse, &QAbstractButton::clicked,
      [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
            "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (!dir.isEmpty())
        le_save_dir->setText(dir);
      }
  );

  // disable relevant fields when user opts to ask for path and name every time
  auto disableFilePathOptions = [this, pb_browse](int cb_state) {
    bool disable = cb_state == Qt::Checked;
    le_save_dir->setDisabled(disable);
    pb_browse->setDisabled(disable);
    le_name->setDisabled(disable);
    cb_overwrite->setDisabled(disable);
  };
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
  connect(cb_always_ask_name, &QCheckBox::checkStateChanged, disableFilePathOptions);
#else
  connect(cb_always_ask_name, &QCheckBox::stateChanged, disableFilePathOptions);
#endif
  disableFilePathOptions(cb_always_ask_name->isChecked());

  // emit screenshot signal with relevant settings
  connect(pb_screenshot, &QAbstractButton::clicked,
      [this]() {
        QString fpath = QDir(le_save_dir->text()).absoluteFilePath(le_name->text());
        QRectF clip_rect = clip_area->sceneRect().normalized();
        QRectF lattice_rect = lattice_clip_area != nullptr ? lattice_clip_area->sceneRect().normalized() : QRectF();
        const bool lattice_visible = lattice_layer != nullptr && lattice_layer->isVisible();
        const bool lattice_clip_set = lattice_clip_area != nullptr && lattice_rect.isValid() && !lattice_rect.isNull();
        const bool clip_set = clip_rect.isValid() && !clip_rect.isNull();

        if (lattice_visible && lattice_clip_set && clip_set && !clip_rect.intersects(lattice_rect)) {
          QMessageBox::StandardButton reply = QMessageBox::warning(
              this,
              tr("No Lattice In Screenshot"),
              tr("The lattice clip area does not overlap with the screenshot clip area. "
                 "The screenshot will not include any lattice sites. Do you want to continue?"),
              QMessageBox::Ok | QMessageBox::Cancel);
          if (reply == QMessageBox::Cancel)
            return;
        }

        emit sig_takeScreenshot(fpath, clip_rect, cb_overwrite->isChecked());
      }
  );

  // close dialog
  connect(pb_close, &QAbstractButton::clicked,
      [this]() {
        close();
      }
  );
      
  QHBoxLayout *hl_save_dir = new QHBoxLayout();
  hl_save_dir->addWidget(label_save_dir);
  hl_save_dir->addWidget(le_save_dir);
  hl_save_dir->addWidget(pb_browse);

  QFormLayout *fl_screenshot = new QFormLayout();
  fl_screenshot->addRow(hl_save_dir);
  fl_screenshot->addRow(label_name, le_name);
  fl_screenshot->addRow(cb_overwrite);
  fl_screenshot->addRow(cb_always_ask_name);
  group_screenshot->setLayout(fl_screenshot);

  // Add them to this widget
  QVBoxLayout *vl_widget = new QVBoxLayout();
  vl_widget->addWidget(group_scale_bar);
  vl_widget->addWidget(group_clip);
  vl_widget->addWidget(group_screenshot);
  vl_widget->addWidget(dbb_ss_buttons);
  setLayout(vl_widget);


  // TODO somewhere - check if the chosen save directory is writable before attempting screenshot
}


void ScreenshotManager::closeEvent(QCloseEvent *bar)
{
  // emit signal informing design panel of closing
  emit sig_closeEventTriggered();

  bar->accept();
}



} // end gui namespace
