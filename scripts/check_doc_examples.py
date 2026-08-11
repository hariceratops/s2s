#!/usr/bin/env python3
"""Check that every bound doc example matches the source that compiles it.

A fenced block in a markdown file is bound to a source file by an HTML comment
on the line directly above the fence:

    <!-- docs: test/single_header/foo.cpp -->
    ```cpp
    ...
    ```

The source file marks the region that must appear in the doc with
`// docs-begin` and `// docs-end` lines. Both must agree exactly.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

BINDING = re.compile(r"^<!--\s*docs:\s*(\S+)\s*-->\s*$")
FENCE = re.compile(r"^```")

# A block is a complete program, rather than a signature or declaration sketch,
# when it defines an entry point. Those must be bound to a source that compiles;
# alias lists and API-shape snippets are exempt.
ENTRY_POINT = re.compile(r"\bmain\s*\(")


def doc_blocks(md: Path) -> list[tuple[int, str | None, list[str]]]:
    """Yield (line_no, bound_path_or_None, block_lines) for each fenced block."""
    lines = md.read_text().splitlines()
    blocks: list[tuple[int, str | None, list[str]]] = []
    i = 0
    while i < len(lines):
        if FENCE.match(lines[i]):
            start = i
            binding = None
            if i > 0:
                m = BINDING.match(lines[i - 1])
                if m:
                    binding = m.group(1)
            i += 1
            body: list[str] = []
            while i < len(lines) and not FENCE.match(lines[i]):
                body.append(lines[i])
                i += 1
            blocks.append((start + 1, binding, body))
        i += 1
    return blocks


def source_region(src: Path) -> list[str] | None:
    lines = src.read_text().splitlines()
    try:
        begin = next(i for i, l in enumerate(lines) if l.strip() == "// docs-begin")
        end = next(i for i, l in enumerate(lines) if l.strip() == "// docs-end")
    except StopIteration:
        return None
    return lines[begin + 1 : end]


def main(argv: list[str]) -> int:
    root = Path(argv[1]) if len(argv) > 1 else Path.cwd()
    failures: list[str] = []

    for name in ("README.md", "UserGuide.md"):
        md = root / name
        if not md.exists():
            continue
        for line_no, binding, body in doc_blocks(md):
            if binding is None:
                if any(ENTRY_POINT.search(l) for l in body):
                    failures.append(
                        f"{name}:{line_no}: complete program has no "
                        f"'<!-- docs: <path> -->' binding. Back it with a source file "
                        f"under test/single_header/, register that file in "
                        f"test/single_header/CMakeLists.txt, and bind the block to it."
                    )
                continue

            src = root / binding
            if not src.exists():
                failures.append(
                    f"{name}:{line_no}: bound to '{binding}', which does not exist."
                )
                continue

            region = source_region(src)
            if region is None:
                failures.append(
                    f"{name}:{line_no}: '{binding}' has no "
                    f"'// docs-begin' / '// docs-end' markers."
                )
                continue

            if region != body:
                failures.append(
                    f"{name}:{line_no}: block has drifted from '{binding}'.\n"
                    + _first_difference(body, region, binding, name)
                )

    for f in failures:
        print(f"doc example check: {f}", file=sys.stderr)

    if failures:
        print(
            f"\n{len(failures)} documentation example problem(s).",
            file=sys.stderr,
        )
        return 1

    print("doc examples: all bound blocks match their sources")
    return 0


def _first_difference(doc: list[str], src: list[str], binding: str, name: str) -> str:
    for i in range(max(len(doc), len(src))):
        d = doc[i] if i < len(doc) else "<end of block>"
        s = src[i] if i < len(src) else "<end of region>"
        if d != s:
            return (
                f"    first difference at block line {i + 1}:\n"
                f"      {name}: {d!r}\n"
                f"      {binding}: {s!r}"
            )
    return "    (blocks differ in trailing whitespace only)"


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
