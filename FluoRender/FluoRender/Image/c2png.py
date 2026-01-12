#!/usr/bin/env python3

import sys
import re

USAGE = """Usage: c2png input_file.c output_file.png
Convert a C array back into a PNG file.

Arguments:
  input_file.c    The C header file containing the PNG byte array.
  output_file.png The output PNG file to be created.
"""

if len(sys.argv) != 3:
    print(USAGE)
    sys.exit(1)

input_file = sys.argv[1]
output_file = sys.argv[2]

try:
    with open(input_file, 'r') as f:
        c_array_str = f.read()
except Exception as e:
    print(f"Failed to read file {input_file}: {e}")
    sys.exit(1)

def c_array_to_png(c_array_str, output_filename):
    # Extract the array content from the C array string
    match = re.search(r'{(.*?)}', c_array_str, re.DOTALL)
    if not match:
        print("No valid C array found in the input file.")
        sys.exit(1)

    array_content = match.group(1)

    # Convert the array content to a list of integers
    try:
        byte_values = [int(x.strip(), 16) for x in array_content.split(',') if x.strip()]
    except ValueError as e:
        print(f"Error parsing byte values: {e}")
        sys.exit(1)

    # Write the byte values to a PNG file
    with open(output_filename, 'wb') as f:
        f.write(bytearray(byte_values))

c_array_to_png(c_array_str, output_file)
print(f"PNG file has been successfully created: {output_file}")
