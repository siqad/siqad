# SiQAD Connector

SiQAD Connector is a class containing convenient functions for physics engines to interact with the SiQAD GUI. The use of the class is recommended, but ultimately optional as developers may want to implement their own I/O with SiQAD. A Python wrapper generated using [SWIG](http://www.swig.org/) has also been included.

Please keep in mind that SiQAD is still undergoing rapid development, there are no guarantees that older versions of SiQAD Connector would be compatible with newer versions of the SiQAD GUI.

## Usage

### C++ projects

Just copy `siqadconn.cc` and `siqadconn.h` to your project directory and include `siqadconn.h` appropriately.

TODO sample code.

### Python projects

There are various ways to acquire a Python wrapper for SiQAD Connector (read the subsections below). After acquiring `siqadconn.py` and `_siqadconn.so` (Linux) or `_siqadconn.pyd` (Windows), copy them to your Python project folder, import siqadconn, and use the SiQADConnector class.

TODO sample code.

#### Pre-compiled binaries

TODO upload pre-compiled Linux, macOS and Windows binaries.

#### Using setup.py

You may use the provided `setup.py` which uses the distutil library to build the wrapper. First, generate the Python and C++ wrapper sources with SWIG:
```
swig -v -python -c++ siqadconn.i
```

Then run `setup.py`:
```
python3 setup.py build_ext --inplace
```

#### Manual compilation

First run:
```
swig -v -python -c++ siqadconn.i
```

Then compile `siqadconn.cc` (requires Boost property tree library) and `siqadconn_wrap.cxx` (requires Python library) using your preferred compiler. Lastly, link the compiled binaries to either `_siqadconn.so` (Linux) or `_siqadconn.pyd`.
