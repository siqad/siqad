# SiQAD

SiQAD (Silicon Quantum Atomic Designer) is a next-generation CAD tool that enables the design and simulation of silicon dangling bond (Si-DB) circuits through an intuitive graphical user interface (GUI) and a modular simulation back-end. The tool currently offers simulators that predict ground-state and dynamic electron configuration of given Si-DB configurations, and electrostatics simulation given electrode layouts. Please read the [arXiv paper](https://arxiv.org/abs/1808.04916) for a detailed introduction to the tool and simulators, and visit the [Walus Lab website](https://waluslab.ece.ubc.ca/siqad/) for information about us and other projects that Walus Lab works on!


## Binary releases

Binary builds for Windows are available [here](https://github.com/retallickj/siqad/releases). [Visual C++ Redistributable for Visual Studio 2015](https://www.microsoft.com/en-ca/download/details.aspx?id=48145) is required to run the Hopping Dynamics simulator on Windows. Note that one of the first-party simulators, PoisSolver, is not available on Windows builds due to the lack Windows support of a dependent module, FEniCS. Future support for the Docker or Windows Subsystem for Linux (WSL) versions of FEniCS [has been planned](https://github.com/retallickj/siqad/issues/33).

For now, Linux and macOS binaries are not distributed and requires the user to build from source.

## Building from source on Linux

### Prerequisites

This tutorial is based on Ubuntu 17.10, and should also work on Ubuntu 18.04 LTS. Install all dependencies on Ubuntu systems:
```
# gui, simanneal and hoppingdynamics dependencies
sudo apt install python3-pip python3-tk make gcc g++ qtchooser qt5-default libqt5svg5-dev qttools5-dev qttools5-dev-tools libboost-dev libboost-filesystem-dev libboost-system-dev libboost-thread-dev pkg-config
# poissolver dependencies
sudo apt install python3-dolfin gmsh swig
sudo pip3 install pillow networkx matplotlib numpy
# hoppingdynamics python dependencies
sudo pip3 install matplotlib numpy scipy pyside2
```

On non-Debian systems, packages equivalent to the ones listed above will be needed. Feel free to contribute to [this issue](https://github.com/retallickj/siqad/issues/32) with dependencies required on other systems.


### Quick Compilation

First, clone the repository (including submodules) onto your local machine using the following command:

```
git clone --recurse-submodules https://github.com/retallickj/qsi-sim.git
```

Next, run the `make_everything` script from the project root:
```
chmod +x make_everything
./make_everything -swig
```

If the `make_everything` script returns errors, please jump to the "Detailed Compilation" section and try again. Otherwise, you should be able to run the binary by running `./build/debug/siqad` from the project root.

We don't have a Linux installation process in place yet as the tool is still under active development, but we welcome ideas and contributions on that regard.


### Detailed Compilation

First, clone the repository (including submodules) onto your local machine using the following command:

```
git clone --recurse-submodules https://github.com/retallickj/qsi-sim.git
```

Next, compile SimAnneal:

```
cd siqad/src/phys/simanneal
make
```

Compile SWIG wrappers for AFM-Sim and PoisSolver:
TODO

Traverse make to the project root and build:
```
cd ../../..
qmake
make
make install
```

Don't be alarmed by `make install`, this won't install the simulator to your system. All it does is compile the binaries and copy the physics simulation files over to the compiled folders. We don't have a Linux installation process in place yet as the tool is still under active development, but we welcome ideas and contributions on that regard.

Finally, run `./build/debug/siqad` from the project root to invoke the GUI. In order to run a hopping animation, create a DB layout, click on the play button on the top bar, choose the "Hopping Animator" engine and run.



### Cross-compiling for Windows from a Ubuntu server

The cross-compiling guide uses [MXE (M Cross Environment)](http://mxe.cc/) for convenient access to pre-compiled cross-compilation binaries and toolchains. First, add their Debian PPA following the guide on their website: [http://pkg.mxe.cc/](http://pkg.mxe.cc/).

Install the following packages:
```
sudo apt install mxe-x86-64-w64-mingw32.static-boost mxe-i686-w64-mingw32.static-boost mxe-x86-64-w64-mingw32.static-qtbase mxe-x86-64-w64-mingw32.static-qtsvg mxe-i686-w64-mingw32.static-qtbase mxe-i686-w64-mingw32.static-qtsvg mxe-x86-64-w64-mingw32.static-qttools mxe-i686-w64-mingw32.static-qttools
```

MXE's Qt does not support compiling in DEBUG mode, so take out the `debug` flag from `CONFIG` and add the `release` flag in `db-sim.pro`:
```
CONFIG += release
```

From the root directory of siqad, create a `win64` directory. From that directory, run:
```
FOR_OS=win64 ../make_everything -swig
```

The `FOR_OS` flag informs the `make_everything` script which platform to cross-compile for, currently supported options are: `FOR_OS=<win32|win64>`. Leave blank if not cross-compiling.


## Licensing

The open source version of Qt5 falls under the GNU LGPL v3 license, as does the GUI code. Qt5 includes some packages which include third-party content under different licenses. If these are used their specific licenses must be considered. Refer to http://doc.qt.io/qt-5/licenses-used-in-qt.html for a list of third-party licensed libraries.
