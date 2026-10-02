#include <QtTest/QtTest>
#include <QClipboard>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeData>
#include <QNativeGestureEvent>
#include <QPointingDevice>

#include "gui/widgets/design_panel.h"
#include "scene_fixture.h"
#include "test_data.h"

namespace {
using test_data::db;
using test_data::aggregate;

void setClipboard(const QJsonArray &items)
{
  const QJsonObject root{{"format", "siqad-sidb-selection"}, {"version", 1}, {"items", items}};
  auto *mime = new QMimeData;
  mime->setData("application/x-siqad-sidb-selection", QJsonDocument(root).toJson());
  QApplication::clipboard()->setMimeData(mime);
}

template<class T>
T *control(gui::ScreenshotManager *manager, const QString &name)
{
  return manager->findChild<T*>(name);
}
} // namespace

class SiQADTests : public QObject
{
  Q_OBJECT

private slots:
  void initTestCase()
  {
    QStandardPaths::setTestModeEnabled(true);
  }

  void cleanup()
  {
    test_support::resetCase();
  }

  void rejectsInvalidClipboard_data()
  {
    QTest::addColumn<QJsonArray>("items");
    QTest::newRow("duplicate-sites") << QJsonArray{db(), db()};
    QTest::newRow("duplicate-across-nested-groups")
        << QJsonArray{db(), aggregate({aggregate({db()})})};
    QTest::newRow("empty-aggregate") << QJsonArray{db(), aggregate({})};
    QTest::newRow("empty-selection") << QJsonArray{};
    QTest::newRow("invalid-basis") << QJsonArray{db(0, 0, 2)};
    QTest::newRow("negative-basis") << QJsonArray{db(0, 0, -1)};
    QJsonObject fractional = db();
    fractional["lat"] = QJsonObject{{"n", 0.5}, {"m", 0}, {"l", 0}};
    QTest::newRow("fractional-coordinate") << QJsonArray{fractional};
    QJsonObject overflow = db();
    overflow["lat"] = QJsonObject{{"n", 1e30}, {"m", 0}, {"l", 0}};
    QTest::newRow("coordinate-overflow") << QJsonArray{overflow};
    QJsonObject invalid_layer = db();
    invalid_layer["layer"] = -2;
    QTest::newRow("negative-layer") << QJsonArray{invalid_layer};
    QJsonObject invalid_children = aggregate({db()});
    invalid_children["children"] = "invalid";
    QTest::newRow("wrong-children-type") << QJsonArray{invalid_children};
  }

  void rejectsInvalidClipboard()
  {
    QFETCH(QJsonArray, items);
    test_support::Scene panel;
    setClipboard(items);
    QVERIFY(QMetaObject::invokeMethod(&panel, "pasteAction", Qt::DirectConnection));
    QVERIFY(!prim::Ghost::instance()->isVisible());
    QVERIFY(prim::Ghost::instance()->getSources().isEmpty());
    QVERIFY(panel.getAllDBs().isEmpty());
  }

  void acceptsNestedClipboardAndSignedCoordinates()
  {
    test_support::Scene panel;
    setClipboard({aggregate({db(-3, -2, 0), aggregate({db(-2, -2, 1)})})});
    QVERIFY(QMetaObject::invokeMethod(&panel, "pasteAction", Qt::DirectConnection));
    auto *ghost = prim::Ghost::instance();
    QVERIFY(ghost->isVisible());
    QCOMPARE(ghost->getSources().size(), 2);
    QCOMPARE(ghost->getTopItems().size(), 1);
    auto *first = static_cast<prim::DBDot*>(ghost->getSources().first());
    QCOMPARE(first->latticeCoord().n, -3);
    QCOMPARE(first->latticeCoord().m, -2);
  }

  void invalidImportDoesNotPasteStaleSelection()
  {
    test_support::Scene panel;
    QVERIFY(panel.commandCreateItem("DBDot", "auto", {"2", "3", "0"}));
    panel.getAllDBs().first()->setSelected(true);
    QVERIFY(QMetaObject::invokeMethod(&panel, "copyAction", Qt::DirectConnection));
    setClipboard({db(), db()});
    QVERIFY(QMetaObject::invokeMethod(&panel, "pasteAction", Qt::DirectConnection));
    QVERIFY(!prim::Ghost::instance()->isVisible());
    QCOMPARE(panel.getAllDBs().size(), 1);
  }

  void firstLatticeClipRetainsPreview()
  {
    test_support::Scene panel;
    auto *manager = panel.findChild<gui::ScreenshotManager*>();
    QVERIFY(manager);
    panel.setDisplayMode(gui::ScreenshotMode);
    auto *preview = control<QCheckBox>(manager, "latticeClipPreview");
    auto *set_clip = control<QPushButton>(manager, "setLatticeClip");
    QVERIFY(preview && set_clip);
    connect(&panel, &gui::DesignPanel::sig_toolChangeRequest, &panel, &gui::DesignPanel::setTool);
    set_clip->click();
    QVERIFY(preview->isChecked());
    QVERIFY(!manager->latticeClipVisible());
    manager->setLatticeClipArea(QRectF(0, 0, 1000, 1000));
    QVERIFY(preview->isChecked());
    QVERIFY(manager->latticeClipVisible());
    manager->setLatticeClipArea();
    QVERIFY(!preview->isChecked());
    QVERIFY(!manager->latticeClipVisible());
  }

  void singleMoveDragSelectsItems()
  {
    test_support::Scene panel;
    panel.resize(640, 480);
    panel.show();
    QCoreApplication::processEvents();
    QVERIFY(panel.commandCreateItem("DBDot", "auto", {"0", "0", "0"}));
    panel.setTool(gui::SelectTool);
    const QPoint center = panel.mapFromScene(panel.getAllDBs().first()->pos());
    const QPoint start = center - QPoint(60, 60);
    const QPoint end = center + QPoint(60, 60);
    QTest::mousePress(panel.viewport(), Qt::LeftButton, Qt::NoModifier, start);
    QMouseEvent move(QEvent::MouseMove, end, panel.viewport()->mapToGlobal(end),
                     Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(panel.viewport(), &move);
    QTest::mouseRelease(panel.viewport(), Qt::LeftButton, Qt::NoModifier, end);
    QVERIFY(panel.getAllDBs().first()->isSelected());
  }

  void latticeClipTracksSimulationVisibility()
  {
    test_support::Scene panel;
    auto *manager = panel.findChild<gui::ScreenshotManager*>();
    QVERIFY(manager);
    panel.setDisplayMode(gui::ScreenshotMode);
    auto *set_clip = control<QPushButton>(manager, "setLatticeClip");
    auto *preview = control<QCheckBox>(manager, "latticeClipPreview");
    QVERIFY(set_clip && preview);
    manager->setLatticeClipArea(QRectF(0, 0, 1000, 1000));
    manager->setLatticeClipVisibility(true, true);
    panel.enableSimVis();
    QVERIFY(!panel.getLattice(true)->isVisible());
    QVERIFY(panel.getLattice(false)->isVisible());
    QVERIFY(set_clip->isEnabled());
    QVERIFY(preview->isChecked());
    QVERIFY(manager->latticeClipVisible());
    panel.getLattice(false)->setVisible(false);
    QVERIFY(!set_clip->isEnabled());
    QVERIFY(!manager->latticeClipVisible());
    QVERIFY(preview->isChecked());
    panel.getLattice(false)->setVisible(true);
    QVERIFY(set_clip->isEnabled());
    QVERIFY(manager->latticeClipVisible());
    panel.layerManager()->setSimVisualizeMode(false);
    QVERIFY(set_clip->isEnabled());
    QVERIFY(manager->latticeClipVisible());
  }

  void latticeClipDoesNotRoundOutward()
  {
    test_support::Scene panel;
    auto *manager = panel.findChild<gui::ScreenshotManager*>();
    QVERIFY(manager);
    panel.setDisplayMode(gui::ScreenshotMode);
    const qreal cell_width = panel.getLattice(true)->sceneLatticeVector(0).x();
    const qreal cell_height = panel.getLattice(true)->sceneLatticeVector(1).y();
    const QRectF between_sites(cell_width * 0.3, cell_height * 0.1,
                               cell_width * 0.4, cell_height * 0.05);
    manager->setLatticeClipArea(between_sites);
    manager->setLatticeClipVisibility(false, true);
    manager->setScaleBarVisibility(false, true);
    const QColor background = settings::GUISettings::instance()->get<QColor>("view/bg_col_pb");
    QImage image(240, 240, QImage::Format_ARGB32);
    image.fill(background);
    QPainter painter(&image);
    panel.screenshot(&painter, QRectF(-cell_width, -cell_height,
                                     cell_width * 3, cell_height * 3), image.rect());
    painter.end();
    for (int y = 0; y < image.height(); ++y)
      for (int x = 0; x < image.width(); ++x)
        QCOMPARE(image.pixelColor(x, y), background);
    QVERIFY(panel.getLattice(true)->isVisible());
    QVERIFY(!manager->latticeClipVisible());

    // A clip containing an actual site must still draw lattice graphics.
    manager->setLatticeClipArea(QRectF(-cell_width * 0.2, -cell_height * 0.1,
                                      cell_width * 0.4, cell_height * 0.2));
    image.fill(background);
    QPainter site_painter(&image);
    panel.screenshot(&site_painter, QRectF(-cell_width, -cell_height,
                                          cell_width * 3, cell_height * 3), image.rect());
    site_painter.end();
    bool site_drawn = false;
    for (int y = 0; y < image.height(); ++y)
      for (int x = 0; x < image.width(); ++x)
        site_drawn = site_drawn || image.pixelColor(x, y) != background;
    QVERIFY(site_drawn);
  }

  void pinchFinishDoesNotRepeatZoom()
  {
#if defined(Q_OS_MACOS) && QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    test_support::Scene panel;
    panel.resize(640, 480);
    panel.show();
    QCoreApplication::processEvents();
    const QPointF anchor(150, 150);
    auto send = [&](Qt::NativeGestureType type, qreal value) {
      QNativeGestureEvent event(type, QPointingDevice::primaryPointingDevice(), 2,
          anchor, anchor, panel.viewport()->mapToGlobal(anchor.toPoint()), value, QPointF(), 1);
      QApplication::sendEvent(panel.viewport(), &event);
      QCoreApplication::processEvents();
    };
    const qreal before = panel.transform().m11();
    send(Qt::BeginNativeGesture, 0);
    send(Qt::ZoomNativeGesture, 0.1);
    const qreal zoomed = panel.transform().m11();
    QVERIFY(zoomed > before);
    send(Qt::EndNativeGesture, 0);
    QCOMPARE(panel.transform().m11(), zoomed);
#else
    QSKIP("Native macOS gesture regression requires Qt >= 6.2 on macOS");
#endif
  }
};

SIQAD_TEST_MAIN(SiQADTests)
#include "siqad_tests.moc"
