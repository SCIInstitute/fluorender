#!/bin/bash
set -e

BIN="FluoRender.app/Contents/MacOS/FluoRender"

echo "Checking LC_RPATH entries..."
otool -l "$BIN" | grep -A2 LC_RPATH

echo
echo "Ensuring no absolute paths remain..."
BAD=$(otool -l "$BIN" | grep -E "path /Users|path /Volumes|path /Library/Frameworks/Python.framework")
if [ -n "$BAD" ]; then
    echo "❌ ERROR: Found forbidden absolute rpaths:"
    echo "$BAD"
    exit 1
fi

echo
echo "Ensuring @rpath resolves only inside the bundle..."
if ! otool -L "$BIN" | grep -q "@rpath/Python.framework/Python"; then
    echo "❌ ERROR: Python.framework is not linked via @rpath"
    exit 1
fi

echo
echo "Ensuring no wxWidgets build paths are embedded..."
if strings "$BIN" | grep -q "wxWidgets/mybuild"; then
    echo "❌ ERROR: Found embedded wxWidgets build path"
    exit 1
fi

echo
echo "Ensuring no Python.framework paths outside the bundle..."
if strings "$BIN" | grep -q "/Library/Frameworks/Python.framework"; then
    echo "❌ ERROR: Found system Python.framework reference"
    exit 1
fi

echo
echo "Ensuring correct bundle rpath exists..."
if ! otool -l "$BIN" | grep -A2 LC_RPATH | grep -q "@executable_path/../Frameworks"; then
    echo "❌ ERROR: Missing bundle Frameworks rpath"
    exit 1
fi

echo
echo "✅ All checks passed — safe to sign."
