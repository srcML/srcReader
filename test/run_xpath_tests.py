#!/usr/bin/env python3
import argparse
import subprocess
import sys
from pathlib import Path
# python3 test/run_xpath_tests.py --binary build/bin/print_xpaths


def read_lines(path: Path) -> list[str]:
    lines = []
    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        lines.append(line)
    return lines


def run_print_xpaths(binary: Path, diff_xml: Path) -> list[str]:
    result = subprocess.run(
        [str(binary), str(diff_xml)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=False,
    )

    if result.returncode != 0:
        raise RuntimeError(
            f"print_xpaths failed for {diff_xml}\n"
            f"exit code: {result.returncode}\n"
            f"stderr:\n{result.stderr}"
        )

    return [line.strip() for line in result.stdout.splitlines() if line.strip()]


def verify_resolution(diff_xml: Path, paths: list[str]) -> None:
    """Require each generated locator to identify its own input XML element."""
    from lxml import etree
    src = "http://www.srcML.org/srcML/src"
    tree = etree.parse(str(diff_xml), etree.XMLParser(resolve_entities=False, no_network=True))
    root = tree.getroot()
    attributes = {etree.QName(name).localname for name in root.attrib}
    archive = etree.QName(root).localname == "unit" and "url" in attributes and "filename" not in attributes
    elements = [node for node in root.iter() if isinstance(node.tag, str)]
    if len(paths) != len(elements):
        raise AssertionError("Generated locator count differs from XML element count")
    for node, path in zip(elements, paths):
        namespaces = {prefix: uri for prefix, uri in node.nsmap.items() if prefix}
        namespaces.update(src=src, diff="http://www.srcML.org/srcDiff", mv="http://www.srcML.org/srcMove")
        expression = ("/src:unit[1]" if archive and node is not root else "") + path
        matches = tree.xpath(expression, namespaces=namespaces)
        if len(matches) != 1 or matches[0] is not node:
            raise AssertionError(f"Locator must identify its own XML element: {path} ({len(matches)} matches)")


def run_case(binary: Path, case_dir: Path, check_resolution: bool = False) -> bool:
    diff_xml = case_dir / "diff.xml"
    must_contain = case_dir / "must_contain.txt"
    must_not_contain = case_dir / "must_not_contain.txt"

    if not diff_xml.exists():
        print(f"[FAIL] {case_dir.name}: missing diff.xml")
        return False

    if not must_contain.exists():
        print(f"[FAIL] {case_dir.name}: missing must_contain.txt")
        return False

    try:
        actual_lines = run_print_xpaths(binary, diff_xml)
        if check_resolution:
            verify_resolution(diff_xml, actual_lines)
    except Exception as exc:
        print(f"[FAIL] {case_dir.name}: {exc}")
        return False

    actual_set = set(actual_lines)
    expected_present = read_lines(must_contain)
    expected_absent = read_lines(must_not_contain) if must_not_contain.exists() else []

    missing = [line for line in expected_present if line not in actual_set]
    unexpected = [line for line in expected_absent if line in actual_set]

    if missing or unexpected:
        print(f"[FAIL] {case_dir.name}")

        if missing:
            print("  Missing expected XPaths:")
            for line in missing:
                print(f"    {line}")

        if unexpected:
            print("  Found forbidden XPaths:")
            for line in unexpected:
                print(f"    {line}")

        return False

    print(f"[PASS] {case_dir.name} ({len(expected_present)} checks)")
    return True


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--binary",
        default="build/bin/print_xpaths",
        help="Path to print_xpaths binary",
    )
    parser.add_argument(
        "--cases",
        default="test/cases",
        help="Directory containing test case subdirectories",
    )
    parser.add_argument("--verify-resolution", action="store_true",
                        help="Resolve every emitted XPath to its own XML element (requires lxml)")
    args = parser.parse_args()

    binary = Path(args.binary)
    cases_dir = Path(args.cases)

    if not binary.exists():
        print(f"Binary not found: {binary}", file=sys.stderr)
        return 2

    if not cases_dir.exists():
        print(f"Cases directory not found: {cases_dir}", file=sys.stderr)
        return 2

    case_dirs = sorted(p for p in cases_dir.iterdir() if p.is_dir())
    if not case_dirs:
        print("No test cases found.", file=sys.stderr)
        return 2

    all_passed = True
    for case_dir in case_dirs:
        ok = run_case(binary, case_dir, args.verify_resolution)
        if not ok:
            all_passed = False

    return 0 if all_passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
