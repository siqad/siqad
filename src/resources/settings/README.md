These XML files are used for generating setting dialogs via the PropertyMap class. Each file is for one tab/page of settings. The `val` element should be left blank as the user setting would be filled into those fields instead. 

Two elements must be present inside the `meta` element: `category` and `key`. They are used to reference the appropriate setting category (`app`, `gui` or `lattice`) and the key for the setting.

TODO specification of XML files.
