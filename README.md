# SiQAD

SiQAD (Silicon Quantum Atomic Designer) is a next-generation CAD tool that enables the design and simulation of silicon dangling bond (Si-DB) circuits through an intuitive graphical user interface (GUI) and a modular simulation back-end. The tool currently offers simulators that predict ground-state and dynamic electron configuration of given Si-DB configurations, and electrostatics simulation given electrode layouts. Please read the [SiQAD publication on IEEE Transactions on Nanotechnology](https://ieeexplore.ieee.org/document/8963859) for a detailed introduction to the tool and simulators, and visit the [Walus Lab website](https://waluslab.ece.ubc.ca/siqad/) for information about us and other projects that Walus Lab works on!


## Binary releases

Binary builds for Windows are available in the [Releases](https://github.com/retallickj/siqad/releases) page. Note that one of the first-party simulators, PoisSolver, is not available on Windows builds due to incompatibility of its dependencies. Future support via Docker or Windows Subsystem for Linux [has been planned](https://github.com/retallickj/siqad/issues/33). You also have the option of using a virtual machine (Ubuntu 18.04 LTS recommended) or [Windows Subsystem for Linux](https://docs.microsoft.com/en-us/windows/wsl/install-win10) if you're on Windows 10, in which case you will need to compile from source.

For now, Linux and macOS binaries are not distributed and requires compilation from source.

## Building from source on Linux

### Prerequisites

This tutorial is based on Ubuntu 18.04 LTS. Install all dependencies using super user privileges:

```
# general dependencies
apt install python3-pip python3-tk make gcc g++ qtchooser qt5-default libqt5svg5-dev qttools5-dev qttools5-dev-tools libqt5charts5 libqt5charts5-dev libboost-dev libboost-filesystem-dev libboost-system-dev libboost-thread-dev libboost-random-dev pkg-config cmake
# siqadconnector dependencies
pip3 install --user scikit-build
# poissolver dependencies
apt install python3-dolfin gmsh swig
pip3 install --user pillow networkx matplotlib numpy shapely
# hoppingdynamics python dependencies
pip3 install --user matplotlib numpy scipy pyside2
```

---
**NOTE for Ubuntu 20.04 LTS**

PoisSolver is incompatible due to changes in dependent packages that require further adaptation.

---

On non-Debian systems, packages equivalent to the ones listed above will be needed. Feel free to create a GitHub issue to contribute dependencies required on other systems.


### Scripted Compilation

To quickly test out the tool without installing SiQAD into your system, you may use the quick compilation script with the following instructions.

Clone the repository (including submodules) onto your local machine:

```
git clone --recurse-submodules https://github.com/siqad/siqad.git
```

Run the `make_everything_dev` script from the project root:

```
chmod +x make_everything_dev
./make_everything_dev release
```

(When debugging, change the `release` argument to `debug`.)

If the `make_everything_dev` script returns errors, please jump to the "CMake Compilation" section and try again. If compilation is successful, you should be able to invoke the binary by running 

```
./build/release/siqad
```

from the project root. If you've compiled using the `debug` flag, then the binary is located at `./build/debug/siqad` instead.

We don't have a Linux installation process in place yet as the tool is still under active development, but we welcome ideas and contributions on that regard.


### CMake Compilation

Clone the repository (including submodules) onto your local machine:

```
git clone --recurse-submodules https://github.com/siqad/siqad.git
```

In the project root, create a `build` directory and run `cmake`:

```
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=./siqad -DCMAKE_BUILD_TYPE=Release ..
```

Set `-DCMAKE_INSTALL_PREFIX` to your preferred installation path. If it is not set, the default prefix will be set to `/opt/siqad`.

If CMake finishes successfully, compile and install.

```
make
make install
```

For multi-threaded compilation, add the `-j N` flag to `make` where `N` is the number of cores you want to use. `make install` copies the appropriate files to the path set in `CMAKE_INSTALL_PREFIX` in the `cmake` command of the previous step, and may require super user privileges depending on the prefix that you've chosen.

To invoke SiQAD, enter the full path to the binary (e.g. `/opt/siqad/siqad` if `CMAKE_INSTALL_PREFIX` was set to `/opt/siqad`). If you would like to simply invoke SiQAD without having to enter the full path, some of your options include:

1. Add the installation prefix to your `$PATH`;

2. Make a symbolic link from one of directories in `$PATH` to the binary. For example, `ln -s /opt/siqad/siqad "${HOME}/.local/bin/siqad"` if `CMAKE_INSTALL_PREFIX` was set to `/opt/siqad`.



### Cross-compiling for Windows from a Ubuntu host

---

Due to recent problems with the MXE PPA, we have moved the compilation of our Windows builds over to Visual Studio 2017 on AppVeyor. The cross-compilation guide is kept for reference, but may require changes to the `WIN32` section of [SiQAD GUI's CMake file](src/CMakeLists.txt) for cross-compilation to work.

---

**Before you continue:** pre-compiled Windows binaries are available in the [Releases](https://github.com/siqad/siqad/releases) page which allows you to use SiQAD without compiling your own.

There are two major parts to this:

1. Compiling the SiQAD GUI and associated C++ simulators (so far, only SimAnneal is C++), which is detailed here.
2. Compiling the SiQADConnector Python wrapper for Python-based simulation engines. We have not attempted cross-compiling SiQADConnector, and have instead opted for using CI pipelines that offer native Windows compilation support. Please read the [SiQADConnector documentation](https://github.com/siqad/siqadconn/blob/master/README.md) for more information.

If you are more interested in compiling on Windows natively, we have had success in the past through the MinGW-w64 toolchain in MSYS2 as well as a native Visual Studio toolchain. We settled with cross-compilation in the end since all SiQAD developers in Walus Lab develop on Ubuntu.

This cross-compiling guide uses [MXE (M Cross Environment)](http://mxe.cc/) for convenient access to pre-compiled cross-compilation binaries and toolchains. First, either install MXE's pre-compiled binaries from their PPA, or compile MXE from source.
Note that the pre-compiled MXE binaries on the PPA **do not work** due to [this MXE bug](https://github.com/mxe/mxe/issues/2449) at the time of writing. Please check their bug tracker before attempting to use the pre-compiled MXE binaries.

If you opt for their PPA, the following packages are required for 64-bit cross-compilation:
```
apt install mxe-x86-64-w64-mingw32.static-boost mxe-x86-64-w64-mingw32.static-qtbase mxe-x86-64-w64-mingw32.static-qtsvg mxe-x86-64-w64-mingw32.static-qttools mxe-x86-64-w64-mingw32.static-qtcharts
```
Also install i686 cross-compilation packages by changing `x86-64` to `i686` if you would like to compile 32-bit binaries. 

If you opt for compiling from source, you are strongly encouraged to read their [official instructions](https://mxe.cc/#tutorial). For reference, the MXE compilation command that we use is:
```
make MXE_TARGETS='x86_64-w64-mingw32.static' MXE_USE_CCACHE= cmake boost qtbase qtsvg qttools qtcharts
```
Use `MXE_TARGETS='x86_64-w64-mingw32.static i686-w64-mingw32.static'` if you do plan on cross-compiling 32-bit binaries. The inclusion of `MXE_USE_CCACHE=` disables the use of ccache due to [an MXE bug](https://github.com/mxe/mxe/issues/2449).

At SiQAD's project root, edit the `make_everything_dev` script, search for `MXE_PATH=` and change it to where your MXE is compiled or installed. Also read the comments at the top of `make_everything_dev` before running it. When everything is set, run:
```
FOR_OS=win64 ./make_everything_dev
```
Use `FOR_OS=win32` for cross-compiling 32-bit binaries.

At this point, you should find `siqad.exe` in `build-x64/release/` or `build-i686/release/` relative to SiQAD's project root. `simanneal.exe` should also be available in `plugins/simanneal/` relative to the directory where `siqad.exe` is located.

For Python-based simulators, you will need to get [SiQADConnector](https://github.com/siqad/siqadconn/blob/master/README.md) working separately.

Note that when compiling the SiQAD GUI for Windows (refer to `src/CMakeLists.txt`), `qmake` is used rather than relying on CMake's Qt handling. The latter generated very buggy Windows binaries for reasons unknown to us, but seems to not be an isolated issue based on a few StackOverflow posts discussing cross-compiling Qt applications via CMake.

If you don't want to use the `make_everything_dev` script for cross-compilation, read `make_everything_dev` to find the general workflow for compiling it manually.



## Licensing

The open source version of Qt5 falls under the GNU LGPL v3 license, as does the GUI code. Qt5 includes some packages which include third-party content under different licenses. If these are used their specific licenses must be considered. Refer to http://doc.qt.io/qt-5/licenses-used-in-qt.html for a list of third-party licensed libraries.
