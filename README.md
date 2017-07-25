# qsi-sim

Simulation Tool for QSi's dangling bonds. There are two proposed contributions:

	1. A CAD tool to assist in designing arrangements of dangling bonds on the surface.
  
	2. A set of physics tools for the simulations of these arrangements.



## Prerequisites

#### GUI

The GUI is currently written in C++ and uses the Qt5 framework. The Qt5 framework will need to be installed in order to compile the source:

1. The latest Qt5 source can be installed from https://github.com/qt/qt5, see their README for build instructions.

2. Alternatively, an installer for the latest stable framework can be obtained from https://www.qt.io/download/. Currently, all source code for the GUI can fall under the GNU LGPL v3 license so select "Open source distribution...". This may change in future.

Depending on your installation method, you may need to first install Qt5 dependencies. On debian systems, these can be installed as (build-essential, libfontconfig1, mesa-common-dev, libglu1-mesa-dev).

	
## Building

There is currently no installer binary so the tool must be built from source. Development has been done purely in Linux and there has been no modification made to the .pro file to account for any additional OS-specified linking/includes.

If you are going to load the source as a project in Qt Creator, make a copy of the .pro file before first compile to prevent including unnecessary formatting in the official version. When committing your code, move only the necessary changes into the official .pro file with comments as needed.


## Licensing

The open source version of Qt5 falls under the GNU LGPL v3 license, as does the GUI code. Qt5 includes some packages which include third-party content under different licenses. If these are used their specific licenses must be considered. Refer to http://doc.qt.io/qt-5/licenses-used-in-qt.html for a list of third-party licensed libraries.


# TODO

> content directly related to the GUI and not any solver or physics functionality
## GUI

### Generic
* Save DB layouts and load from save
  * "You have unsaved changes..."
  * Periodic autosave
* Don't deselect cells when entering Panning mode
* When using DBGen Tool, show ghosts indicating where DBs will be created
* Labels (input, output, other arbitrary labels)
* Screen capture tool options (light background mode, capture area)
* CMI mode (e.g. single command to run batch simulations)
* Shift + middle click drag "zoom to region"
* ~~Esc cancels paste operation~~ Implemented 17.07.13
* ~~Esc cancels DB Tool~~ Implemented 17.07.13
* ~~Visual feedback on which tool is currently in use (e.g. changed background of the button)~~ Implemented 17.07.12

### Aggregates
* Disallow creation of new dots inside aggregates
* Offset of moving aggregates - ghost should not be centered to the cursor, instead centered at the same offset as the starting point
* Component library
* Tight aggregate boundaries (instead of the current sqaure, taking up too much space)
  * Multiple aggregate boundary algorithms, so we can choose the one that ensures the highest accuracy depending for the standard library.
  * Possibly implement Chan's algorithm for faster convex hull computation
  * Right click on object to change the aggregate boundary algorithm, give out warning if they're attempting to do this on an aggregate that came from a library
  * Associate hull computation wit changes to the aggregate rather than the ::shape function (recomputes every time the boundary is painted/checked).
* "flattenAggregate" function: for each selected aggregate, for each child of that aggregate, split if an aggregate and add children to parent.
* Enter aggregate to make changes inside the aggregate - aggregate layers (like entering group in Inkscape)
* Save and load aggregates
* ~~Highlight group boundaries when mouse over aggregates~~ Implemented 17.07.12
* ~~Select parent aggregate when clicking on child aggregate~~ Implemented 17.07.12

### Electrode Design
* Side view of vertical electrode stack
* Top view of electrode layers
* Creating, moving, and deleting electrodes
* Snapping, aligning and distributing like Inkscape

### Config
* Make config file paths configurable
* User-friendly config file, custom functions in QSettings to read from config to QSetting's own structure / writeback changed settings to user-readable file

### Lattice
* Background lattice sites -> change to bitmap for efficiency


> solvers, physics engine, and I/O formatting
## Physics Engine

* Interface with solvers (standards for passing DB configuration to them, and taking results back)
* Simple estimation tool of electron distribution
* Static or animated display of charge (like the AFM images)

> general bugs
## Bugs

* Segfault when undoing aggregates then moving dots
* When attempting to select DBs using rubberband but clicked on a DB/aggregate as the starting location, the object at the starting position takes the press event and rubberband fails to show



