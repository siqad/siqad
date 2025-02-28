# SiQAD

[![Ubuntu CI](https://img.shields.io/github/actions/workflow/status/siqad/siqad/ubuntu.yml?label=Ubuntu&logo=ubuntu&style=flat-square)](https://github.com/siqad/siqad/actions/workflows/ubuntu.yml)
[![macOS CI](https://img.shields.io/github/actions/workflow/status/siqad/siqad/macos.yml?label=macOS&logo=apple&style=flat-square)](https://github.com/siqad/siqad/actions/workflows/macos.yml)
[![IEEEXplore](https://img.shields.io/static/v1?label=IEEEXplore&message=SiQAD&color=informational&style=flat-square)](https://ieeexplore.ieee.org/document/8963859)

SiQAD (Silicon Quantum Atomic Designer) is a next-generation CAD tool that enables the design and simulation of silicon dangling bond (Si-DB) circuits through an intuitive graphical user interface (GUI) and a modular simulation back-end. The tool currently offers simulators that predict ground-state and dynamic electron configuration of given Si-DB configurations, and electrostatics simulation given electrode layouts. There are a few key resources available:

* Feel free to join our [official Slack team](https://join.slack.com/t/siqad/shared_invite/zt-enavwvlg-anRYYpslNbpxXI96zx4Wxg) for SiQAD-related discussion and support!
* Please read [the SiQAD Documentation](https://siqad.readthedocs.io/) hosted on Read the Docs. A helpful [Basic Tutorial](https://siqad.readthedocs.io/en/latest/getting-started/basic-tutorial.html) is provided to get you started through a step-by-step tutorial. Other details regarding SiQAD are also documented on the site.
* Please read the [SiQAD publication on IEEE Transactions on Nanotechnology](https://ieeexplore.ieee.org/document/8963859) (open access) for a detailed introduction to the tool and simulators.
* The [Walus Lab website](https://waluslab.ece.ubc.ca/siqad/) contains information about us and other projects that we work on.

We are transitioning most of the information present in the rest of this README onto the documentation webpage. However, the information is still largely relevant at this time.

[![Documentation Status](https://readthedocs.org/projects/siqad/badge/?version=latest)](https://siqad.readthedocs.io/en/latest/?badge=latest)


## Binary releases

Binary builds for Windows are available in the [Releases](https://github.com/retallickj/siqad/releases) page. Note that one of the first-party simulators, PoisSolver, is not available on Windows builds due to incompatibility of its dependencies. For more information, please refer to the [Windows installation](https://siqad.readthedocs.io/en/latest/getting-started/installation.html#windows) section in our official documentation.

For Linux, a Ubuntu PPA is available:
```
sudo add-apt-repository ppa:siqad/ppa
sudo apt-get update
sudo apt-get install siqad
```
You may then invoke SiQAD through the command line: `siqad`. Users of other Linux distributions would have to build from source.

For now, macOS binaries are not distributed and requires compilation from source.


## Building from source on Linux

The compilation documentation has been moved to the [Installation guide](https://siqad.readthedocs.io/en/latest/getting-started/installation.html#linux) on our official documentation.


## Licensing

The open source version of Qt5 falls under the GNU LGPL v3 license, as does the GUI code. Qt5 includes some packages which include third-party content under different licenses. If these are used their specific licenses must be considered. Refer to http://doc.qt.io/qt-5/licenses-used-in-qt.html for a list of third-party licensed libraries.
