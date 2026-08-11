#!/usr/bin/env python3
"""Check that mkdocs.yml's nav lists every page under docs/, and nothing else.

`mkdocs build --strict` already fails on a nav entry pointing at a missing file
and on a broken internal link. What it does not fail on is the opposite: a page
that exists, builds, and is reachable by nobody because no nav entry mentions
it. That is how the previous single-file guide accumulated sections its own
contents page never listed, so it is checked here.

Parsed with a narrow reader rather than a YAML library, to keep the docs build
the only thing that needs mkdocs installed.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

NAV_ENTRY = re.compile(r":\s*([\w./-]+\.md)\s*$")


def nav_pages(mkdocs_yml: Path) -> list[str]:
    lines = mkdocs_yml.read_text().splitlines()
    try:
        start = next(i for i, l in enumerate(lines) if l.rstrip() == "nav:")
    except StopIteration:
        return []

    pages = []
    for line in lines[start + 1 :]:
        # nav ends at the next top-level key
        if line and not line[0].isspace() and not line.lstrip().startswith("#"):
            break
        m = NAV_ENTRY.search(line)
        if m:
            pages.append(m.group(1))
    return pages


def main(argv: list[str]) -> int:
    root = Path(argv[1] if len(argv) > 1 else ".")
    docs = root / "docs"
    mkdocs_yml = root / "docs" / "mkdocs.yml"

    if not mkdocs_yml.exists():
        print("check_nav: no docs/mkdocs.yml", file=sys.stderr)
        return 1

    listed = nav_pages(mkdocs_yml)
    on_disk = sorted(p.relative_to(docs).as_posix() for p in docs.rglob("*.md"))

    problems = []
    for page in on_disk:
        if page not in listed:
            problems.append(
                f"docs/{page} exists but no nav entry points at it — it would "
                f"build and be unreachable"
            )
    for page in listed:
        if page not in on_disk:
            problems.append(f"nav lists {page}, which is not under docs/")

    duplicates = {p for p in listed if listed.count(p) > 1}
    for page in sorted(duplicates):
        problems.append(f"nav lists {page} more than once")

    for p in problems:
        print(f"check_nav: {p}", file=sys.stderr)
    if problems:
        print(f"\n{len(problems)} navigation problem(s).", file=sys.stderr)
        return 1

    print(f"nav: {len(on_disk)} pages, all listed exactly once")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
