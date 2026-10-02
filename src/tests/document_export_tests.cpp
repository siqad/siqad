#include "scene_fixture.h"
#include <QDomDocument>
#include <QRegularExpression>
#include <QSvgRenderer>

using test_support::Scene;

static QString siteKey(prim::DBDot *dot) {
  const auto c = dot->latticeCoord();
  return QString("%1,%2,%3").arg(c.n).arg(c.m).arg(c.l);
}

// SVG assertions measure geometry in output coordinates, independent of fonts.
static QList<double> numbers(const QString &text) {
  QList<double> result;
  const QRegularExpression number(R"([-+]?(?:\d*\.\d+|\d+\.?\d*)(?:[eE][-+]?\d+)?)");
  auto matches = number.globalMatch(text);
  while (matches.hasNext()) result << matches.next().captured().toDouble();
  return result;
}
static QString inherited(QDomElement element, const QString &attribute) {
  while (!element.isNull()) {
    if (element.hasAttribute(attribute)) return element.attribute(attribute);
    element = element.parentNode().toElement();
  }
  return {};
}
struct BarGeometry { QPointF start, end; double width = 0; };
static QList<BarGeometry> bars(const QByteArray &svg) {
  QDomDocument doc;
  if (!doc.setContent(svg)) return {};
  QList<BarGeometry> result;
  const auto lines = doc.elementsByTagName("polyline");
  for (int i = 0; i < lines.count(); ++i) {
    auto element = lines.at(i).toElement();
    const auto points = numbers(element.attribute("points"));
    if (points.size() != 4) continue;
    QPointF start(points[0], points[1]), end(points[2], points[3]);
    const double stroke = inherited(element, "stroke-width").toDouble();
    QPointF widthStart(0, 0), widthEnd(stroke, 0);
    for (auto parent = element; !parent.isNull(); parent = parent.parentNode().toElement()) {
      const auto transform = parent.attribute("transform");
      if (transform.isEmpty()) continue;
      // QSvgGenerator emits matrix transforms. Fail rather than ignore a new format.
      const auto values = numbers(transform);
      if (!transform.startsWith("matrix(") || values.size() != 6) return {};
      QTransform matrix(values[0], values[1], values[2], values[3], values[4], values[5]);
      start = matrix.map(start); end = matrix.map(end);
      widthStart = matrix.map(widthStart); widthEnd = matrix.map(widthEnd);
    }
    result << BarGeometry{start, end, QLineF(widthStart, widthEnd).length()};
  }
  return result;
}

class DocumentExportTests : public QObject {
  Q_OBJECT
private slots:
  void cleanup() { test_support::resetCase(); }

  void historicalDocumentsMigrate_data() {
    QTest::addColumn<QString>("file");
    QTest::addColumn<int>("aggregates");
    QTest::addColumn<bool>("hasColors");
    QTest::newRow("physical-only-root-layers") << QString("legacy-physical.sqd") << 0 << false;
    QTest::newRow("v0.1-lattice-coordinates") << QString("v0.1-lattice.sqd") << 0 << false;
    QTest::newRow("v0.2.1-nested-colors") << QString("v0.2.1-aggregates.sqd") << 2 << true;
  }
  void historicalDocumentsMigrate() {
    QFETCH(QString, file); QFETCH(int, aggregates); QFETCH(bool, hasColors);
    Scene panel(false);
    QVERIFY(panel.load(test_support::fixture(file)));
    const QMap<QString, QPointF> expected{{"0,0,0", {0,0}}, {"-1,-1,1", {-3.84,-5.43}}, {"3,0,1", {11.52,2.25}}};
    const QColor fallback = settings::GUISettings::instance()->get<QColor>("dbdot/fill_col");
    const QMap<QString, QColor> colors{{"0,0,0", QColor("#ff112233")}, {"-1,-1,1", QColor("#ff445566")}, {"3,0,1", QColor("#ff778899")}};
    const auto saved = test_support::artifact(file + "-migrated.sqd");
    for (int round = 0; round < 2; ++round) {
      QCOMPARE(panel.dbs().size(), 3);
      QSet<QString> sites;
      for (auto *dot : panel.dbs()) {
        const auto key = siteKey(dot);
        QVERIFY(expected.contains(key)); QVERIFY(!sites.contains(key)); sites.insert(key);
        QVERIFY((dot->physLoc() - expected.value(key)).manhattanLength() < 1e-5);
        QVERIFY(panel.getLattice(true)->isOccupied(dot->latticeCoord()));
        QCOMPARE(dot->getCurrentFillColor(), hasColors ? colors.value(key) : fallback);
      }
      auto *surface = panel.layerManager()->getLayer("Surface");
      auto *metal = panel.layerManager()->getLayer("Metal");
      QVERIFY(surface); QVERIFY(metal);
      QCOMPARE(surface->role(), prim::Layer::Design);
      QCOMPARE(metal->role(), prim::Layer::Design);
      QCOMPARE(metal->zOffset(), 12.f); QCOMPARE(metal->zHeight(), 3.f);
      QVERIFY(surface->isVisible()); QVERIFY(surface->isActive());
      QVERIFY(panel.save(saved));
      QFile output(saved); QVERIFY(output.open(QIODevice::ReadOnly));
      QDomDocument doc; QVERIFY(doc.setContent(output.readAll()));
      QCOMPARE(doc.elementsByTagName("latcoord").count(), 3);
      QCOMPARE(doc.elementsByTagName("aggregate").count(), aggregates);
      if (aggregates) {
        const auto outer = doc.elementsByTagName("aggregate").at(0).toElement();
        const auto inner = outer.firstChildElement("aggregate");
        QVERIFY(!outer.firstChildElement("dbdot").isNull());
        QCOMPARE(inner.elementsByTagName("dbdot").count(), 2);
      }
      QVERIFY(!doc.elementsByTagName("lat_vec").isEmpty());
      if (round == 0) QVERIFY(panel.load(saved));
    }
  }

  void latticeCoordinatesOverridePhysicalMetadata() {
    Scene panel(false);
    QFile input(test_support::fixture("v0.1-lattice.sqd")); QVERIFY(input.open(QIODevice::ReadOnly));
    auto xml = input.readAll();
    xml.replace("<physloc x=\"0\" y=\"0\"", "<physloc x=\"100\" y=\"100\"");
    QXmlStreamReader reader(xml); QVERIFY(reader.readNextStartElement());
    panel.loadFromFile(&reader); QVERIFY(!reader.hasError());
    QCOMPARE(panel.dbs().size(), 3);
    QVERIFY(panel.getLattice(true)->isOccupied(prim::LatticeCoord(0,0,0)));
  }

  void scaleBarUnitsAndAnchor_data() {
    QTest::addColumn<float>("length"); QTest::addColumn<int>("unit");
    QTest::addColumn<double>("pixels");
    QTest::newRow("picometers") << 1000.f << int(gui::Unit::pm) << 100.;
    QTest::newRow("angstroms") << 10.f << int(gui::Unit::ang) << 100.;
    QTest::newRow("nanometers") << 1.f << int(gui::Unit::nm) << 100.;
    QTest::newRow("micrometers") << 0.001f << int(gui::Unit::um) << 100.;
    QTest::newRow("millimeters") << 0.000001f << int(gui::Unit::mm) << 100.;
    QTest::newRow("meters") << 0.000000001f << int(gui::Unit::m) << 100.;
    QTest::newRow("double-length") << 2.f << int(gui::Unit::nm) << 200.;
  }
  void scaleBarUnitsAndAnchor() {
    QFETCH(float, length); QFETCH(int, unit); QFETCH(double, pixels);
    Scene panel(false); panel.getLattice(true)->setVisible(false);
    panel.setDisplayMode(gui::ScreenshotMode);
    auto *manager = panel.findChild<gui::ScreenshotManager*>(); QVERIFY(manager);
    manager->setScaleBarVisibility(true, true);
    manager->setScaleBar(length, static_cast<gui::Unit::DistanceUnit>(unit));
    manager->setScaleBarAnchor({400,800});
    const QRectF region(0,0,3200,2400);
    const auto svg = panel.svg(QString("scale-%1.svg").arg(QTest::currentDataTag()), region);
    const auto geometry = bars(svg); QCOMPARE(geometry.size(), 1);
    const auto bar = geometry.first();
    // Square caps extend half the stroke past each endpoint.
    QVERIFY(qAbs(bar.start.x() - bar.width / 2 - 40) < 0.02);
    QVERIFY(qAbs(bar.start.y() - 80) < 0.02);
    QVERIFY(qAbs(bar.end.y() - 80) < 0.02);
    QVERIFY(qAbs(QLineF(bar.start,bar.end).length() + bar.width - pixels) < 0.02);
    QDomDocument doc; QVERIFY(doc.setContent(svg));
    const QString label = QString::number(length) + " " + gui::Unit::distanceUnitString(static_cast<gui::Unit::DistanceUnit>(unit));
    QVERIFY(doc.documentElement().text().contains(label));
    panel.setTransform(QTransform().rotate(23).scale(0.6,0.6));
    QCOMPARE(panel.svg("scale-transformed.svg", region), svg);
    manager->setScaleBarAnchor({700,1000});
    const auto moved = bars(panel.svg("scale-moved.svg", region)); QCOMPARE(moved.size(), 1);
    QVERIFY(qAbs(moved.first().start.x() - bar.start.x() - 30) < 0.02);
    QVERIFY(qAbs(moved.first().start.y() - bar.start.y() - 20) < 0.02);
  }

  void scaleBarVisibilityAndClipping() {
    Scene panel(false); panel.getLattice(true)->setVisible(false);
    panel.setDisplayMode(gui::ScreenshotMode);
    auto *manager = panel.findChild<gui::ScreenshotManager*>(); QVERIFY(manager);
    manager->setScaleBar(1, gui::Unit::nm); manager->setScaleBarAnchor({400,800});
    manager->setScaleBarVisibility(true, true);
    QCOMPARE(bars(panel.svg("scale-visible.svg", {0,0,3200,2400})).size(), 1);
    manager->setScaleBarVisibility(false, true);
    QVERIFY(bars(panel.svg("scale-hidden.svg", {0,0,3200,2400})).isEmpty());
    manager->setScaleBarVisibility(true, true);
    QVERIFY(bars(panel.svg("scale-excluded.svg", {4000,0,3200,2400})).isEmpty());
    const auto cropped = panel.svg("scale-cropped.svg", {900,0,3200,2400});
    QVERIFY(!cropped.isEmpty());
    QSvgRenderer renderer(cropped); QVERIFY(renderer.isValid());
    QImage image(320,240,QImage::Format_ARGB32); image.fill(Qt::white);
    QPainter painter(&image); renderer.render(&painter); painter.end();
    // Only 0.5 nm of the bar remains inside the export. Avoid its text label.
    for (int x = 0; x < image.width(); ++x) {
      const auto color = image.pixelColor(x,80);
      if (x < 49) QVERIFY2(color.lightness() < 30, qPrintable(QString("missing bar pixel %1").arg(x)));
      if (x > 51) QVERIFY2(color.lightness() > 240, qPrintable(QString("bar escaped crop at %1").arg(x)));
    }
    image.save(test_support::artifact("scale-cropped.png"));
  }
};

SIQAD_TEST_MAIN(DocumentExportTests)
#include "document_export_tests.moc"
