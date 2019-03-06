This Qt Installer Framework is intended for packaging on Windows for Windows installation. It can be extended to other OSes in principle.

# Procedure

Add the required files for installation to the @packages\COMPONENT\data@ directory.

From this directory, run the following command in PowerShell:
```
binarycreator --offline-only -c config/config.xml -p packages installer
```
An installer.exe file will be created in this directory. It can be renamed and moved as needed.
