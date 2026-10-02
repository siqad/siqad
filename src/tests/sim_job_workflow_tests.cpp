#include "scene_fixture.h"
#include "gui/widgets/managers/job_manager.h"
#include "gui/widgets/managers/plugin_manager.h"
#include "gui/widgets/visualizers/sim_visualizer.h"
#include <QDomDocument>

using test_support::Scene;
using comp::JobResult;
using comp::SimJob;

// All engine discovery, profiles, problems and outputs live under a unique root.
class Workflow {
public:
  Scene panel{false};
  QTemporaryDir dir{QDir(qEnvironmentVariable("SIQAD_PROFILE_ROOT")).filePath("job workflow XXXXXX")};
  std::unique_ptr<gui::PluginManager> plugins;
  gui::SimVisualizer visualizer{&panel};
  std::unique_ptr<gui::JobManager> manager;
  QString loadError;
  int exported = 0;
  QList<SimJob*> jobs; // Observations only: JobManager owns these pointers.

  bool init() {
    if (!dir.isValid() || !panel.load(test_support::fixture("nested-sidbs.sqd"))) return false;
    const QString engineDir = QDir(dir.path()).filePath("engines/fake engine");
    if (!QDir().mkpath(engineDir)) return false;
    QFile desc(QDir(engineDir).filePath("fake.sqplug"));
    if (!desc.open(QIODevice::WriteOnly)) return false;
    QXmlStreamWriter writer(&desc);
    writer.writeStartDocument(); writer.writeStartElement("sqplug");
    writer.writeTextElement("name", "Deterministic test engine");
    writer.writeTextElement("version", "1");
    writer.writeTextElement("services", "Custom");
    writer.writeTextElement("bin_path", QString::fromUtf8(SIQAD_FAKE_SIMULATOR));
    writer.writeStartElement("commands");
    for (const auto &mode : {"success", "failure", "missing", "malformed", "wait", "unstartable"}) {
      writer.writeStartElement("command"); writer.writeAttribute("label", mode);
      writer.writeTextElement("program", QString(mode) == "unstartable"
          ? QDir(dir.path()).filePath("no such executable") : "@BINPATH@");
      for (const auto &arg : QStringList{mode, "@PROBLEMPATH@", "@RESULTPATH@",
                                       test_support::fixture("simulation-results.xml")})
        writer.writeTextElement("arg", arg);
      writer.writeEndElement();
    }
    writer.writeEndElement(); writer.writeEndElement(); writer.writeEndDocument();
    desc.close();
    if (writer.hasError()) return false;
    auto *settings = settings::AppSettings::instance();
    settings->setValue("plugs/eng_lib_dirs", QStringList{QDir(dir.path()).filePath("engines")});
    settings->setValue("plugs/runtime_tmp_root_path", QDir(dir.path()).filePath("job outputs"));
    // This compiled plugin never invokes Python or requests a virtualenv.
    gui::python_path = "unused-by-compiled-test-engine";
    plugins = std::make_unique<gui::PluginManager>();
    if (plugins->count() != 1) return false;
    manager = std::make_unique<gui::JobManager>(plugins.get(), &visualizer);
    QObject::connect(manager.get(), &gui::JobManager::sig_exportJobProblem, &panel,
        [this](comp::JobStep *step, gui::DesignInclusionArea) {
          if (!panel.save(step->problemPath())) loadError = "Cannot export problem";
          ++exported;
        });
    // Same result-layer loading boundary used by ApplicationGUI.
    QObject::connect(&visualizer, &gui::SimVisualizer::sig_loadProblemFile, &panel,
        [this](const QString &path) {
          panel.clearSimResults();
          if (!panel.load(path, true)) loadError = "Cannot load job problem";
          panel.enableSimVis();
          panel.setDisplayMode(gui::SimDisplayMode);
        });
    return true;
  }

  SimJob *job(const QStringList &modes) {
    auto *job = new SimJob("test job");
    auto *engine = plugins->pluginEngines().first();
    for (const auto &mode : modes) {
      for (const auto &command : engine->commandFormats()) {
        if (command.first == mode)
          job->addJobStep(new comp::JobStep(engine, command.second, engine->defaultPropertyMap()));
      }
    }
    // Register immediately: the manager owns job/steps even on assertion failure.
    manager->addJob(job);
    jobs.append(job);
    return job;
  }

  ~Workflow() {
    // Preserve subprocess evidence before deleting jobs or their temporary root.
    if (manager) {
      for (auto *job : jobs)
        for (auto *step : job->jobSteps()) {
          const QDir root(step->jobStepTempDirPath());
          step->exportTerminalOutputs(root.filePath("runtime_stdout.log"), root.filePath("runtime_stderr.log"));
        }
      QDirIterator files(dir.path(), QDir::Files, QDirIterator::Subdirectories);
      const QString target = test_support::artifact(QString("jobs-%1-%2").arg(QTest::currentTestFunction(), QTest::currentDataTag()));
      while (files.hasNext()) {
        const auto source = files.next();
        const auto dest = QDir(target).filePath(QDir(dir.path()).relativeFilePath(source));
        QDir().mkpath(QFileInfo(dest).absolutePath());
        QFile::remove(dest); QFile::copy(source, dest);
      }
    }
    visualizer.clearJob();
  }
};

static QString manifestState(SimJob *job, int step = -1) {
  QFile file(QDir(job->runtimeTempPath()).filePath("manifest.xml"));
  if (!file.open(QIODevice::ReadOnly)) return {};
  QDomDocument doc;
  if (!doc.setContent(file.readAll())) return {};
  const auto node = step < 0 ? doc.documentElement()
      : doc.elementsByTagName("job_step").at(step).toElement();
  return node.firstChildElement("state").text();
}

class SimJobWorkflowTests : public QObject {
  Q_OBJECT
private slots:
  void cleanup() { test_support::resetCase(); gui::python_path.clear(); }

  void successDiscoveryVisualizationAndClear() {
    Workflow flow; QVERIFY(flow.init());
    auto *job = flow.job({"success", "success"});
    QSignalSpy finished(job, &SimJob::sig_jobFinishState);
    QSignalSpy shown(&flow.visualizer, &gui::SimVisualizer::sig_showJobInvoked);
    QSignalSpy loaded(&flow.visualizer, &gui::SimVisualizer::sig_loadProblemFile);
    flow.panel.setDisplayMode(gui::ScreenshotMode);
    auto *screenshots = flow.panel.findChild<gui::ScreenshotManager*>();
    screenshots->setScaleBarVisibility(false, true);
    flow.panel.getLattice(true)->setVisible(false);
    const QRectF region(-500, -500, 4000, 4000);
    const auto before = flow.panel.svg("job-before.svg", region);
    QVERIFY(!before.isEmpty());
    flow.manager->runJob(job);
    QTRY_COMPARE_WITH_TIMEOUT(finished.count(), 1, 10000);
    QCOMPARE(job->jobState(), SimJob::FinishedNormally);
    QCOMPARE(flow.exported, 2);
    QVERIFY2(flow.loadError.isEmpty(), qPrintable(flow.loadError));
    QCOMPARE(shown.count(), 1);
    QCOMPARE(job->resultTypeStepMap().count(JobResult::DBLocationsResult), 2);
    QCOMPARE(job->resultTypeStepMap().count(JobResult::ChargeConfigsResult), 2);
    for (auto *step : job->jobSteps()) {
      auto *charges = qobject_cast<comp::ChargeConfigSet*>(step->jobResults().value(JobResult::ChargeConfigsResult));
      QVERIFY(charges);
      QCOMPARE(charges->totalConfigCount(), 5);
      QCOMPARE(charges->dbPhysicalLocations().size(), 3);
      QVERIFY(step->terminalOutput(QProcess::StandardOutput).contains("FAKE_RESULT success"));
      QCOMPARE(manifestState(job, step->jobStepPlacement()), QString("FinishedNormally"));
    }
    auto *selector = flow.visualizer.findChild<QComboBox*>("chargeConfigJobStep");
    QVERIFY(selector); QCOMPARE(selector->count(), 2);
    for (const QString &stepNumber : {QString("0"), QString("1")}) {
      selector->setCurrentText(stepNumber);
      QCOMPARE(loaded.last().first().toString(), job->getJobStep(stepNumber.toInt())->problemPath());
    }
    QCOMPARE(flow.panel.getLattice(false)->dbsAtPhysLocs({{0,0},{11.52,2.25},{15.36,15.36}}).size(), 3);
    flow.panel.setDisplayMode(gui::ScreenshotMode);
    flow.panel.getLattice(false)->setVisible(false);
    const auto charged = flow.panel.svg("job-visualized.svg", region);
    QVERIFY(!charged.isEmpty()); QVERIFY(charged != before);
    flow.visualizer.clearJob();
    flow.panel.clearSimResults();
    flow.panel.setDisplayMode(gui::ScreenshotMode);
    flow.panel.getLattice(true)->setVisible(false);
    QCOMPARE(flow.panel.svg("job-cleared.svg", region), before);
    QCOMPARE(flow.panel.dbs().size(), 3);
    QVERIFY(!flow.visualizer.isEnabled());
    QCOMPARE(manifestState(job), QString("FinishedNormally"));
    QVERIFY(!job->guiControlElems().pb_terminate->isEnabled());
  }

  void failedResultsStopQueuedSteps_data() {
    QTest::addColumn<QString>("mode");
    for (const auto &mode : {"failure", "malformed", "missing", "unstartable"})
      QTest::newRow(mode) << QString(mode);
  }
  void failedResultsStopQueuedSteps() {
    QFETCH(QString, mode);
    Workflow flow; QVERIFY(flow.init());
    auto *job = flow.job({mode, "success"});
    QSignalSpy finished(job, &SimJob::sig_jobFinishState);
    QSignalSpy shown(&flow.visualizer, &gui::SimVisualizer::sig_showJobInvoked);
    flow.manager->runJob(job);
    QTRY_COMPARE_WITH_TIMEOUT(finished.count(), 1, 10000);
    QCOMPARE(job->jobState(), SimJob::FinishedWithError);
    QVERIFY(job->resultTypeStepMap().isEmpty());
    QVERIFY(job->getJobStep(0)->jobResults().isEmpty());
    QVERIFY(!QFileInfo::exists(job->getJobStep(1)->resultPath()));
    QVERIFY(job->getJobStep(1)->terminalOutput(QProcess::StandardOutput).isEmpty());
    QCOMPARE(manifestState(job, 0), QString("FinishedWithError"));
    QCOMPARE(manifestState(job, 1), QString("NotInvoked"));
    QCOMPARE(manifestState(job), QString("FinishedWithError"));
    QCOMPARE(shown.count(), 0);
    QCOMPARE(flow.panel.dbs().size(), 3);
    if (mode == "failure")
      QVERIFY(job->getJobStep(0)->terminalOutput(QProcess::StandardOutput).contains("intentional engine failure"));
  }

  void laterStepStartupFailureFinishesJob() {
    Workflow flow; QVERIFY(flow.init());
    auto *job = flow.job({"success", "unstartable", "success"});
    QSignalSpy finished(job, &SimJob::sig_jobFinishState);
    flow.manager->runJob(job);
    QTRY_COMPARE_WITH_TIMEOUT(finished.count(), 1, 10000);
    QCOMPARE(job->jobState(), SimJob::FinishedWithError);
    QCOMPARE(manifestState(job, 0), QString("FinishedNormally"));
    QCOMPARE(manifestState(job, 1), QString("FinishedWithError"));
    QCOMPARE(manifestState(job, 2), QString("NotInvoked"));
    QVERIFY(!QFileInfo::exists(job->getJobStep(2)->resultPath()));
  }

  void cancellationStopsQueuedSteps() {
    Workflow flow; QVERIFY(flow.init());
    auto *job = flow.job({"wait", "success"});
    QSignalSpy finished(job, &SimJob::sig_jobFinishState);
    flow.manager->runJob(job);
    QTRY_VERIFY_WITH_TIMEOUT(job->getJobStep(0)->terminalOutput(QProcess::StandardOutput).contains("FAKE_READY"), 10000);
    job->terminateJob();
    QTRY_COMPARE_WITH_TIMEOUT(finished.count(), 1, 10000);
    QCOMPARE(job->jobState(), SimJob::FinishedWithError);
    QCOMPARE(manifestState(job), QString("FinishedWithError"));
    QCOMPARE(manifestState(job, 0), QString("FinishedWithError"));
#ifndef Q_OS_WIN
    QVERIFY(job->getJobStep(0)->terminalOutput(QProcess::StandardOutput).contains("COOPERATIVE_EXIT"));
#endif
    QCOMPARE(manifestState(job, 1), QString("NotInvoked"));
    QVERIFY(!QFileInfo::exists(job->getJobStep(1)->resultPath()));
    QVERIFY(job->resultTypeStepMap().isEmpty());
    QVERIFY(!job->guiControlElems().pb_terminate->isEnabled());
  }
};

SIQAD_TEST_MAIN(SimJobWorkflowTests)
#include "sim_job_workflow_tests.moc"
