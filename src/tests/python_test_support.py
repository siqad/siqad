"""Standard-library unittest entry point with JUnit output for CTest/CI."""
import argparse
from pathlib import Path
import time
import unittest
import xml.etree.ElementTree as ET


def run(test_class):
    parser = argparse.ArgumentParser()
    parser.add_argument("--junit", type=Path)
    args = parser.parse_args()
    started = time.monotonic()
    suite = unittest.defaultTestLoader.loadTestsFromTestCase(test_class)
    cases = list(suite)
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    if args.junit:
        root = ET.Element("testsuite", name=test_class.__name__, tests=str(result.testsRun),
                          failures=str(len(result.failures)), errors=str(len(result.errors)),
                          time=str(time.monotonic() - started))
        failures = dict(result.failures); errors = dict(result.errors); skips = dict(result.skipped)
        for test in cases:
            case = ET.SubElement(root, "testcase", name=test.id())
            if test in failures: ET.SubElement(case, "failure").text = failures[test]
            if test in errors: ET.SubElement(case, "error").text = errors[test]
            if test in skips: ET.SubElement(case, "skipped", message=skips[test])
        args.junit.parent.mkdir(parents=True, exist_ok=True)
        ET.ElementTree(root).write(args.junit, encoding="utf-8", xml_declaration=True)
    raise SystemExit(not result.wasSuccessful())
