#include "scene_fixture.h"
#include "gui/widgets/components/job_results/db_locations.h"
#include "gui/widgets/visualizers/electron_config_set_visualizer.h"
#include <QBuffer>
#include <QDomDocument>
#include <QNativeGestureEvent>
#include <QWheelEvent>
#include <functional>
using test_support::Scene;

class FailingOutput : public QIODevice {
public:
  int writes = 0;
  FailingOutput() { open(QIODevice::WriteOnly); }
protected:
  qint64 readData(char*, qint64) override { return -1; }
  qint64 writeData(const char*, qint64) override { ++writes; return -1; }
};

class ChunkedInput : public QBuffer {
public:
  std::function<void()> onRead;
protected:
  qint64 readData(char *data, qint64 size) override {
    if (onRead) onRead();
    return QBuffer::readData(data, qMin<qint64>(64, size));
  }
};

class GuiWorkflowTests : public QObject {
  Q_OBJECT
private slots:
  void cleanup() { test_support::resetCase(); }

  void settingsAreIsolated() {
    const auto root = qEnvironmentVariable("SIQAD_PROFILE_ROOT");
    QVERIFY(!root.isEmpty());
    QVERIFY(settings::GUISettings::instance()->fileName().startsWith(root + "/"));
    QVERIFY(settings::Settings::pathReplacement("<SYSTMP>/autosave").startsWith(root + "/"));
    QVERIFY(settings::Settings::pathReplacement("<APPLOCALDATA>/plugins").startsWith(root + "/"));
    settings::GUISettings::instance()->setValue("fixture/sentinel", "removed by cleanup");
  }
  void previousCaseDoesNotLeakSettings() {
    QVERIFY(!settings::GUISettings::instance()->contains("fixture/sentinel"));
  }

  void svgClippingAndPreviewRestoration() {
    Scene panel;
    auto *manager = panel.findChild<gui::ScreenshotManager*>();
    QVERIFY(manager);
    QVERIFY(panel.commandCreateItem("DBDot", "auto", {"0", "0", "0"}));
    QCOMPARE(panel.dbs().size(), 1);
    panel.setDisplayMode(gui::ScreenshotMode);
    manager->setScaleBarVisibility(false, true);
    auto *dot = panel.dbs().first();
    dot->setSelected(true);
    const QRectF region(-500, -500, 2000, 2000);
    // Clip between lattice sites: only the design DB should remain in the SVG.
    manager->setLatticeClipArea(QRectF(100, 40, 60, 10));
    manager->setLatticeClipVisibility(false, true);
    const auto withoutPreview = panel.svg("clip-preview-off.svg", region);
    QVERIFY(!withoutPreview.isEmpty());
    manager->setLatticeClipVisibility(true, true);
    const auto withPreview = panel.svg("clip-preview-on.svg", region);
    QCOMPARE(withPreview, withoutPreview);
    QVERIFY(manager->latticeClipVisible());
    QVERIFY(dot->isSelected());
    QVERIFY(panel.getLattice(true)->isVisible());
    QDomDocument doc;
    QVERIFY(doc.setContent(withPreview));
    QCOMPARE(doc.documentElement().tagName(), QString("svg"));
    QCOMPARE(doc.documentElement().attribute("viewBox"), QString("0 0 320 240"));
    const int designCircles = doc.elementsByTagName("circle").count() + doc.elementsByTagName("ellipse").count();
    QVERIFY(designCircles > 0);
    manager->setLatticeClipArea(QRectF(300, -40, 150, 80)); // contains the (1,0,0) site
    const auto withSite = panel.svg("clip-one-site.svg", region);
    QVERIFY(doc.setContent(withSite));
    QCOMPARE(doc.elementsByTagName("circle").count() + doc.elementsByTagName("ellipse").count(), designCircles + 1);
    bool hasTransform = false;
    const auto groups = doc.elementsByTagName("g");
    for (int i = 0; i < groups.count(); ++i)
      hasTransform |= groups.at(i).toElement().hasAttribute("transform");
    QVERIFY(hasTransform);
  }

  void documentRoundTripAndResultLayers() {
    Scene panel;
    QVERIFY(panel.load(test_support::fixture("nested-sidbs.sqd")));
    QCOMPARE(panel.dbs().size(), 3);
    const auto saved = test_support::artifact("document-roundtrip.sqd");
    QVERIFY(panel.save(saved));
    QVERIFY(panel.load(saved));
    QCOMPARE(panel.dbs().size(), 3);
    QVERIFY(panel.load(test_support::fixture("nested-sidbs.sqd"), true));
    panel.enableSimVis();
    QVERIFY(panel.getLattice(false)->isOccupied(prim::LatticeCoord(0, 0, 0)));
    QVERIFY(panel.getLattice(false)->isOccupied(prim::LatticeCoord(4, 2, 0)));
    auto *manager = panel.findChild<gui::ScreenshotManager*>();
    panel.setDisplayMode(gui::ScreenshotMode);
    manager->setLatticeClipArea(QRectF(-100, -100, 1000, 1000));
    QVERIFY(manager->findChild<QPushButton*>("setLatticeClip")->isEnabled());
    panel.getLattice(false)->setVisible(false);
    QVERIFY(!manager->findChild<QPushButton*>("setLatticeClip")->isEnabled());
    panel.layerManager()->setSimVisualizeMode(false);
    QVERIFY(panel.getLattice(true)->isVisible());
    QCOMPARE(panel.dbs().size(), 3);
  }

  void exportFailureAndRotatedViewRestoreState() {
    Scene panel;
    QVERIFY(panel.commandCreateItem("DBDot", "auto", {"0", "0", "0"}));
    auto *dot = panel.dbs().first();
    dot->setSelected(true);
    panel.setDisplayMode(gui::ScreenshotMode);
    auto *manager = panel.findChild<gui::ScreenshotManager*>();
    manager->setScaleBarVisibility(false, true);
    manager->setLatticeClipArea(QRectF(100, 40, 60, 10));
    manager->setLatticeClipVisibility(true, true);
    const QRectF region(-500, -500, 2000, 2000);
    const auto baseline = panel.svg("export-before-failure.svg", region);
    QVERIFY(!baseline.isEmpty());
    panel.rotate(30);
    QCOMPARE(panel.svg("export-rotated-view.svg", region), baseline);
    FailingOutput output;
    QSvgGenerator generator;
    generator.setOutputDevice(&output);
    generator.setSize(QSize(320, 240)); generator.setViewBox(QRect(0, 0, 320, 240));
    QPainter painter;
    QVERIFY(painter.begin(&generator));
    panel.screenshot(&painter, region, QRectF(0, 0, 320, 240));
    painter.end();
    QVERIFY(output.writes > 0);
    QVERIFY(manager->latticeClipVisible());
    QVERIFY(panel.getLattice(true)->isVisible());
    QVERIFY(dot->isSelected());
    QCOMPARE(panel.svg("export-after-failure.svg", region), baseline);
  }

  void customLatticeExport() {
    Scene panel;
    QFile input(test_support::fixture("nested-sidbs.sqd"));
    QVERIFY(input.open(QIODevice::ReadOnly));
    auto bytes = input.readAll();
    bytes.replace("3.84", "5.00");
    const auto path = test_support::artifact("custom-lattice.sqd");
    QFile output(path); QVERIFY(output.open(QIODevice::WriteOnly));
    QCOMPARE(output.write(bytes), bytes.size()); output.close();
    QVERIFY(panel.load(path));
    const auto site = panel.getLattice(true)->latticeCoord2ScenePos(prim::LatticeCoord(1, 0, 0));
    auto *manager = panel.findChild<gui::ScreenshotManager*>();
    QVERIFY(manager);
    panel.setDisplayMode(gui::ScreenshotMode);
    manager->setScaleBarVisibility(false, true);
    manager->setLatticeClipArea(QRectF(site - QPointF(10, 10), QSizeF(20, 20)));
    QDomDocument doc;
    QVERIFY(doc.setContent(panel.svg("custom-lattice.svg", QRectF(-500, -500, 4000, 4000))));
    // Three occupied DBs and exactly one empty site in the tight clip.
    QCOMPARE(doc.elementsByTagName("circle").count() + doc.elementsByTagName("ellipse").count(), 4);
  }

  void recordedLocationResult() {
    QFile file(test_support::fixture("db-locations.xml"));
    QVERIFY(file.open(QIODevice::ReadOnly));
    QXmlStreamReader reader(&file);
    QVERIFY(reader.readNextStartElement());
    comp::DBLocations locations(&reader);
    QVERIFY(!reader.hasError());
    const QList<QPointF> expected{{0, 0}, {11.52, 2.25}, {15.36, 15.36}};
    QCOMPARE(locations.locations().size(), expected.size());
    for (int i = 0; i < expected.size(); ++i)
      QVERIFY((locations.locations()[i] - expected[i]).manhattanLength() < 1e-5);
  }

  void recordedChargeOverlay() {
    Scene panel;
    QVERIFY(panel.load(test_support::fixture("nested-sidbs.sqd"), true));
    panel.enableSimVis();
    QFile file(test_support::fixture("charge-configs.xml"));
    QVERIFY(file.open(QIODevice::ReadOnly));
    QXmlStreamReader reader(&file);
    QVERIFY(reader.readNextStartElement());
    comp::ChargeConfigSet configs(&reader);
    QVERIFY(!reader.hasError());
    QCOMPARE(configs.totalConfigCount(), 5);
    const QList<QPointF> positions{{0, 0}, {11.52, 2.25}, {15.36, 15.36}};
    const auto dbs = panel.getLattice(false)->dbsAtPhysLocs(positions);
    QCOMPARE(dbs.size(), 3);
    panel.setDisplayMode(gui::ScreenshotMode);
    auto *manager = panel.findChild<gui::ScreenshotManager*>();
    manager->setScaleBarVisibility(false, true);
    manager->setLatticeClipArea(QRectF(100, 40, 60, 10));
    const QRectF region(-500, -500, 4000, 4000);
    const auto before = panel.svg("charge-before.svg", region);
    QVERIFY(!before.isEmpty());
    gui::ChargeConfigSetVisualizer visualizer(panel.getLattice(false));
    const auto charge = configs.chargeConfigs(true).first();
    QCOMPARE(charge.config, QList<int>({1, 0, -1}));
    visualizer.showChargeConfigResult(charge, positions);
    const auto charged = panel.svg("charge-overlay.svg", region);
    QVERIFY(!charged.isEmpty());
    QVERIFY(charged != before);
    visualizer.clearChargeConfigResult();
    QCOMPARE(panel.svg("charge-cleared.svg", region), before);
    panel.layerManager()->setSimVisualizeMode(false);
    QVERIFY(panel.getLattice(true)->isVisible());
  }

  void loadingSuppressesInputAndRestoresTouch() {
    Scene panel;
    QFile file(test_support::fixture("nested-sidbs.sqd"));
    QVERIFY(file.open(QIODevice::ReadOnly));
    ChunkedInput input;
    input.setData(file.readAll()); QVERIFY(input.open(QIODevice::ReadOnly));
    QXmlStreamReader reader(&input);
    QVERIFY(reader.readNextStartElement());
    int observations = 0;
    input.onRead = [&] {
      if (panel.viewport()->testAttribute(Qt::WA_AcceptTouchEvents)) return;
      ++observations;
      const auto before = panel.transform();
      const QPointF anchor(150, 150);
      QWheelEvent event(anchor, panel.viewport()->mapToGlobal(anchor.toPoint()),
          QPoint(0, 20), QPoint(), Qt::NoButton, Qt::ControlModifier, Qt::ScrollUpdate, false);
      QApplication::sendEvent(panel.viewport(), &event);
      QCOMPARE(panel.transform(), before);
      QVERIFY(!event.isAccepted());
    };
    panel.loadFromFile(&reader);
    QVERIFY(!reader.hasError());
    QVERIFY(observations > 0);
    QVERIFY(panel.viewport()->testAttribute(Qt::WA_AcceptTouchEvents));
    QCOMPARE(panel.dbs().size(), 3);
  }

  void nativePinchSequenceAndAnchor() {
#if defined(Q_OS_MACOS) && QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    Scene panel;
    const QPointF anchor(150, 150);
    const auto anchored = panel.mapToScene(anchor.toPoint());
    auto send = [&](Qt::NativeGestureType type, qreal value) {
      QNativeGestureEvent event(type, QPointingDevice::primaryPointingDevice(), 2,
          anchor, anchor, panel.viewport()->mapToGlobal(anchor.toPoint()), value, QPointF(), 1);
      QApplication::sendEvent(panel.viewport(), &event);
      QCoreApplication::processEvents();
    };
    const auto before = panel.transform().m11();
    send(Qt::BeginNativeGesture, 0);
    for (int i = 0; i < 5; ++i) send(Qt::ZoomNativeGesture, 0.05);
    const auto zoomed = panel.transform().m11();
    QVERIFY(zoomed > before);
    QVERIFY((panel.mapToScene(anchor.toPoint()) - anchored).manhattanLength() * zoomed < 8);
    send(Qt::EndNativeGesture, 0);
    QCOMPARE(panel.transform().m11(), zoomed);
    QEvent cancel(QEvent::TouchCancel);
    QApplication::sendEvent(panel.viewport(), &cancel);
    QCOMPARE(panel.transform().m11(), zoomed);
    send(Qt::BeginNativeGesture, 0);
    send(Qt::ZoomNativeGesture, -0.05);
    send(Qt::EndNativeGesture, 0);
    QVERIFY(panel.transform().m11() < zoomed);
#else
    QSKIP("Native macOS pinch sequence requires Qt >= 6.2 on macOS");
#endif
  }

  void wheelPanSequences() {
    Scene panel;
    const auto before = panel.transform();
    const int h = panel.horizontalScrollBar()->value(), v = panel.verticalScrollBar()->value();
    const QPointF anchor(150, 150);
    QWheelEvent horizontal(anchor, panel.viewport()->mapToGlobal(anchor.toPoint()),
        QPoint(20, 0), QPoint(), Qt::NoButton, Qt::NoModifier, Qt::ScrollUpdate, false);
    QApplication::sendEvent(panel.viewport(), &horizontal);
    QVERIFY(horizontal.isAccepted());
    QVERIFY(panel.horizontalScrollBar()->value() != h);
    QCOMPARE(panel.verticalScrollBar()->value(), v);
    QWheelEvent vertical(anchor, panel.viewport()->mapToGlobal(anchor.toPoint()),
        QPoint(), QPoint(0, 120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(panel.viewport(), &vertical);
    QVERIFY(panel.verticalScrollBar()->value() != v);
    QCOMPARE(panel.transform(), before);
  }

  void zoomLimitsAndModeReentry() {
    Scene panel;
    for (int i = 0; i < 300; ++i) panel.stepZoom(true);
    const auto maximum = settings::GUISettings::instance()->get<qreal>("view/zoom_max");
    QVERIFY(panel.transform().m11() <= maximum + 1e-8);
    for (int i = 0; i < 500; ++i) panel.stepZoom(false);
    const auto minimum = settings::GUISettings::instance()->get<qreal>("view/zoom_min");
    QVERIFY(panel.transform().m11() >= minimum - 1e-8);
    auto *manager = panel.findChild<gui::ScreenshotManager*>();
    panel.setDisplayMode(gui::ScreenshotMode);
    manager->setLatticeClipArea(QRectF(0, 0, 1000, 1000));
    manager->setLatticeClipVisibility(true, true);
    panel.setDisplayMode(gui::DesignMode);
    QVERIFY(!manager->latticeClipVisible());
    panel.setDisplayMode(gui::ScreenshotMode);
    QVERIFY(manager->latticeClipVisible());
    manager->findChild<QPushButton*>("resetLatticeClip")->click();
    QVERIFY(!manager->latticeClipVisible());
  }

  void wheelModifiersAndAnchor() {
    Scene panel;
    auto *preferences = settings::GUISettings::instance();
    preferences->setValue("view/wheel_pan_step", 15.0);
    preferences->setValue("view/wheel_pan_boost", 2.0);
    preferences->setValue("view/scroll_direction", "normal");
    const QPointF anchor(150, 150);
    auto wheel = [&](Qt::KeyboardModifier modifier) {
      QWheelEvent event(anchor, panel.viewport()->mapToGlobal(anchor.toPoint()),
          QPoint(0, 20), QPoint(), Qt::NoButton, modifier, Qt::ScrollUpdate, false);
      QApplication::sendEvent(panel.viewport(), &event);
      return event.isAccepted();
    };
    const int initial = panel.verticalScrollBar()->value();
    QVERIFY(wheel(Qt::NoModifier));
    const int normal = qAbs(panel.verticalScrollBar()->value() - initial);
    QVERIFY(normal > 0);
    const int v = panel.verticalScrollBar()->value(), h = panel.horizontalScrollBar()->value();
    QVERIFY(wheel(Qt::ShiftModifier));
    QCOMPARE(panel.verticalScrollBar()->value(), v);
    QCOMPARE(qAbs(panel.horizontalScrollBar()->value() - h), normal);
    QVERIFY(wheel(Qt::AltModifier));
    QCOMPARE(qAbs(panel.verticalScrollBar()->value() - v), normal * 2);
    const auto transform = panel.transform();
    const auto sceneAnchor = panel.mapToScene(anchor.toPoint());
    QVERIFY(wheel(Qt::ControlModifier));
    QVERIFY(panel.transform().m11() > transform.m11());
    QVERIFY((panel.mapToScene(anchor.toPoint()) - sceneAnchor).manhattanLength()
        * panel.transform().m11() < 3);
  }
};
SIQAD_TEST_MAIN(GuiWorkflowTests)
#include "gui_workflow_tests.moc"
