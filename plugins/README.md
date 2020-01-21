Each plugin occupies its own subdirectory. For example:

./                                  Current directory.
./example-plugin                    Subdirectory containing the plugin.
./example-plugin/example.sqplug     Plugin description file, similar to the `*.physeng` files for physics engines (TODO in the future, the two filetypes might be merged).
./example-plugin/example.py         Plugin binary/script, in this example it is a Python file.

TODO eventually, the SiQAD directories should be set up in the way that all C++ plugins/engines point to the same SiQADConnector header, and all Python plugins/engines point to the same SWIGed SiQADConnector module. The SiQADConnector source can be distributed with all binaries such that it can be recompiled in the user's environment for special cases (e.g. running in Docker, WSL, etc.).
