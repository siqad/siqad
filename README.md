# SiQAD

SiQAD (Silicon Quantum Atomic Designer) is a next-generation CAD tool that enables the design and simulation of silicon dangling bond (Si-DB) circuits through an intuitive graphical user interface (GUI) and a modular simulation back-end. The tool currently offers simulators that predict ground-state and dynamic electron configuration of given Si-DB configurations, and electrostatics simulation given electrode layouts. There are a few key resources available:

* Feel free to join our [official Slack team](https://join.slack.com/t/siqad/shared_invite/zt-enavwvlg-anRYYpslNbpxXI96zx4Wxg) for SiQAD-related discussion and support!
* Please read [the SiQAD Documentation](https://siqad.readthedocs.io/) hosted on Read the Docs. A helpful [Basic Tutorial](https://siqad.readthedocs.io/en/latest/getting-started/basic-tutorial.html) is provided to get you started through a step-by-step tutorial. Other details regarding SiQAD are also documented on the site.
* Please read the [SiQAD publication on IEEE Transactions on Nanotechnology](https://ieeexplore.ieee.org/document/8963859) (open access) for a detailed introduction to the tool and simulators.
* The [Walus Lab website](https://waluslab.ece.ubc.ca/siqad/) contains information about us and other projects that we work on.


## Binary releases

Binary builds for Windows are available in the [Releases](https://github.com/retallickj/siqad/releases) page. Note that one of the first-party simulators, PoisSolver, is not available on Windows builds due to incompatibility of its dependencies. For more information, please refer to the [Windows installation](https://siqad.readthedocs.io/en/latest/getting-started/installation.html#windows) section in our official documentation.

For Linux, our PPA is out of date. For now please compile from source.


## Building from source

### Ubuntu

1. Install build dependencies:

```
# 22.04 LTS
sudo apt install cmake pkg-config python3-pip python3-tk python3-venv make gcc g++ qt6-base-dev qt6-tools-dev libqt6charts6-dev libqt6uitools6 libqt6svg6-dev libboost-dev libboost-filesystem-dev libboost-sytem-dev libboost-thread-dev libboost-random-dev libxkbcommon-dev

# 24.04 LTS
sudo apt install cmake pkg-config python3-pip python3-tk python3-venv make gcc g++ qt6-base-dev qt6-tools-dev qt6-svg-dev qt6-charts-dev libqt6charts6 libqt6uitools6 libqt6svg6 libboost-dev libboost-filesystem-dev libboost-sytem-dev libboost-thread-dev libboost-random-dev
```

2. Run the build script from project root

```
./make_everything_dev release
```

Or substitute `release` with `debug` if so desired.

3. Run the compiled binary

```
./build/release/siqad
```

Substitute `release` with `debug` if you used the `debug` flag in the previous step.


Note that PoisSolver currently might not work with Ubuntu 22.04 and 24.04.


### macOS

Once you have installed Xcode and other required dependencies, steps 2 and 3 from the Ubuntu compilation section should work for you. An easy way to get the required packages is through [Homebrew](https://brew.sh/). The following packages are needed:

* CMake
* Boost
* Qt6


### Windows

You can refer to our [Windows binary build workflow](/.github/workflows/build-windows.yml) to get a sense of the required packages and build workflow.


## Licensing

The open source version of Qt5 falls under the GNU LGPL v3 license, as does the GUI code. Qt5 includes some packages which include third-party content under different licenses. If these are used their specific licenses must be considered. Refer to http://doc.qt.io/qt-5/licenses-used-in-qt.html for a list of third-party licensed libraries.
