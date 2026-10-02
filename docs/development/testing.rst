Testing SiQAD
*************

Build and run
=============

Use CMake 3.20+, a C++17 compiler, Qt 6 including Test, Python 3, and Bash
for build-script tests. The GUI needs Core, Gui, Widgets, Svg, PrintSupport,
UiTools, Charts and Xml. Simulator dependencies/tests belong to their upstream
repositories. Deterministic tests need neither a display nor external engines::

    cmake -S . -B build-tests -DCMAKE_BUILD_TYPE=Debug -DSIQAD_BUILD_PLUGINS=OFF
    cmake --build build-tests --parallel 4
    ctest --test-dir build-tests --output-on-failure --no-tests=error

Building never runs tests implicitly. CTest reruns tests after an unchanged
build, discovers them from the root (also with standalone ``src`` builds),
and fails empty selections with ``--no-tests=error``. Suites have timeouts.

``BUILD_TESTING`` defaults to ON. OFF omits tests and the QtTest/Python test
dependencies. Deprecated ``BUILD_TEST`` and ``SKIP_SIQAD_TESTS`` are accepted
on the first configure when the canonical option is absent; a true SKIP option
overrides BUILD_TEST. Thereafter the cached canonical value controls the build.
Migrate automation to ``-DBUILD_TESTING=ON`` or ``OFF``.

``make_everything_dev`` explicitly runs CTest before installation. ``notest``
disables tests for that invocation; omitting it re-enables them. Supported legacy
flags normalize to the canonical option. Arguments after ``--`` preserve spaces.
``CMAKE_COMMAND`` and ``CTEST_COMMAND`` select matching tool binaries::

    JOBS=4 ./make_everything_dev debug -- -DSIQAD_BUILD_PLUGINS=OFF
    ./make_everything_dev release notest -- -DCMAKE_INSTALL_PREFIX="/tmp/SiQAD release"

Suites and ownership
====================

* ``unit``: QtCore clipboard codec, build-script contracts, real CMake test-option
  policy, and report contracts. Includes generated malformed coordinates,
  root/version validation, duplicates and exact item/depth limits.
* ``gui``: offscreen Qt events, document round trips, active result layers,
  charge overlays, SVG structure, failed-output restoration, custom lattices,
  wheel modifiers, zoom bounds and loading suppression. macOS pinch cases
  explicitly skip on other platforms.
* ``integration``: document, result and clipboard workflows.
* ``native``: opt-in independent processes using the system clipboard.
* ``build-script``: portable arguments and error contracts; no real installs.

Select labels with ``ctest --test-dir build-tests -L unit --output-on-failure
--no-tests=error``. Checked-in XML/SQD fixtures are deterministic reference data,
not evidence of a live simulator run. Result tests use real parsers, result
lattice loading and the charge visualizer without launching an engine.

``test_support.h`` sets ``SIQAD_PROFILE_ROOT`` before QApplication construction.
Lazy settings defaults keep preferences, autosaves and plugin state in a unique
temporary profile. Set this variable before any settings object in new actors.
``Scene`` owns its panel layers/items. Keep one panel alive per process because
Ghost and Emitter are shared singletons; destroy it before resetting layer IDs
and settings. Select controls by object name, not translated text.

The QtCore codec lives in ``src/gui/clipboard_codec.*``. Version 1 supports
nonempty DB/aggregate trees, finite positions, signed integer coordinates and
nonnegative layers. It rejects duplicate sites and invalid target basis indices.
Limits are 1 MiB serialized, 10,000 total nodes and 64 levels (top-level items
are level 1). Invalid decode leaves output intact; invalid encode returns empty.
Source layers are metadata; placement uses the target's active DB layer.

Native clipboard tests
======================

Enable explicitly on a disposable desktop/display::

    cmake -S . -B build-native -DCMAKE_BUILD_TYPE=Debug -DSIQAD_BUILD_PLUGINS=OFF -DSIQAD_NATIVE_TESTS=ON
    cmake --build build-native --parallel 4
    ctest --test-dir build-native -L native --output-on-failure --no-tests=error

On Linux install Xvfb/xauth and prefix CTest with ``xvfb-run -a``. Tests reject
offscreen/minimal backends, run serially, use independent profiles and atomic
request/response files, and bound startup/commands/shutdown. Children are killed
and reaped on timeout. Qt mouse events drive placement; the clipboard crosses
the OS boundary. This does not qualify physical input or accessibility behavior.

The parent restores advertised clipboard MIME formats on normal teardown; a
crash cannot guarantee restoration, so use an isolated desktop. macOS/Windows
must retain published data after source exit. X11 without a clipboard manager
loses selection ownership on exit; tests accept that behavior and reject stale
imports. Coverage includes nested aggregates, translation, differing layers,
occupied destinations, mixed selections, save/reload and undo/redo.
On X11, clipboard restoration cannot survive parent exit without a clipboard
manager either; the CI job uses a disposable Xvfb display.

Evidence and CI
===============

C++/Python suites emit JUnit XML to ``<build>/test-results`` and retain text
output. Scene failures save viewport PNGs; exported SVGs, saved documents and
native actor logs/transcripts remain there. CTest logs are in
``Testing/Temporary``. Summarize expected evidence with::

    python3 src/tests/report_tests.py build-tests/test-results --expect siqad_tests gui_workflow_tests clipboard_codec_tests developer_script_tests report_tests_tests

The reporter fails missing, empty, malformed or failed evidence and lists skip
reasons separately. CI uploads evidence even on failure. Existing platform jobs
run explicit Debug/Release tests. The focused workflow adds Xvfb clipboard tests,
sanitizers and real developer builds on Linux/macOS with Ninja and Unix Makefiles.
Run the real build smoke locally::

    python3 src/tests/developer_build_smoke.py --generator "Unix Makefiles" --artifacts smoke-results

It builds actual GUI sources in a temporary wrapper checkout, installs below
that directory, and retains logs/reports at the requested path. It exercises
Release/notest to Debug/tests, spaced prefixes, invocation outside the checkout,
unchanged rebuilds and stale SDK/install-root recovery.
SDK cache injection is macOS-only; Linux retains its own dependency discovery.

Sanitizers and remaining coverage
================================

For Clang/GCC configure a separate GUI-only build with ``SIQAD_SANITIZERS=ON``.
CI enables leak detection for the pure unit suite. GUI tests use
``ASAN_OPTIONS=detect_leaks=0`` because existing application/Qt singletons retain
process-lifetime allocations; address errors and undefined behavior remain
fatal. Full GUI leak qualification requires singleton ownership cleanup.

Native Windows/macOS CI jobs and physical input automation remain additional
coverage. The Xvfb clipboard-manager lifetime variant, full simulation job
discovery/external engines, historical design migration, scale-bar geometry,
and portable touch recognizer cancellation also need dedicated environments or
fixtures. Prefer observable behavior checks over implementation mirrors or
font-dependent pixel goldens.
