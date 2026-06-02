#!/bin/bash
set -e

# Usage: ARCH=x86_64 ./bundle_python.sh
#     or ARCH=arm64 ./bundle_python.sh

if [ -z "$ARCH" ]; then
    echo "❌ ARCH not set. Use ARCH=x86_64 or ARCH=arm64"
    exit 1
fi

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

echo "📦 Copying Python 3.13 from system framework"
cp -R /Library/Frameworks/Python.framework/Versions/3.13 "$PYFW/Versions/"

echo "🔗 Creating symlinks and top-level files"
ln -s Versions/3.13 "$PYFW/Current"
ln -s Versions/3.13/Headers "$PYFW/Headers"
ln -s Versions/3.13/Resources "$PYFW/Resources"

# Top-level Python and Info.plist MUST be real files for code signing
cp "$PYVER/Python" "$PYFW/Python"
cp "$PYVER/Resources/Info.plist" "$PYFW/Info.plist"

#############################################
# SAFE THINNING / REMOVAL LOGIC
#############################################

echo "🏋️ Processing Mach-O binaries for ARCH=$ARCH"

find "$PYVER" -type f -print0 | while IFS= read -r -d '' f; do
    # Only process Mach-O files
    if file "$f" | grep -q "Mach-O"; then
        
        # Does this file contain the target architecture?
        if lipo -info "$f" 2>/dev/null | grep -q "$ARCH"; then
            echo "  ✔ Thinning to $ARCH: $f"
            lipo -thin "$ARCH" "$f" -output "$f"
        else
            echo "  ❌ Removing non-$ARCH binary: $f"
            rm -f "$f"
        fi
    fi
done

#############################################

echo "🔧 Fixing install_name for FluoRender"
install_name_tool -change \
  /Library/Frameworks/Python.framework/Versions/3.13/Python \
  @rpath/Python.framework/Python \
  "$BIN"

echo "🧹 Removing stale wxWidgets rpath (if present)"
install_name_tool -delete_rpath \
  /Users/fluorender/Documents/FLUORENDER/wxWidgets/mybuild/lib \
  "$BIN" || true

echo "➕ Ensuring correct bundle rpath exists"
install_name_tool -add_rpath \
  @executable_path/../Frameworks \
  "$BIN" || true

echo "🎉 Python.framework bundling complete for ARCH=$ARCH"
