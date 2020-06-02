# SiQAD Unit Tests

These unit tests are intended to be performed at compile time in order to ensure programming correctness. The implementation of these unit tests are still in early phases, the following list of tests represent the intended priority for future unit test updates:

* Save/load file consistency
* Proper loading of design files from older versions of SiQAD
* Simulation problem file generation and result file reading

In the beginning, unit testing will rely on the programming interface. GUI unit testing will be the next step afterwards:

* Item creation and removal
* Electrode resizing and rotation
* Electrode property updates and their undo/redo
* Undo/redo actions
* Proper display mode switching and proper rejection of prohibited actions in certain display modes (e.g. no DB/electrode creation at simulation display mode)

Unit testing for plugins are to be done separatedly within the repositories of those plugins, not lumped together here.
