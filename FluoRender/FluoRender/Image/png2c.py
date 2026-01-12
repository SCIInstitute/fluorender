#!/usr/bin/env python3

import sys
import os
import re
import array

USAGE = """Usage: png2c [-s] [file...]
Output input PNG files as C arrays to standard output. Used to embed PNG images
in C code (like XPM but with full alpha channel support).

  -s    embed the image size in the image names in generated code."""

if len(sys.argv) < 2:
    print(USAGE)
    sys.exit(1)

r = re.compile(r"^([a-zA-Z._][a-zA-Z._0-9]*)[.][pP][nN][gG]$")

with_size = False
size_suffix = ''
for path in sys.argv[1:]:
    if path == '-s':
        with_size = True
        continue

    filename = os.path.basename(path).replace('-', '_')
    m = r.match(filename)

    if not m:
        print(f"Skipped file (unsuitable filename): {filename}")
        continue

    try:
        with open(path, "rb") as f:
            bytes_arr = array.array('B', f.read())
    except Exception as e:
        print(f"Failed to read file {path}: {e}")
        continue

    count = len(bytes_arr)

    # Check PNG signature
    if bytes_arr[:16].tobytes() != b'\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR':
        print(f'"{filename}" doesn\'t seem to be a valid PNG file.')
        continue

    if with_size:
        def getInt(start):
            return (bytes_arr[start] << 24) + (bytes_arr[start+1] << 16) + \
                   (bytes_arr[start+2] << 8) + bytes_arr[start+3]

        size_suffix = f"_{getInt(16)}x{getInt(20)}"

    text = f"/* {filename} - {count} bytes */\n"
    text += f"static const unsigned char {m.group(1)}{size_suffix}_png[] = {{\n"

    for i, byte in enumerate(bytes_arr):
        if i % 8 == 0:
            text += "  "
        text += f"0x{byte:02x}"
        if i < count - 1:
            text += ", "
        if i % 8 == 7:
            text += "\n"

    text += "\n};\n\n"
    print(text)
