"""Portable script contracts, with isolated fake build tools and no real installs."""
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

SOURCE = Path(__file__).resolve().parents[2] / "make_everything_dev"


class DeveloperScriptTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix="siqad-script-")
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name) / "checkout with spaces"
        self.root.mkdir()
        self.script = self.root / "make_everything_dev"
        shutil.copy2(SOURCE, self.script)
        self.tools = Path(self.tmp.name) / "tools"
        self.tools.mkdir()
        self.log = Path(self.tmp.name) / "commands.jsonl"
        self.env = dict(os.environ, PATH=str(self.tools) + os.pathsep + os.environ["PATH"],
                        CMAKE_COMMAND=(self.tools / "cmake").as_posix(), CTEST_COMMAND=(self.tools / "cmake").as_posix(), JOBS="3", SDKROOT="/selected SDK")
        for key in ("FOR_OS", "CMAKE_FLAGS", "APPEND_TO_PATH", "QMAKE_COMMAND", "FAIL_STAGE"):
            self.env.pop(key, None)
        # bash is a documented prerequisite; use an absolute interpreter in fake tools.
        import sys
        fake = ("#!/usr/bin/env python\n" if os.name == "nt" else "#!" + sys.executable + "\n") + (
            "import json, os, sys\nfrom pathlib import Path\n"
            f"with open({str(self.log)!r}, 'a') as f: f.write(json.dumps(sys.argv[1:])+'\\n')\n"
            "if '-B' in sys.argv:\n"
            "    build = Path(sys.argv[sys.argv.index('-B')+1]); build.mkdir(parents=True, exist_ok=True)\n"
            "    opts = [x.split('=',1)[1] for x in sys.argv if x.startswith('-DBUILD_TESTING=')]\n"
            "    (build/'CMakeCache.txt').write_text('BUILD_TESTING:BOOL='+opts[-1]+'\\n')\n"
            "sys.exit(7 if os.environ.get('FAIL_STAGE') in sys.argv[1:] else 0)\n"
        )
        (self.tools / "cmake").write_text(fake)
        (self.tools / "cmake").chmod(0o755)

    def run_script(self, *args, success=True):
        bash = "bash"
        if os.name == "nt":
            git_bash = Path(os.environ.get("ProgramFiles", "C:/Program Files")) / "Git/bin/bash.exe"
            if git_bash.exists(): bash = str(git_bash)
        proc = subprocess.run([bash, self.script.as_posix(), *args], cwd=self.tmp.name,
                              env=self.env, capture_output=True, text=True, timeout=15)
        if success:
            self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)
        return proc

    def calls(self):
        return [json.loads(line) for line in self.log.read_text().splitlines()]

    def test_outside_checkout_and_spaced_prefix(self):
        prefix = (self.root / "custom install").as_posix()
        self.run_script("release", "--", "-G", "Ninja", "-DCMAKE_INSTALL_PREFIX=" + prefix)
        calls = self.calls()
        self.assertEqual(Path(calls[0][calls[0].index("-S") + 1]), self.root)
        self.assertIn("-DCMAKE_INSTALL_PREFIX=" + prefix, calls[0])
        self.assertIn("Ninja", calls[0])
        self.assertEqual(calls[1][-2:], ["--parallel", "3"])
        self.assertEqual(calls[-1][0], "--install")
        self.assertTrue(any(arg == "--no-tests=error" for call in calls for arg in call))

    def test_modes_and_test_reenable(self):
        self.run_script("release", "notest")
        self.assertIn("-DBUILD_TESTING=OFF", self.calls()[0])
        self.assertFalse(any(call[0] == "--test-dir" for call in self.calls()))
        self.log.unlink()
        self.run_script("debug")
        self.assertIn("-DBUILD_TESTING=ON", self.calls()[0])
        self.assertIn("-DCMAKE_BUILD_TYPE=Debug", self.calls()[0])
        self.assertTrue(any(call[0] == "--test-dir" for call in self.calls()))

    def test_multiline_flags_and_legacy_override(self):
        self.env["CMAKE_FLAGS"] = "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON\n-DBUILD_TEST=OFF"
        self.run_script()
        configure = self.calls()[0]
        self.assertIn("-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", configure)
        self.assertEqual([x for x in configure if x.startswith("-DBUILD_TESTING=")][-1], "-DBUILD_TESTING=OFF")
        self.assertFalse(any(call[0] == "--test-dir" for call in self.calls()))

    def test_errors_and_fail_fast(self):
        for jobs in ("0", "no", "-1"):
            self.env["JOBS"] = jobs
            self.assertEqual(self.run_script(success=False).returncode, 2)
        self.env["JOBS"] = "2"
        self.assertEqual(self.run_script("unknown", success=False).returncode, 2)
        self.env["FAIL_STAGE"] = "--build"
        self.assertEqual(self.run_script(success=False).returncode, 7)
        self.assertEqual(len(self.calls()), 2)
        self.log.unlink()
        self.env["FAIL_STAGE"] = "--test-dir"
        self.assertEqual(self.run_script(success=False).returncode, 7)
        self.assertFalse(any(call[0] == "--install" for call in self.calls()))

    def test_typed_legacy_skip_flags(self):
        for value, expected in (("true", "OFF"), ("no", "ON")):
            self.run_script("--", "-DSKIP_SIQAD_TESTS:BOOL=" + value)
            self.assertEqual([x for x in self.calls()[0] if x.startswith("-DBUILD_TESTING=")][-1],
                             "-DBUILD_TESTING=" + expected)
            self.log.unlink()

    def test_sdk_cache_reset_only_on_macos(self):
        for system in ("Linux", "Darwin"):
            for name, result in (("uname", system), ("sysctl", "4"), ("getconf", "4")):
                path = self.tools / name
                path.write_text("#!/bin/sh\nprintf '%s\\n' '" + result + "'\n")
                path.chmod(0o755)
            self.run_script("notest")
            configure = self.calls()[0]
            self.assertEqual("-UCUPS_*" in configure, system == "Darwin")
            self.assertEqual("-DCMAKE_OSX_SYSROOT=/selected SDK" in configure, system == "Darwin")
            self.assertIn("SIQAD_INSTALL_ROOT", configure)
            self.log.unlink()

    def test_cmake_option_precedence_and_root_discovery(self):
        # Exercise the real policy in a tiny project without Qt/plugin dependencies.
        policy = SOURCE.parent / "cmake/Testing.cmake"
        (self.root / "CMakeLists.txt").write_text(
            'cmake_minimum_required(VERSION 3.20)\nproject(TestPolicy NONE)\n'
            + 'include("' + policy.as_posix() + '")\n'
            + 'if(BUILD_TESTING)\nadd_test(NAME sentinel COMMAND "${CMAKE_COMMAND}" -E true)\nendif()\n')
        for index, (flags, enabled) in enumerate((
                ([], True), (["-DBUILD_TEST=OFF"], False),
                (["-DSKIP_SIQAD_TESTS=ON"], False),
                (["-DBUILD_TEST=OFF", "-DBUILD_TESTING=ON"], True),
                (["-DSKIP_SIQAD_TESTS=ON", "-DBUILD_TESTING=ON"], True))):
            build = self.root / ("policy-" + str(index))
            subprocess.run(["cmake", "-S", str(self.root), "-B", str(build), *flags],
                           capture_output=True, check=True, timeout=10)
            result = subprocess.run(["ctest", "--test-dir", str(build), "-C", "Debug", "--no-tests=error"],
                                    capture_output=True, timeout=10)
            self.assertEqual(result.returncode == 0, enabled, result.stdout + result.stderr)


if __name__ == "__main__":
    from python_test_support import run
    run(DeveloperScriptTests)
