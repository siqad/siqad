# SiQAD Connector

SiQAD Connector is a class containing convenient functions for physics engines to interact with the SiQAD GUI. The use of the class is recommended, but ultimately optional as developers may want to implement their own I/O with SiQAD. A Python wrapper generated using [SWIG](http://www.swig.org/) has also been included. Note that SiQAD Connector uses Apache License 2.0 as opposed to the GUI's LGPLv3 in order to allow physics engines to use any license compatible with APLv2.

Please keep in mind that SiQAD is still undergoing rapid development, there are no guarantees that older versions of SiQAD Connector would be compatible with newer versions of the SiQAD GUI.

## Usage

### C++ projects

Just copy `siqadconn.cc` and `siqadconn.h` to your project directory and include `siqadconn.h` appropriately.

TODO: sample code.

### Python projects

There are various ways to acquire a Python wrapper for SiQAD Connector (read the subsections below). After acquiring `siqadconn.py` and `_siqadconn.so` (Linux) or `_siqadconn.pyd` (Windows), copy them to your Python project folder, import siqadconn, and use the SiQADConnector class.

TODO sample code.

#### Use pre-compiled binaries

TODO upload pre-compiled Linux, macOS and Windows binaries.

#### Using setup.py

TODO list dependencies. Off the top of my head: from apt `boost` and `swig`, from pip3 `scikit-build`

You may use the provided `setup.py` which uses the distutil library to build the wrapper. Using your preferred Python interpretor (here we use `python3`), run
```
python3 setup.py bdist_wheel
```
which creates a Python wrapper of SiQADConnector via SWIG, compiles SiQADConnector, and packages the generated files into a proper Python package in the `dist` directory.

You should now find `siqadtools-[version]-[platform_info].whl` inside the `dist` directory. Install this into your system by running
```
pip3 install siqadtools-[version]-[platform_info].whl
```
substituting `pip3` with your preferred Python package manager, `[version]` and `[platform_info]` with what you see in the generated wheel file.

To uninstall, run
```
pip3 uninstall siqadtools
```
