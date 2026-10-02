#pragma once
#include "gui/widgets/design_panel.h"
#include "test_support.h"
#include <QFile>
#include <QSaveFile>
#include <QSvgGenerator>

namespace test_support {
inline QString artifact(const QString &name) {
  const QString root = QString::fromUtf8(SIQAD_TEST_ARTIFACT_DIR);
  QDir().mkpath(root);
  return QDir(root).filePath(name);
}

// Panels own their layers/items. Keep one panel alive per process; the Ghost and
// Emitter are application singletons. Reset layer IDs only after panel teardown.
class Scene : public gui::DesignPanel {
public:
  explicit Scene(bool showWindow = true) {
    resize(640, 480);
    if (showWindow) show();
    else ensurePolished();
    QCoreApplication::processEvents();
  }
  QList<prim::DBDot*> dbs() {
    // The existing getAllDBs accessor returns only top-level DBs. Walk the
    // public item hierarchy independently when checking aggregate contents.
    QStack<prim::Item*> pending;
    for (auto *layer : layerManager()->getLayers(prim::Layer::DB))
      for (auto *item : layer->getItems()) pending.push(item);
    QList<prim::DBDot*> result;
    while (!pending.isEmpty()) {
      auto *item = pending.pop();
      if (item->item_type == prim::Item::DBDot) result.append(static_cast<prim::DBDot*>(item));
      else if (item->item_type == prim::Item::Aggregate)
        for (auto *child : static_cast<prim::Aggregate*>(item)->getChildren()) pending.push(child);
    }
    return result;
  }
  ~Scene() {
    if (QTest::currentTestFailed())
      viewport()->grab().save(artifact(QString("%1-failure.png").arg(QTest::currentTestFunction())));
  }
  bool load(const QString &path, bool result = false) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return false;
    QXmlStreamReader reader(&file);
    if (!reader.readNextStartElement() || reader.name() != QStringLiteral("siqad")) return false;
    loadFromFile(&reader, result);
    return !reader.hasError();
  }
  bool save(const QString &path) {
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return false;
    QXmlStreamWriter writer(&file);
    writer.writeStartDocument(); writer.writeStartElement("siqad");
    writeToXmlStream(&writer, gui::IncludeEntireDesign);
    writer.writeEndElement(); writer.writeEndDocument();
    return !writer.hasError() && file.commit();
  }
  QByteArray svg(const QString &name, const QRectF &region) {
    QSvgGenerator generator;
    const auto path = artifact(name);
    generator.setFileName(path);
    generator.setSize(QSize(320, 240)); generator.setViewBox(QRect(0, 0, 320, 240));
    QPainter painter;
    if (!painter.begin(&generator)) return {};
    screenshot(&painter, region, QRectF(0, 0, 320, 240));
    painter.end();
    QFile file(path); if (!file.open(QIODevice::ReadOnly)) return {};
    return file.readAll();
  }
};

inline void resetCase() {
  QApplication::clipboard()->clear();
  prim::Layer::resetLayers();
  settings::AppSettings::instance()->clear();
  settings::GUISettings::instance()->clear();
  settings::LatticeSettings::updateLattice();
  settings::LatticeSettings::instance()->clear();
}
inline QString fixture(const QString &name) {
  return QDir(QString::fromUtf8(SIQAD_TEST_FIXTURE_DIR)).filePath(name);
}
}
