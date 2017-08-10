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
  * Save load script for each class
    * Each item has a unique ID? Aggregates save those ID instead of pointer?
    * Things that need to be saved:
      * Layers (future-proof with electrodes and all)
      * Aggregate structures
      * DBDots
      * (Optional) Undo stack
  * "You have unsaved changes..."
  * C-s C-Shift-s shortcuts
  * Periodic autosave (and recovery)
    * tmp folder, 3-5 files saved per minute or so
    * on startup, check tmp folder for autosaved files
    * on shutdown, remove those files
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


> keep track of remaining tasks on ongoing work
### ONGOING
All tasks described here contribute to save/load functionality
* layer id related
  * ~~Change items to store layer id instead of layer pointer~~ Implemented 17.08.01
  * ~~Add functionality to change layer id stored in layers and in child items when the layer's index changes in the stack~~ Implemented 17.08.02
  * Clean up design panel code for layer id (make it less 'hacked-together', get rid of unnecessary converions)
* aggregate related
  * NOTE: Top level aggregates are still just items inside the layer
  * NOTE: Inefficient to have items refer to an aggregate id or something like that
  * NOTE: e.g. when an aggregate is deleted, all items in subsequent aggregates have to update their IDs
  * NOTE: What about only assigning an ID when saving, and keep using pointers otherwise?
  * Saving
    * ~~Nested saving - recursive~~ Implemented 17.08.09
    * Escape names properly
    * Example:
    <dbdesigner>
      <!-- gui flags -->
      <gui>
        <lattice>si_100_2x1</lattice> <!-- DO NOT CHANGE FROM HERE -->
      </gui>

      <layer_prop>
        <name>DB Main</name>
        <visible>1</visible>
        <active>1</active>
      </layer_prop>

      <layer>
        <aggregate>
          <aggregate>
            <dbdot>
              <layer_id>1</layer_id>
              <phys_loc x="1" y="1" />
            </dbdot>
            <dbdot>
              <layer_id>1</layer_id>
              <phys_loc x="3" y="1" />
            </dbdot>
          </aggregate>
          <dbdot>
            <layer_id>1</layer_id>
            <phys_loc x="2" y="2" />
          </dbdot>
        </aggregate>
      </layer>
    </dbdesigner>
    
  * Loading
    * Nested loading - recursive
    * while inside while implementation?
    * Right now lots of repeated code, try to reuse code when possible
    * Potential of not being able to find latdot for creating dbdot?
    * CLEAN UP THE CODE
* writing to save file
  * SaveLoad class
    * File write handling and XML stream handling
  * saveToFile function in design panel (layer stack)
  * saveToFile function in each primitive class
* loading from save file
  * SaveLoad class
    * File read handling and XML stream handling
  * loadFromFile function in each primitive class
  * debug message if there are errors in the XML file, or even an automatic fixer
* autosaving per x minutes
  * Detect changes in program. If no changes (maybe undo/redo stack could help?), don't autosave.
  * Autosave to some temp directory. Write to autosave-yymmdd-hhmmss_in-progress.xml, then mv to autosave-yymmdd-hhmmss.xml when finished.
  * If program exited normally and user doesn't want the autosave, keep the autosave as autosave-yymmdd-hhmmss_discarded.xml just in case the user regrets...
  * When the user saves manually, discard autosaves
* autorecovery
  * Check for autosave-yymmdd-hhmmss.xml, ask the user whether (s)he wants to recover the latest autosave.
