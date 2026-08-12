#!/usr/bin/env python3
"""Write the binary file hello_s2s reads.

The example opens a file the way a consumer would, so the file has to exist
before it runs. CMake registers this as a CTest fixture. Same arrangement as
test/doc_examples/make_doc_fixtures.py.
"""

from pathlib import Path

TEXT = b"hello, s2s"
SAMPLE = len(TEXT).to_bytes(8, "little") + TEXT


def main() -> int:
    Path("sample.bin").write_bytes(SAMPLE)
    print(f"example fixture: sample.bin ({len(SAMPLE)} bytes)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
