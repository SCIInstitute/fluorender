#!/bin/bash
set -e

# Must be run from fluorender/build/bin/Release
if [ ! -d "FluoRender.app" ]; then
    echo "❌ Error: Run this script from fluorender/build/bin/Release"
    exit 1
fi

APP="FluoRender.app"
FW_DIR="$APP/Contents/Frameworks"
PYFW="$FW_DIR/Python.framework"
PYVER="$PYFW/Versions/3.13"
BIN="$APP/Contents/MacOS/FluoRender"

echo "🔧 Cleaning old Python.framework"
rm -rf "$PYFW"
mkdir -p "$PYFW/Versions"

echo "📦 Copying Python 3.13"
cp -R /Library/Frameworks/Python.framework/Versions/3.13 "$PYFW/Versions/"

echo "🔗 Creating symlinks and top-level files"
ln -s Versions/3.13 "$PYFW/Current"
ln -s Versions/3.13/Headers "$PYFW/Headers"
ln -s Versions/3.13/Resources "$PYFW/Resources"

# Top-level Python and Info.plist must be regular files
cp "$PYVER/Python" "$PYFW/Python"
cp "$PYVER/Resources/Info.plist" "$PYFW/Info.plist"

echo "🏋️ Thinning Mach-O binaries to x86_64"
find "$PYVER" -type f -exec sh -c '
    if file "$1" | grep -q "Mach-O"; then
        lipo -thin x86_64 "$1" -output "$1"
    fi
' sh {} \;

echo "🔧 Fixing install_name for FluoRender"
install_name_tool -change \
  /Library/Frameworks/Python.framework/Versions/3.13/Python \
  @rpath/Python.framework/Python \
  "$BIN"

echo "🧹 Removing stale wxWidgets rpath"
install_name_tool -delete_rpath \
  /Users/fluorender/Documents/FLUORENDER/wxWidgets/mybuild/lib \
  "$BIN" || true

echo "➕ Adding correct bundle rpath"
install_name_tool -add_rpath \
  @executable_path/../Frameworks \
  "$BIN" || true

echo "🎉 Python.framework bundling + rpath cleanup complete (Intel-only)"
