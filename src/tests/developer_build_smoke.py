"""Real CMake/Qt builds exercising developer-script cache and install recovery."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import tempfile


def cache(build):
    values = {}
    for line in (build / "CMakeCache.txt").read_text().splitlines():
        if "=" in line and not line.startswith(("#", "//")):
            key, value = line.split("=", 1)
            values[key.split(":", 1)[0]] = value
    return values


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument("--generator", default="Ninja")
    parser.add_argument("--artifacts", type=Path, required=True)
    args = parser.parse_args()
    source = Path(__file__).resolve().parents[2]
    args.artifacts.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, JOBS="4")
    for key in ("CMAKE_FLAGS", "FOR_OS", "CMAKE_COMMAND", "CTEST_COMMAND", "QMAKE_COMMAND"):
        env.pop(key, None)
    with tempfile.TemporaryDirectory(prefix="siqad-real-build-") as tmp:
        root = Path(tmp) / "checkout with spaces"
        root.mkdir()
        shutil.copy2(source / "make_everything_dev", root / "make_everything_dev")
        # Use the actual project sources/CMake. Disable only simulator plugins;
        # this suite measures the GUI build script, not external engine installs.
        (root / "CMakeLists.txt").write_text(
            'cmake_minimum_required(VERSION 3.20)\nproject(SiQADScriptSmoke)\ninclude(CTest)\n'
            + 'add_subdirectory("' + source.as_posix() + '" siqad)\n')
        build = root / "build"
        for index, (mode, notest) in enumerate((("release", True), ("debug", False), ("debug", False))):
            prefix = root / ("installed " + str(index))
            if index == 2:
                with (build / "CMakeCache.txt").open("a") as file:
                    if os.uname().sysname == "Darwin":
                        file.write("\nCUPS_INCLUDE_DIR:PATH=/stale-sdk/usr/include\n")
                        file.write("OPENGL_INCLUDE_DIR:PATH=/stale-sdk/usr/include\n")
                    file.write("\nSIQAD_INSTALL_ROOT:STRING=/stale-install\n")
            command = ["bash", str(root / "make_everything_dev"), mode]
            if notest: command.append("notest")
            command += ["--", "-G", args.generator, "-DSIQAD_BUILD_PLUGINS=OFF",
                        "-DCMAKE_INSTALL_PREFIX=" + str(prefix)]
            with (args.artifacts / (f"{index}-{mode}.log")).open("w") as log:
                subprocess.run(command, cwd=tmp, env=env, stdout=log, stderr=subprocess.STDOUT,
                               check=True, timeout=1200)
            values = cache(build)
            assert values["BUILD_TESTING"] == ("OFF" if notest else "ON"), values
            assert values["SIQAD_INSTALL_ROOT"] == str(prefix), values["SIQAD_INSTALL_ROOT"]
            assert values["SIQAD_PLUGINS_ROOT"].startswith(str(prefix) + "/")
            assert (prefix / "siqad").exists()
            if index == 2 and os.uname().sysname == "Darwin":
                assert "/stale-sdk" not in values.get("CUPS_INCLUDE_DIR", "")
                assert "/stale-sdk" not in values.get("OPENGL_INCLUDE_DIR", "")
            results = build / "test-results"
            if results.exists(): shutil.copytree(results, args.artifacts / f"{index}-results")
        print("Release/notest → Debug/tests → unchanged Debug with stale cache: passed")


if __name__ == "__main__":
    main()
