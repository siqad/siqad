// A deterministic subprocess exercising the real plugin/job protocol, not physics.
#include <QCoreApplication>
#include <QFile>
#include <QSaveFile>
#include <QTimer>
#include <QXmlStreamReader>
#include <iostream>
#include <csignal>

#ifndef Q_OS_WIN
static volatile std::sig_atomic_t stopRequested = 0;
static void requestStop(int) { stopRequested = 1; }
#endif

int main(int argc, char **argv) {
  QCoreApplication app(argc, argv);
  const auto args = app.arguments();
  if (args.size() != 5) return 64;
  const QString mode = args[1];
  QFile problem(args[2]);
  if (!problem.open(QIODevice::ReadOnly)) return 65;
  QXmlStreamReader reader(&problem);
  if (!reader.readNextStartElement() || reader.name() != QStringLiteral("siqad")) return 66;
  int dots = 0;
  while (!reader.atEnd()) {
    reader.readNext();
    if (reader.isStartElement() && reader.name() == QStringLiteral("dbdot")) ++dots;
  }
  if (reader.hasError() || dots != 3) return 67;
  if (mode == "failure") {
    std::cerr << "intentional engine failure" << std::endl;
    return 23;
  }
  if (mode == "missing") return 0;
  QFile fixture(args[4]);
  if (!fixture.open(QIODevice::ReadOnly)) return 69;
  QByteArray result = fixture.readAll();
  if (mode == "malformed") result.truncate(result.indexOf("</sim_out>"));
  else if (mode != "success" && mode != "wait") return 70;
  QSaveFile output(args[3]);
  if (!output.open(QIODevice::WriteOnly) || output.write(result) != result.size()
      || !output.commit()) return 71;
  if (mode == "wait") {
#ifndef Q_OS_WIN
    std::signal(SIGTERM, requestStop);
    QTimer stopTimer;
    QObject::connect(&stopTimer, &QTimer::timeout, &app, [] {
      if (stopRequested) {
        std::cout << "COOPERATIVE_EXIT" << std::endl;
        QCoreApplication::exit(0); // Cancellation must still stop queued steps.
      }
    });
    stopTimer.start(10);
#endif
    std::cout << "FAKE_READY dbdots=" << dots << std::endl;
    // Safety bound if a test fails before it can terminate the child.
    QTimer::singleShot(30000, &app, [] { QCoreApplication::exit(68); });
    return app.exec();
  }
  std::cout << "FAKE_RESULT " << mode.toStdString() << std::endl;
  return 0;
}
