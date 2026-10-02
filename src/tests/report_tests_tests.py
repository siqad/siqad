import tempfile
from pathlib import Path
import unittest
from report_tests import summarize


class ReportTests(unittest.TestCase):
    def test_missing_or_empty_reports_fail(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            self.assertEqual(summarize(root, ["missing"])[1], 1)
            (root / "empty.xml").write_text("<testsuite/>")
            self.assertEqual(summarize(root, ["empty"])[1], 1)

    def test_failures_and_skips_remain_distinct(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "suite.xml").write_text('''<testsuite>
                <testcase name="pass"/><testcase name="fail"><failure/></testcase>
                <testcase name="native"><skipped message="requires macOS"/></testcase>
                </testsuite>''')
            report, failures = summarize(root, ["suite"])
            self.assertEqual(failures, 1)
            self.assertIn("requires macOS", report)
            self.assertIn("| suite | failed | 3 | 1 | 1 |", report)


if __name__ == "__main__":
    from python_test_support import run
    run(ReportTests)
