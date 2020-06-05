# Windows Portable Build Script

To create a portable Windows archive for SiQAD, read the comments at the top of the `win_portable` script file for instructions.

This assumes that you already have the SiQAD Windows executable (`siqad.exe`) and siqadtools wheels (`*.whl`) compiled and ready for deployment. For the developers' workflow, we compile Windows binaries on CI platform AppVeyor. Please consult [appveyor.yml](../../appveyor.yml) for relevant setup parameters and compilation scripts if you would like to adapt that workflow to a native Windows 10 installation.

Two archives should be produced if the packages completes without error: `siqad_x86.zip` for the 32-bit build, and `siqad_x64.zip` for the 64-bit build.


# Cross compilation for Windows from a Ubuntu Host

In the past, we had cross-compiled Windows binaries from Ubuntu. We have ceased to use this method due to glitches and bugs that we've encountered with that method and have moved to AppVeyor (see [appveyor.yml](../../appveyor.yml)) to compile Windows builds natively. The following outdated cross compilation guide is kept for reference, but may require changes to [SiQAD GUI's CMake file](../../src/CMakeLists.txt) for cross compilation to work.

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
