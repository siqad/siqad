# Creating deb packages

This may not represent the most efficient workflow, it is merely a record of what worked for Walus Lab.

## Resources

[PPA with CMake!](https://schneegans.github.io/lessons/2011/11/02/ppa-launchpad-cmake)
[IntroDebianPackaging](https://wiki.debian.org/Packaging/Intro?action=show&redirect=IntroDebianPackaging)

## Packaging Procedure

First, prepare a clean archive of the SiQAD source code. The SiQAD repository contains submodules but the `git archive` command doesn't archive submodules work out of the box. My lazy approaching of acquiring clean SiQAD source code is just to clone the repository recursively:

```
git clone --recurse-submodules https://github.com/siqad/siqad.git siqad-0.2.2
```

Change the version number `0.2.2` to the latest version of SiQAD being packaged, the source code should now be cloned into the directory `siqad-0.2.2` relative to where the command was executed. A couple of changes are needed:

```
rm -rf siqad-0.2.2/docs
rm -rf siqad-0.2.2/installer

```

Now archive it by:

```
tar -czvf siqad_0.2.2.orig.tar.gz siqad-0.2.2
```

where, once again, the version number `0.2.2` must be adapted.

Prepare the following directory structure:

```
.
|-- siqad-0.2.2
|   |-- debian
|   |   |-- compat
|   |   |-- control
|   |   |-- copyright
|   |   |-- rules
|   |   |-- source
|   |   |   |-- format
|   |-- [all of the other source files]
|-- siqad_0.2.2.orig.tar.gz
```

Here, `siqad-0.2.2` contains all of the source files as well as the `debian` directory. The `debian` directory content is copied from `[siqad_repo_root]/packaging/debian/debian`.

## Update Configuration

Change into the `siqad-0.2.2` directory and create a changelog by

```
dch --create -v 0.2.2-1 --package siqad
```

where the version needs to be adapted. The `0.2.2` part is the upstream version; the `-1` part is the Debian version. If a future bug fix is introduced to the Debian version based on the same upstream package, then increment to `-2` and so on. Inspect the created `debian/changelog` file and make further changes as needed (e.g. email address, closed issues, etc.)

Edit the `debian/rules` file and change number following `-j` in the line `make -j24 -C $(BUILDDIR)` to the number of CPU cores you can allocate for the build.

TODO: there should be more to change here for version upgrades, revisit later.

Now we compile the package:

```
debuild -i -us -uc
```

Take note of the lintian outputs at the end of the `debuild` run. Outputs preceded by `E: ` and `W: ` are errors and warnings respectively, errors need to be dealt with.

If all goes well, the directory which originally contained `siqad_0.2.2.orig.tar.gz` should now also contain a few additional files, including the DEB package.
