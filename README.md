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

### Ubuntu compilation

This tutorial is based off Ubuntu 17.10. First, clone the repository (including submodules) onto your local machine using the following command:

```
git clone --recurse-submodules https://github.com/retallickj/qsi-sim.git
```

You may be prompted for your Github credentials as this is a private repository. Next, install required dependencies:

```
sudo apt install python3-pip python3-tk make gcc g++ qtchooser qt5-default libqt5svg5* qttools5-dev qttools5-dev-tools libboost-dev libboost-filesystem-dev libboost-system-dev
sudo pip3 install matplotlib numpy pyqt5
```

Next, compile the physics engines. The Marcus simulator is the exciting deal now so that will be the only one built in this tutorial. Navigate to `qsi-sim/src/phys/afmmarcus/src` (`qsi-sim` being the root folder of the cloned repository) and run:

```
make
```

Then navigate back to qsi-sim (`cd ../../../..`) and run (without sudo!):

```
qmake && make install
```

Don't be alarmed by the `make install`, this won't install the simulator to your system as long as you don't run it as sudo. All it does is compile the binaries and copy the physics simulation files over to the compiled folders. I know, I know, this will be improved in the future.

Finally, run `./build/debug/db-sim` (from the qsi-sim directory) to run the GUI. In order to run a hopping animation, create a DB layout, click on the play button on the top bar, choose the "Hopping Animator" engine and run. To run a line scan (which only supports one line for now, the top one), choose the "AFM Line Scan" engine. The simulation parameters form is very barebones right now consisting of only textboxes, improvements will be made shortly.

Electrode and AFM paths have not been integrated into the GUI yet.


## Licensing

The open source version of Qt5 falls under the GNU LGPL v3 license, as does the GUI code. Qt5 includes some packages which include third-party content under different licenses. If these are used their specific licenses must be considered. Refer to http://doc.qt.io/qt-5/licenses-used-in-qt.html for a list of third-party licensed libraries.


# TODO

> content directly related to the GUI and not any solver or physics functionality
## GUI

### Generic
* Save DB layouts and load from save
  * ~~Save load script for each class~~ Implemented 17.08.09
  * ~~"You have unsaved changes..."~~ Implemented 17.08.22
  * ~~C-s C-Shift-s shortcuts~~ Implemented
  * ~~Periodic autosave~~ 17.08.22
  * Autorecovery
* Reseting design panel doesn't reset tool type to Select
* Don't deselect cells when entering Panning mode
* When using DBGen Tool, show ghosts indicating where DBs will be created
* Labels (input, output, other arbitrary labels)
* Screen capture tool options (light background mode, capture area)
* CMI mode (e.g. single command to run batch simulations)
* Shift + middle click drag "zoom to region"
* Dialog panel add HTML color processing (and regex remove HTML tags when writing to log)
* Make function that determines whether specific actions are allowed in current display mode
* Break gui::ApplicationGUI::saveToFile apart to a modular file writer, and pick what to include in the output file
* ~~Esc cancels paste operation~~ Implemented 17.07.13
* ~~Esc cancels DB Tool~~ Implemented 17.07.13
* ~~Visual feedback on which tool is currently in use (e.g. changed background of the button)~~ Implemented 17.07.12

### Layers

* Layer editor updates in response to signals emitted from design panel
  * Reset layer editor after loading new layout
* ~~Enumerated layer types~~ Implemented 18.01.15
  * ~~Provide enum to QString conversion~~ 18.01.15
  * ~~Also update save, load, export functions as the Enum strings are different from the original names~~ 18.01.15
* AFM
  * Path primitive object with snap to DB capabilities
  * Constant speed / acceleration profile / etc.
    * Real time info of timing, etc.
  * Side view of AFM path allowing height adjustment and height movement profile, pop-up window when clicked on a segment
* CreateLayer with undo and redo in DesignPanel
* Add zheight property to layers (including updating functions in DP)
* LayerEditor
  * ~~List layers~~ Implemented 18.01.15
  * Add layer
  * Rm layer
  * Rename layer (except for default layers)
  * ~~Edit layer zheight~~ Implemented 18.01.16
* Toggle layer state
  * ~~Layer visibility~~ Implemented 18.01.12
  * Layer editability
    * Current "setActive()" in layer has not been implemented
    * Hiding a layer should also make it uneditable
  * Label visibility (labels can be stored within any layer when implemented)
* Distinguishment between physical layer and logical layer
  * Update code in physics engine

### Aggregates
* Save and load aggregates
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
* ~~Highlight group boundaries when mouse over aggregates~~ Implemented 17.07.12
* ~~Select parent aggregate when clicking on child aggregate~~ Implemented 17.07.12

### Electrode Design
* Side view of vertical electrode stack
* ~~Top view of electrode layers~~
* ~~Creating, moving, and deleting electrodes~~
* Snapping, aligning and distributing like Inkscape
* ~~Save/Load~~
* ~~Setting potentials individually, by batch~~

### Config
* Make config file paths configurable
* User-friendly config file, custom functions in QSettings to read from config to QSetting's own structure / writeback changed settings to user-readable file

### Lattice
* Background lattice sites -> change to bitmap for efficiency
* Order of a1 and a2 in getLatticeInds matters (segfault)


> solvers, physics engine, and I/O formatting
## Physics Engine

* Interface with solvers (standards for passing DB configuration to them, and taking results back)
* Reset SimManager after design panel reset
* Open new window for showing sim results
* Custom class containing physical structure
    * ~~Import size and potential data for electrodes into solver~~
    * Translate size from Qt units to physical lengths
    *	Add buffer region surrounding simulation area
* Location, dimensions, etc
  * Custom class containing properties
* Simple estimation tool of electron distribution
* Static or animated display of charge (like the AFM images)
* Simulation visualization panel that allows users to control visualization of simulation results
  * Control what type of result to show
  * Filter results, e.g. only show results with 2 electrons
  * Time control, if the simulator supports that
  * Degenerate state visualization
  * stuff like that
* SimAnneal
  * Distance dependent hopping: precompute the probability of hopping from each site to any other site, put into matrix
* PoisSolver
	* Custom class containing physical structure
		* ~~Import size and potential data for electrodes into solver~~
		* Translate size from Qt units to physical lengths
		*	~Add buffer region surrounding simulation area~
		* Location, dimensions, etc
		* Implement heat map/colour map support for PoisSolver

> general bugs
## Bugs

* Segfault when undoing aggregates then moving dots
* When attempting to select DBs using rubberband but clicked on a DB/aggregate as the starting location, the object at the starting position takes the press event and rubberband fails to show


> Todo list for the current branch
## Ongoing

* Interface with solvers (standards for passing DB configuration to them, and taking results back)
  * Write data structure to xml
    * Use normal save xml for now
    * In the XML, add a section containing simulation parameters
    * Temperarily use an available xml parser for now, might change later (rapidxml?)
    * Material, material parameters/properties (that can be overidden by the simulator), DB locations
    * Aggregates (in the future: predetermined simulation parameters for aggregates can be stored)
		* ~~Electrode potential and size~~
		* A way to add a "buffer" region to the outer simulation boundaries.
		* Specification of potential simulation boundaries.
		* Translating the QPoint locations in Qt to physical locations in the simulation.
		* Ensuring that Electrodes do not overlap with each other.
* Simple estimation tool of electron distribution
  * Simulated annealing algorithm with 1. electron population determined by bulk-DB interaction and 2. inter-DB electron hopping.
  * Export results to file for gui to read - time, charge distribution, etc

GUI side
* Get available engines
  * Read engine properties from certain directory, properties are stored in XML files
  * Simulation jobs are ran on a runtime temp directory that is structured as follows:
    [tempDir]/[engineDir]/[problemDir]
  * For now, SimEngine::runtimeTempDir is hard coded to return the only physeng
* Show simulation options
  * Show options relevant to the selected engine
* Problem and result files are stored in tmp
  * Delete them when quiting program
* Read simultion result
  * ApplicationGUI::readSimResult still has many unfinished sections

The following mess was made before d-wave meeting, will consolidate into list above
* Rundown:
  * Sim Setup (pick simulator, adjust simulation params)
    * There should be flags in the simulator info XML controlling what kind of params are exported.
    * Other simulation params are described in the simulator info XML too I guess?
  * Run simulation, shows simulation text output
    * XML file describing the problem and parameters is saved to a specific directory that queues simulations. The binary of the simulator is called to run the simulation.
  * When simulation completes, allow users the following options: 1. visualize results (details of visualization, like filters and whatnot, should be handled by another widget I guess); 2. save simulation results
    * Simulation results, corresponding params, energy levels, etc. should be stored to different classes?
* Detect runtime error messages from the simulator and alert the user
Short future:
* Static or animated display of charge (like the AFM images)
  * For degenerate states where the hop is of short distance? Might need to change the way the results are exported to make this easier.
* Has a result screen that allows user to view previous results
Future:
* Show currently running jobs
* For example, some jobs might be detailed simulation for small aggregates, while another crude simulation could be ongoing in the background.
* If this widget is called from the main window while an instance of this is open, just focus such instance.
* Assuming that sim takes a long time to run, and user makes changes to DB, when sim is complete user can open a new tab to view the results without disturbing the updated design.
* gui::DesignPanel::displaySimResults to handle multiple types of simulation results that should be shown


> Past TODOs for implementations of major features
## Past detailed TODOs

### save-load branch
All tasks described here contribute to save/load functionality
* layer id related
  * ~~Change items to store layer id instead of layer pointer~~ Implemented 17.08.01
  * ~~Add functionality to change layer id stored in layers and in child items when the layer's index changes in the stack~~ Implemented 17.08.02
  * Clean up design panel code for layer id (make it less 'hacked-together', get rid of unnecessary converions)
* ~~Undo/Redo stack indexing~~ Implemented 17.08.16
  * ~~Make base class that always increments for each item added to the stack~~ Implemented 17.08.15
  * ~~Each item stores the undo/redo stack ID~~ Implemented 17.08.15
  * ~~When autosave/manual save are performed, store the stack id at which it was performed~~ Implemented 17.08.16
* Saving
  * ~~Nested saving - recursive~~ Implemented 17.08.09
  * ~~File write error handling~~ Implemented 17.08.16
* Loading
  * ~~Nested loading of items - recursive~~ Implemented 17.08.11
  * ~~Load screen offset, zoom and rotation~~ Implemented 17.08.23
  * Load layers with appropriate properties
  * Error alert dialog if latdot cannot be found during dbdot generation
  * Clean up the XML read write code to have a consistent style
* ~~Save dialog when quitting~~ Implemented 17.08.16
* Autosaving per x minutes
  * ~~Detect changes in program. If no changes, don't run autosave.~~ Implemented 17.08.15
  * ~~Number of saves~~ Implemented 17.08.16
  * ~~Rotational save in a folder dedicated to that program instance~~ Implemented 17.08.16
  * Put tmp directory to system tmp?
  * Delete autosaves when quiting the program gracefully
* autorecovery
  * Check for existing autosaves, ask the user whether they want to recover the latest autosave.

* Code improvement
  * QLockFile for locking instance folder during creation
    * Or, get the process ID and use that as part of the directory name - QCoreApplication::applicationPid()
