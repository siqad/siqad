#pragma once
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QtTest>
#include <memory>

namespace test_support {
class Profile {
public:
  Profile() {
    if (!dir.isValid()) qFatal("Cannot create isolated test profile");
    qputenv("SIQAD_PROFILE_ROOT", dir.path().toUtf8());
    QStandardPaths::setTestModeEnabled(true);
  }
private:
  QTemporaryDir dir;
};

// Native suites must restore every advertised MIME format, including custom data.
class ClipboardGuard {
public:
  ClipboardGuard() : saved(new QMimeData) {
    const auto *data = QApplication::clipboard()->mimeData();
    if (data)
      for (const auto &format : data->formats()) saved->setData(format, data->data(format));
  }
  ~ClipboardGuard() { QApplication::clipboard()->setMimeData(saved.release()); }
private:
  std::unique_ptr<QMimeData> saved;
};
}

#define SIQAD_TEST_MAIN(TestClass) \
int main(int argc, char **argv) { \
  test_support::Profile profile; \
  QApplication app(argc, argv); \
  test_support::ClipboardGuard clipboard; \
  TestClass tests; \
  return QTest::qExec(&tests, argc, argv); \
}
