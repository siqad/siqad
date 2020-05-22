Installation
************

First, we need to make this tool available on your machine:

* `Windows`_
* `Linux`_
* `macOS`_

.. note::

    The electrostatic landscape solver, PoisSolver, is currently **only** available on Ubuntu 18.04 LTS. It is unavailable on other platforms for various reasons:

    * Windows pre-compiled binaries: PoisSolver's dependent package, `FENiCS <https://fenicsproject.org/>`_, does not offer native Windows support. Use Windows Subsystem for Linux instead.
    * Ubuntu 20.04 LTS: Breaking changes in dependent libraries have left PoisSolver unuseable on new Ubuntu versions for now. Some temporary remedies include: running SiQAD in a docker container, running SiQAD in a 18.04 LTS virtual machine, contribute to implementing a fix, etc.
    * macOS: Currently untested.

    If you are just getting started with SiQAD, there is still much to explore in SimAnneal and HoppingDynamics to get you started. However, advanced designs involving clocking electrodes will certainly require the use of PoisSolver.

Windows
=======

Pre-compiled Binaries
---------------------

Pre-compiled binaries are available for Windows 10. They can be downloaded on the official `SiQAD GitHub releases page <https://github.com/siqad/siqad/releases>`_ with both x86 (32-bit) and x86-64 (64-bit) builds available. For now, the binaries are packaged in portable form, installer support will be added in the future.

See the note at the top of the page regarding the lack of PoisSolver support in the pre-compiled binaries as well as possible remedies.

Some of the bundled simulators require a Python interpreter (`official download page <https://www.python.org/downloads/>`_). Supported versions include Python 3.5 to 3.8. The following PyPI packages are also required:

* matplotlib
* numpy
* scipy
* pyside2

which can be acquired via pip (`pip module installation guide <https://docs.python.org/3/installing/index.html>`_).


Windows Subsystem for Linux
---------------------------

Windows Subsystem for Linux (WSL) enables the execution of Linux binaries within Windows provided that you have an up-to-date copy of Windows 10. You may refer to the `official guide from Microsoft <https://docs.microsoft.com/en-us/windows/wsl/install-win10>`_ for enabling WSL. **Please use the Ubuntu 18.04 LTS image if you require the use of PoisSolver**.

You will also need an X11 server to display graphical applications. We have tested Xming and it works, but other alternatives out there should also be functional.

After setting up WSL, you may refer to the `Linux`_ section for compiling the binaries from source within the WSL environment.


Linux
=====

We intend to submit SiQAD to package maintainers for binary distribution when the source code becomes more mature. For now, SiQAD needs to be compiled from source.


Building from Source
--------------------

Prerequisites
+++++++++++++

This tutorial is based on Ubuntu 18.04 LTS, but should also work on Ubuntu 20.04 LTS (see note at the top of the page regarding problems with PoisSolver on 20.04 LTS). Install the following prerequisites::

    # general dependencies
    sudo apt install python3-pip python3-tk make gcc g++ qtchooser qt5-default libqt5svg5-dev qttools5-dev qttools5-dev-tools libqt5charts5 libqt5charts5-dev libboost-dev libboost-filesystem-dev libboost-system-dev libboost-thread-dev libboost-random-dev pkg-config cmake
    # siqadconnector dependencies
    pip3 install --user scikit-build
    # poissolver dependencies
    sudo apt install python3-dolfin gmsh swig
    pip3 install --user pillow networkx matplotlib numpy shapely
    # hoppingdynamics python dependencies
    pip3 install --user matplotlib numpy scipy pyside2

On non-Debian systems, packages equivalent to the ones listed above will be needed.


Scripted Compilation
++++++++++++++++++++

To quickly test out the tool without installing SiQAD into your system, you may use the quick compilation script with the following instructions.

Clone the repository (including submodules) onto your local machine::

    git clone --recurse-submodules https://github.com/siqad/siqad.git

Run the make_everything_dev script from the project root::

    chmod +x make_everything_dev
    ./make_everything_dev release

(When debugging, change the release argument to debug.)

If the ``make_everything_dev`` script returns with errors, please jump to the `Step-by-step Cmake Compilation`_ section and try again. If compilation is successful, you should be able to invoke the binary by running::

    ./build/release/siqad

from the project root. If you've compiled using the debug flag, then the binary is located at ``./build/debug/siqad`` instead.

If you would like to be able to invoke SiQAD from anywhere in your system when you're logged in, make a symbolic link from one of the directories in ``$PATH`` to the SiQAD binary. For example, if the binary exists in ``~/git/siqad/build/release/siqad`` and if ``~/.local/bin`` is in your ``$PATH``, you may create a symbolic link by::

    ln -s ~/git/siqad/build/release/siqad ~/.local/bin/siqad

After this, you will be able to invoke SiQAD simply by running ``siqad`` as your logged in user.


Step-by-step CMake Compilation
++++++++++++++++++++++++++++++

Clone the repository (including submodules) onto your local machine::

    git clone --recurse-submodules https://github.com/siqad/siqad.git

In the project root, create a build directory and run cmake::

    mkdir build && cd build
    cmake -DCMAKE_INSTALL_PREFIX=./siqad -DCMAKE_BUILD_TYPE=Release ..

Set ``-DCMAKE_INSTALL_PREFIX`` to your preferred installation path. If it is not set, the default prefix will be set to ``/opt/siqad``.

If CMake finishes successfully, compile and install::

    make
    make install

For multi-threaded compilation, add the ``-j N`` flag to ``make`` where ``N`` is the number of cores you want to use. ``make install`` copies the appropriate files to the path set in ``CMAKE_INSTALL_PREFIX`` in the ``cmake`` command of the previous step, and may require ``sudo`` privileges depending on the prefix that you've chosen.

To invoke SiQAD, enter the full path to the binary (e.g. ``/opt/siqad/siqad`` if CMAKE_INSTALL_PREFIX was set to ``/opt/siqad``). If you would like to simply invoke SiQAD without having to enter the full path, some of the options include:

* Adding the installation prefix to your ``$PATH``;
* Making a symbolic link from one of directories in ``$PATH`` to the binary. For example, ``ln -s /opt/siqad/siqad "${HOME}/.local/bin/siqad"`` if ``CMAKE_INSTALL_PREFIX`` was set to ``/opt/siqad``.




macOS
=====

We do not have an official compilation guide for macOS yet. However, we have had success compiling SiQAD on macOS in the past, albeit haphazardly. We recommend following the `Step-by-step CMake Compilation`_ tutorial for Linux and adapt/debug along the way.
