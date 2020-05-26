This Snap packaging config is currently not in use due to difficulties in packaging PoisSolver's dependency, `python3-dolfin`. Nevertheless, everything else other than PoisSolver works. The snap release will be submitted for release once the PoisSolver dependency problem has been resolved.


# Building the Snap

Make sure snapcraft is available on your system: [introduction to snapcraft](https://snapcraft.io/blog/introduction-to-snapcraft).

From the parent directory of `snapcraft.yaml` (which, in this repository, is the directory where this read me is located), run:

```
SNAPCRAFT_BUILD_ENVIRONMENT_MEMORY=[INSERT_VALUE] snapcraft
```

where `[INSERT_VALUE]` must be replaced by the memory size that you would allocate to the [multipass](https://github.com/canonical/multipass) virtual machine used for the build. Without setting it, the default allocated memory for the virtual machine is insufficient for performing this build. `SNAPCRAFT_BUILD_ENVIRONMENT_MEMORY=8G` works for me but I imagine smaller values should also suffice. Please choose a value that is sensible for your build machine. Beware of your network data limits as a clean build fetches a great number of sizable packages from apt.

If the build finishes without error, you may install the build by:

```
sudo snap install --devmode siqad_[VERSION]_amd64.snap
```

where `[VERSION]` must be replaced with the version of the generated file. For now, this version string is hard-coded in `snapcraft.yml` and has no consequences for the actual SiQAD version packaged.

You can then invoke SiQAD by

```
siqad
```


# Other resources

These snippets were useful for getting pip dependencies going: [link](https://forum.snapcraft.io/t/my-experience-with-python-matplotlib-numpy-pip-and-a-snap/10658).
