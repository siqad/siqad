"""Summarize JUnit cases, making missing evidence and skips visible in CI."""
import argparse
from pathlib import Path
import os
import sys
import xml.etree.ElementTree as ET


def summarize(directory, expected):
    rows, failures, skipped = [], 0, []
    for name in expected:
        file = directory / (name + ".xml")
        if not file.exists():
            rows.append((name, "missing", 0, 0, 0))
            failures += 1
            continue
        try:
            root = ET.parse(file).getroot()
            cases = list(root.iter("testcase"))
            if not cases:
                raise ValueError("No test cases found")
            bad = sum(c.find("failure") is not None or c.find("error") is not None for c in cases)
            skips = [c for c in cases if c.find("skipped") is not None]
            rows.append((name, "failed" if bad else "passed", len(cases), bad, len(skips)))
            failures += bad
            skipped.extend(name + ": " + c.get("name", "unknown") + " — "
                           + (c.find("skipped").get("message") or c.find("skipped").text or "no reason")
                           for c in skips)
        except (ET.ParseError, ValueError) as exc:
            rows.append((name, "invalid: " + str(exc), 0, 0, 0))
            failures += 1
    lines = ["| Suite | Status | Cases | Failures | Skips |", "|---|---|---:|---:|---:|"]
    lines.extend("| " + " | ".join(map(str, row)) + " |" for row in rows)
    if skipped:
        lines += ["", "Skipped cases:"] + ["- " + reason.replace("\n", " ") for reason in skipped]
    return "\n".join(lines) + "\n", failures


if __name__ == "__main__":
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument("directory", type=Path)
    parser.add_argument("--expect", nargs="+", required=True)
    args = parser.parse_args()
    report, failures = summarize(args.directory, args.expect)
    print(report)
    summary = os.environ.get("GITHUB_STEP_SUMMARY")
    if summary:
        with open(summary, "a", encoding="utf-8") as file:
            file.write(report)
    sys.exit(bool(failures))
