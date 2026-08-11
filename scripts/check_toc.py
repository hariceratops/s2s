#!/usr/bin/env python3
"""Check UserGuide.md's table of contents against its actual headings.

Every heading must have a ToC entry and every ToC entry must resolve to a
heading, with GitHub's anchor rules (lowercase, spaces to hyphens, punctuation
dropped, duplicates suffixed -1, -2, ...).
"""
import re
import sys
from pathlib import Path

HEADING = re.compile(r"^(#{2,4})\s+(.*)$")
TOC_LINK = re.compile(r"^\s*-\s+\[(.+?)\]\(#(.+?)\)\s*$")


def anchor(text: str, seen: dict[str, int]) -> str:
    a = text.lower()
    a = a.replace("`", "")
    a = re.sub(r"[^\w\s-]", "", a)      # drop punctuation, keep word chars/space/hyphen
    a = a.strip().replace(" ", "-")
    n = seen.get(a, 0)
    seen[a] = n + 1
    return a if n == 0 else f"{a}-{n}"


def main() -> int:
    md = Path(sys.argv[1] if len(sys.argv) > 1 else "UserGuide.md")
    lines = md.read_text().splitlines()

    seen: dict[str, int] = {}
    headings: list[tuple[str, str]] = []
    for line in lines:
        m = HEADING.match(line)
        if m:
            headings.append((m.group(2).strip(), anchor(m.group(2).strip(), seen)))

    toc: list[tuple[str, str]] = []
    for line in lines:
        m = TOC_LINK.match(line)
        if m:
            toc.append((m.group(1).strip(), m.group(2).strip()))

    problems = []

    # Compare as ordered sequences: the ToC must list every heading, in order,
    # with the anchor that heading actually generates. Set comparison would let
    # two same-named headings swap anchors and still pass.
    if len(toc) != len(headings):
        problems.append(
            f"{len(headings)} headings but {len(toc)} ToC entries"
        )

    for i, (h_text, h_anchor) in enumerate(headings):
        if i >= len(toc):
            problems.append(f"heading '{h_text}' (#{h_anchor}) has no ToC entry")
            continue
        t_text, t_anchor = toc[i]
        if t_anchor != h_anchor:
            problems.append(
                f"ToC entry {i + 1} '{t_text}' points at #{t_anchor}, "
                f"but heading {i + 1} '{h_text}' generates #{h_anchor}"
            )

    for p in problems:
        print(f"toc: {p}", file=sys.stderr)
    if problems:
        print(f"\n{len(problems)} table-of-contents problem(s).", file=sys.stderr)
        return 1
    print(f"toc: {len(headings)} headings, all matched")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
