#!/usr/bin/env python3
"""Write the binary fixtures the documentation examples read.

The README's read example opens a file the way a consumer would, so the file
has to exist before it runs. Generating it here keeps the shown example a pitch
rather than a setup script.

Run in the directory the tests run in; CMake registers this as a CTest fixture.
"""

from pathlib import Path

# firmware_image: "FW", version 1, payload_length 4, then the payload.
FIRMWARE = bytes([
    0x46, 0x57,
    0x00, 0x01,
    0x00, 0x00, 0x00, 0x04,
    0xde, 0xad, 0xbe, 0xef,
])


def main() -> int:
    Path("firmware.bin").write_bytes(FIRMWARE)
    print(f"doc fixtures: firmware.bin ({len(FIRMWARE)} bytes)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
