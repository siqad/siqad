#include "scene_fixture.h"
#include "gui/clipboard_codec.h"
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QTimer>

namespace {
QJsonObject readJson(const QString &path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) return {};
  return QJsonDocument::fromJson(file.readAll()).object();
}
bool writeJson(const QString &path, const QJsonObject &object) {
  QSaveFile file(path);
  const auto bytes = QJsonDocument(object).toJson(QJsonDocument::Compact);
  return file.open(QIODevice::WriteOnly)
      && file.write(bytes) == bytes.size() && file.commit();
}

int actorMain(const QString &mailbox) {
  test_support::Scene panel;
  QTimer timer;
  int previousId = 0;
  QPointF lastPlacement;
  QObject::connect(&timer, &QTimer::timeout, [&] {
    const auto command = readJson(mailbox + "/request.json");
    const int id = command["id"].toInt();
    if (id <= previousId) return;
    previousId = id;
    const auto op = command["op"].toString();
    bool ok = true;
    int imported = -1;
    if (op == "load") ok = panel.load(command["path"].toString());
    else if (op == "copy" || op == "mixedCopy") {
      if (op == "mixedCopy") {
        auto *metal = panel.layerManager()->getLayers(prim::Layer::Electrode).first();
        auto *electrode = new prim::Electrode(metal->layerID(), QRectF(-1000, -1000, 100, 100));
        panel.addItem(electrode, panel.layerManager()->indexOf(metal));
        electrode->setSelected(true);
      }
      for (auto *layer : panel.layerManager()->getLayers(prim::Layer::DB))
        for (auto *item : layer->getItems()) item->setSelected(true);
      ok = QMetaObject::invokeMethod(&panel, "copyAction", Qt::DirectConnection);
    } else if (op == "extraLayer") {
      auto *layer = panel.layerManager()->addDBLayer(panel.getLattice(true), "Clipboard target");
      panel.layerManager()->setActiveLayer(layer);
    } else if (op == "paste" || op == "pasteAgain") {
      panel.activateWindow();
      QCoreApplication::processEvents();
      ok = QMetaObject::invokeMethod(&panel, "pasteAction", Qt::DirectConnection);
      imported = prim::Ghost::instance()->getSources().size();
      auto *ghost = prim::Ghost::instance();
      if (op == "paste" && ghost->snapAnchor()) {
        // Select a destination with the source anchor's basis index. Moving a
        // mixed-basis selection onto the other basis is intentionally invalid.
        int basis = 0;
        for (auto *item : ghost->getSources()) {
          auto *db = static_cast<prim::DBDot*>(item);
          if (ghost->getLatticeCoord(db) == ghost->snapAnchor()->latticeCoord()) basis = db->latticeCoord().l;
        }
        lastPlacement = panel.getLattice(true)->latticeCoord2ScenePos(prim::LatticeCoord(10, 10, basis))
            - ghost->freeAnchor(QPointF());
        panel.updateSceneRect(QRectF(lastPlacement - QPointF(4000, 4000), QSizeF(8000, 8000)));
        panel.centerOn(lastPlacement);
      }
      const auto position = panel.mapFromScene(lastPlacement);
      QMouseEvent move(QEvent::MouseMove, position, panel.viewport()->mapToGlobal(position),
          Qt::NoButton, Qt::NoButton, Qt::NoModifier);
      QApplication::sendEvent(panel.viewport(), &move);
      QMouseEvent press(QEvent::MouseButtonPress, position, panel.viewport()->mapToGlobal(position),
          Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
      QMouseEvent release(QEvent::MouseButtonRelease, position, panel.viewport()->mapToGlobal(position),
          Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
      QApplication::sendEvent(panel.viewport(), &press);
      QApplication::sendEvent(panel.viewport(), &release);
    } else if (op == "undo" || op == "redo") {
      ok = QMetaObject::invokeMethod(&panel, op == "undo" ? "undoAction" : "redoAction", Qt::DirectConnection);
    } else if (op == "save") ok = panel.save(command["path"].toString());
    else if (op == "quit") QTimer::singleShot(0, qApp, &QCoreApplication::quit);
    else ok = false;
    QJsonArray coordinates, layers;
    for (auto *db : panel.dbs()) {
      const auto c = db->latticeCoord();
      coordinates.append(QJsonArray{c.n, c.m, c.l}); layers.append(db->layer_id);
    }
    const auto *mime = QApplication::clipboard()->mimeData();
    writeJson(mailbox + "/response.json", {{"id", id}, {"ok", ok},
        {"coordinates", coordinates}, {"layers", layers},
        {"activeLayer", panel.layerManager()->activeLayer()->layerID()},
        {"imported", imported},
        {"clipboardBytes", mime ? mime->data(gui::clipboard_codec::mimeType).size() : 0},
        {"siqadMime", mime && mime->hasFormat(gui::clipboard_codec::mimeType)}});
  });
  writeJson(mailbox + "/response.json", {{"id", 0}, {"ready", true}});
  timer.start(10);
  return qApp->exec();
}

class Actor {
public:
  explicit Actor(const QString &name) {
    transcript = test_support::artifact(name + "-commands.jsonl");
    QFile::remove(transcript);
    process.setProgram(QCoreApplication::applicationFilePath());
    process.setArguments({"--actor", mailbox.path()});
    process.setStandardErrorFile(test_support::artifact(name + ".log"));
  }
  bool start() {
    if (!mailbox.isValid()) return false;
    process.start();
    if (!process.waitForStarted(5000)) return false;
    QElapsedTimer time; time.start();
    while (time.elapsed() < 5000 && process.state() != QProcess::NotRunning) {
      if (readJson(mailbox.path() + "/response.json")["ready"].toBool()) return true;
      QTest::qWait(10);
    }
    return false;
  }
  QJsonObject command(const QString &op, const QString &path = {}) {
    const QJsonObject request{{"id", ++id}, {"op", op}, {"path", path}};
    QFile requestLog(transcript);
    if (requestLog.open(QIODevice::WriteOnly | QIODevice::Append))
      requestLog.write(QJsonDocument(QJsonObject{{"request", request}}).toJson(QJsonDocument::Compact) + "\n");
    if (!writeJson(mailbox.path() + "/request.json", request)) return {};
    QElapsedTimer time; time.start();
    while (time.elapsed() < 5000) {
      const auto result = readJson(mailbox.path() + "/response.json");
      if (result["id"].toInt() == id) {
        QFile file(transcript);
        if (file.open(QIODevice::WriteOnly | QIODevice::Append))
          file.write(QJsonDocument(QJsonObject{{"op", op}, {"result", result}}).toJson(QJsonDocument::Compact) + "\n");
        return result;
      }
      if (process.state() == QProcess::NotRunning) break;
      QTest::qWait(10);
    }
    return {};
  }
  bool stop() {
    if (process.state() == QProcess::NotRunning) return true;
    const bool replied = command("quit")["ok"].toBool();
    if (process.waitForFinished(1000)) return replied && process.exitCode() == 0;
    process.kill(); process.waitForFinished(1000); return false;
  }
  ~Actor() { stop(); }
private:
  QProcess process;
  QString transcript;
  QTemporaryDir mailbox;
  int id = 0;
};
}

class NativeClipboardTests : public QObject {
  Q_OBJECT
private slots:
  void initTestCase() {
    // Enabled native jobs must fail rather than silently pass using an in-process clipboard.
    QVERIFY(QGuiApplication::platformName() != "offscreen" && QGuiApplication::platformName() != "minimal");
  }
  void nestedTransferLayerRoutingCollisionAndUndo() {
    Actor source("clipboard-source"), target("clipboard-target");
    QVERIFY(source.start()); QVERIFY(target.start());
    QVERIFY(source.command("load", test_support::fixture("nested-sidbs.sqd"))["ok"].toBool());
    QVERIFY(source.command("copy")["siqadMime"].toBool());
    QVERIFY(target.command("extraLayer")["ok"].toBool());
    const auto pasted = target.command("paste");
    QVERIFY(pasted["ok"].toBool());
    const auto coords = pasted["coordinates"].toArray();
    QCOMPARE(coords.size(), 3);
    for (const auto &layer : pasted["layers"].toArray()) QCOMPARE(layer.toInt(), pasted["activeLayer"].toInt());
    // Check one common translation preserves all source lattice offsets.
    const QList<QList<int>> original{{0,0,0}, {3,0,1}, {4,2,0}};
    bool translated = false;
    for (const auto &anchor : coords) {
      const auto a = anchor.toArray();
      int matched = 0;
      for (const auto &c : original)
        for (const auto &value : coords)
          if (value.toArray() == QJsonArray{c[0]+a[0].toInt(), c[1]+a[1].toInt(), c[2]}) ++matched;
      translated |= matched == 3;
    }
    QVERIFY(translated);
    QCOMPARE(target.command("pasteAgain")["coordinates"].toArray(), coords);
    QCOMPARE(target.command("undo")["coordinates"].toArray().size(), 0);
    QCOMPARE(target.command("redo")["coordinates"].toArray(), coords);
    const auto path = test_support::artifact("cross-process-pasted.sqd");
    QVERIFY(target.command("save", path)["ok"].toBool());
    QFile file(path); QVERIFY(file.open(QIODevice::ReadOnly));
    const auto saved = file.readAll();
    QCOMPARE(saved.count("<aggregate>"), 2);
    QVERIFY(target.command("load", path)["ok"].toBool());
    QCOMPARE(target.command("save", path)["coordinates"].toArray(), coords);
    QVERIFY(source.stop()); QVERIFY(target.stop());
  }
  void sourceExitAndUnsupportedMixedSelection() {
    Actor source("clipboard-exit-source"), target("clipboard-exit-target");
    QVERIFY(source.start()); QVERIFY(target.start());
    QVERIFY(source.command("load", test_support::fixture("nested-sidbs.sqd"))["ok"].toBool());
    QVERIFY(!source.command("mixedCopy")["siqadMime"].toBool());
    QCOMPARE(target.command("paste")["coordinates"].toArray().size(), 0);
    // Reload removes the unsupported selection before copying again.
    QVERIFY(source.command("load", test_support::fixture("nested-sidbs.sqd"))["ok"].toBool());
    QVERIFY(source.command("copy")["siqadMime"].toBool());
    QVERIFY(source.stop());
    const auto result = target.command("paste");
    QVERIFY(result["ok"].toBool());
    // X11 without a clipboard manager loses ownership on process exit. Cocoa
    // and Windows retain the eagerly published data. State that contract explicitly.
    const auto *mime = QApplication::clipboard()->mimeData();
    const bool retained = mime && mime->hasFormat(gui::clipboard_codec::mimeType);
#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
    QVERIFY2(retained, "The native clipboard must retain published data after source exit");
#endif
    QCOMPARE(result["coordinates"].toArray().size(), retained ? 3 : 0);
    QVERIFY(target.stop());
  }
};

int main(int argc, char **argv) {
  test_support::Profile profile;
  QApplication app(argc, argv);
  if (argc == 3 && QString::fromLocal8Bit(argv[1]) == "--actor") return actorMain(QString::fromLocal8Bit(argv[2]));
  test_support::ClipboardGuard clipboard;
  NativeClipboardTests tests;
  return QTest::qExec(&tests, argc, argv);
}
#include "native_clipboard_tests.moc"
