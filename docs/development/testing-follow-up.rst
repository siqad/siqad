Testing follow-up from cross-instance-copy
*****************************************

This note records tests wanted during the ``cross-instance-copy`` review and
validation on 2026-10-01. The testing infrastructure work is deferred until
after this PR merges.

What this PR establishes
========================

The previous QtTest file contained commented-out test examples, with no active
regression cases. This PR adds 17 cases in ``src/tests/siqad_tests.cpp``:

* Ten malformed clipboard item-tree cases, including nested duplicate sites,
  empty aggregates, invalid basis indices and non-integral coordinates.
* Valid nested aggregates with signed lattice coordinates.
* Failed clipboard imports must not reuse a stale local selection.
* First lattice-clip selection retains preview intent.
* A drag with a single move event selects enclosed items.
* Clip controls and preview follow the active design/result lattice.
* Rendering excludes lattice sites outside the clip, with a positive control
  proving that a clip containing a site still renders it.
* A native macOS pinch finish event must not apply another zoom step.

The cases use public entry points, Qt meta-object invocation of private action
slots, widget-label lookup, test-mode settings paths, and layer-ID cleanup.
These are bounded regression tests, not an established fixture or UI-testing
framework. The macOS gesture case requires Qt 6.2 or newer and skips elsewhere.

Tests wanted without an established repository pattern
=====================================================

Separate-process clipboard round trips
-------------------------------------

Launch independent applications, copy through the actual operating-system
pasteboard, paste into another document, save/reload, and assert DB coordinates,
aggregate nesting, target layer routing, and undo/redo. Include source process
exit, occupied destinations, differing layer layouts, unsupported mixed
selections, and lattice compatibility policy. The current suite injects MIME
data in one process; it cannot establish cross-process clipboard ownership or
lifetime. A process harness, isolated profiles, synchronization and cleanup
contract are needed. The ordinary three-DB transfer, translation preservation,
save, undo and redo were checked manually in two running instances for this PR.

Native UI workflows
-------------------

Drive selection, copy/paste placement and both clipping tools through actual
mouse/keyboard input. Verify first-use preview, reset, mode re-entry and lattice
visibility changes using stable control identifiers rather than labels or screen
coordinates. The fast-drag issue was discovered during manual UI validation;
its single-event regression is now automated. There is no established native UI
harness or accessibility-identifier convention for these workflows.

Screenshot export contracts
---------------------------

Exercise SVG export to temporary files, parse geometry and transforms, and prove
that previews are omitted while design items remain. Cover clip boundaries,
rotated views, non-default lattices, occupied sites, scale bars, and restoration
of visibility/selection after rendering or output failure. This PR tests raster
output for the specific boundary defect and manually exports/parses an SVG.
Reusable scene builders and format-aware assertions are still needed; pixel
goldens alone would be sensitive to Qt, fonts and platform rendering differences.

Simulation-result fixtures
--------------------------

Load recorded simulation results through the application's visualization flow,
then validate result-lattice clipping and transitions back to design mode.
The regression suite switches layer-manager mode directly. It does not establish
real result-file loading, charge overlays or engine integration. Small recorded
result fixtures and a policy separating deterministic GUI tests from external
simulator tests are needed.

Gesture and wheel event sequences
--------------------------------

Replay begin/update/finish/cancel events, repeated updates, horizontal and vertical
wheel deltas, modifiers, zoom limits, disabled interactions and loading state.
Check both zoom factor and anchor stability. The existing native-event case
covers the macOS finish defect only. Platform-specific event factories, a
supported Qt-version matrix, and visible skip reporting are needed.

Fixture isolation and parser-focused tests
-----------------------------------------

Create settings isolation before QApplication construction and before static
defaults initialize. Restore the previous system clipboard and reset shared
settings, layer, item and ghost state between tests. Document object ownership
and teardown expectations. Extract the clipboard codec behind a narrow API so
serialization round trips, malformed root/version fields, large/deep trees and
generated inputs can be tested without constructing the full GUI. The current
fixture assumes the default Si(100) lattice and invokes private slots; avoid
turning those conveniences into an infrastructure contract.

Test execution and evidence
---------------------------

Separate building from running tests: CMake currently runs CTest in a test
target's POST_BUILD command, so an unchanged build need not rerun tests.
Unify BUILD_TEST and SKIP_SIQAD_TESTS, define unit/GUI/native/integration labels,
and document developer commands and required prerequisites. Run explicit CTest
steps in CI with timeouts and uploaded failure logs, skipped-case reports and
export artifacts. Keep GUI-only validation available independently of simulator
plugin builds. Add sanitizer runs once fixtures have reliable lifetime cleanup.

Acceptance for the later infrastructure work
============================================

A clean checkout should run the deterministic suite with documented commands,
without reading or altering user preferences or depending on external simulators.
Native and cross-process tests should have explicit environment requirements,
bounded waits and reliable cleanup. CI should distinguish failures, skips and
manual-only coverage, and retain artifacts sufficient to reproduce a failure.
