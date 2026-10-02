#include <QtTest>
#include "gui/clipboard_codec.h"
#include "test_data.h"
using namespace gui::clipboard_codec;
using namespace test_data;

class ClipboardCodecTests : public QObject {
  Q_OBJECT
private slots:
  void roundTrip() {
    const QJsonArray original{aggregate({db(-3, -2, 0), aggregate({db(1, 4, 1, 99)})})};
    QJsonArray decoded;
    QVERIFY(decode(encode(original), 2, decoded));
    QCOMPARE(decoded, original);
  }
  void rejectsRoot_data() {
    QTest::addColumn<QByteArray>("input");
    QTest::newRow("syntax") << QByteArray("{broken");
    QTest::newRow("array-root") << QByteArray("[]");
    QTest::newRow("fractional-version") << payload({db()}, 1.5);
    QTest::newRow("future-version") << payload({db()}, 2);
    QTest::newRow("missing-version") << QByteArray("{\"format\":\"siqad-sidb-selection\",\"items\":[]}");
    QTest::newRow("empty") << payload({});
    QTest::newRow("oversized") << QByteArray(maxBytes + 1, ' ');
    QTest::newRow("wrong-tag") << QByteArray("{\"format\":\"other\",\"version\":1,\"items\":[]}");
  }
  void rejectsRoot() {
    QFETCH(QByteArray, input);
    QJsonArray output{db(9)};
    QString error;
    QVERIFY(!decode(input, 2, output, &error));
    QCOMPARE(output, QJsonArray{db(9)});
    QVERIFY(!error.isEmpty());
  }
  void boundedTrees() {
    QJsonArray items{db()};
    for (int i = 1; i < maxDepth; ++i) items = QJsonArray{aggregate(items)};
    QJsonArray decoded;
    QVERIFY(decode(payload(items), 2, decoded));
    QVERIFY(!decode(payload({aggregate(items)}), 2, decoded));
    QJsonArray many;
    for (int i = 0; i < maxItems; ++i) many.append(db(i));
    QVERIFY(decode(payload(many), 2, decoded));
    many.append(db(maxItems));
    QVERIFY(!decode(payload(many), 2, decoded));
  }
  void generatedMalformedCoordinates() {
    // Fixed seed and deterministic corpus: retain reproducibility without a fuzz runtime.
    for (int i = 0; i < 200; ++i) {
      auto item = db(i - 100, i % 7, i % 2);
      QJsonArray decoded;
      QVERIFY(decode(payload({item}), 2, decoded));
      auto lat = item["lat"].toObject();
      lat["n"] = i + 0.25;
      item["lat"] = lat;
      QVERIFY(!decode(payload({item}), 2, decoded));
      QVERIFY(!decode(payload({db(i), aggregate({db(i)})}), 2, decoded));
    }
  }
  void basisAndTypes() {
    QJsonArray decoded;
    QVERIFY(!decode(payload({db(0, 0, 2)}), 2, decoded));
    QVERIFY(decode(payload({db(0, 0, 2)}), 3, decoded));
    auto bad = db(); bad["type"] = "electrode";
    QVERIFY(!decode(payload({bad}), 2, decoded));
    bad = db(); bad["pos"] = QJsonObject{{"x", "NaN"}, {"y", 0}};
    QVERIFY(!decode(payload({bad}), 2, decoded));
  }
};
QTEST_GUILESS_MAIN(ClipboardCodecTests)
#include "clipboard_codec_tests.moc"
