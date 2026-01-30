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
ENTITLEMENTS="../../../entitlements.plist"
SIGN_ID="Developer ID Application: SCIENTIFIC COMPUTING AND IMAGING INSTITUTE (SCI INSTITUTE) (WG354P62PL)"

echo "🧹 Cleaning stale rpaths from FluoRender binary"

# Remove the wxWidgets build directory rpath if present
install_name_tool -delete_rpath \
  /Users/fluorender/Documents/FLUORENDER/wxWidgets/mybuild/lib \
  "$BIN" || true

# Ensure correct bundle rpath exists
install_name_tool -add_rpath \
  @executable_path/../Frameworks \
  "$BIN" || true

echo "🔏 Signing Tcl/Tk stub libraries"

codesign --force --timestamp --options runtime \
  --entitlements "$ENTITLEMENTS" \
  --sign "$SIGN_ID" \
  "$PYVER/Frameworks/Tcl.framework/Versions/8.6/libtclstub8.6.a"

codesign --force --timestamp --options runtime \
  --entitlements "$ENTITLEMENTS" \
  --sign "$SIGN_ID" \
  "$PYVER/Frameworks/Tk.framework/Versions/8.6/libtkstub8.6.a"

echo "🔏 Signing Tcl and Tk binaries"

codesign --force --timestamp --options runtime \
  --entitlements "$ENTITLEMENTS" \
  --sign "$SIGN_ID" \
  "$PYVER/Frameworks/Tcl.framework/Versions/8.6/Tcl"

codesign --force --timestamp --options runtime \
  --entitlements "$ENTITLEMENTS" \
  --sign "$SIGN_ID" \
  "$PYVER/Frameworks/Tk.framework/Versions/8.6/Tk"

echo "🔏 Signing Python binary"

codesign --force --timestamp --options runtime \
  --entitlements "$ENTITLEMENTS" \
  --sign "$SIGN_ID" \
  "$PYVER/Python"

echo "🔏 Signing Python.framework wrapper"

codesign --force --timestamp --options runtime \
  --entitlements "$ENTITLEMENTS" \
  --sign "$SIGN_ID" \
  "$PYFW"

echo "🔏 Signing FluoRender binary"

codesign --force --timestamp --options runtime \
  --entitlements "$ENTITLEMENTS" \
  --sign "$SIGN_ID" \
  "$BIN"

echo "🔏 Deep-signing entire app bundle"

codesign --force --timestamp --options runtime --deep \
  --entitlements "$ENTITLEMENTS" \
  --sign "$SIGN_ID" \
  "$APP"

echo "🧪 Verifying signature"

codesign --verify --deep --strict --verbose=4 "$APP"
spctl --assess --type execute --verbose "$APP"

echo "🎉 Signing complete — ready for packaging and notarization"
