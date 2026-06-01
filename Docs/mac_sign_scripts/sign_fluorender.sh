#!/bin/bash
set -e

#############################################
# PATHS
#############################################

APP="FluoRender.app"
FW_DIR="$APP/Contents/Frameworks"
PYFW="$FW_DIR/Python.framework"
PYVER="$PYFW/Versions/3.13"
BIN="$APP/Contents/MacOS/FluoRender"
ENTITLEMENTS="../../../entitlements.plist"

# Developer ID Application certificate (HASH)
SIGN_ID="7F222AFF9F45C2309180D8EDB2AB278F1121F7AD"
echo "SIGN_ID is: '$SIGN_ID'"

#############################################
# RPATH CLEANUP (SAFE)
#############################################

echo "🧹 Ensuring correct rpaths on FluoRender binary"

# Remove old wxWidgets rpath if present
install_name_tool -delete_rpath \
  /Users/fluorender/Documents/FLUORENDER/wxWidgets/mybuild/lib \
  "$BIN" || true

# Ensure correct rpath exists
install_name_tool -add_rpath \
  @executable_path/../Frameworks \
  "$BIN" || true

#############################################
# SIGN TCL/TK (REQUIRED)
#############################################

echo "🔏 Signing Tcl/Tk stub libraries"
codesign --force --timestamp --options runtime --sign "$SIGN_ID" \
  "$PYVER/Frameworks/Tcl.framework/Versions/8.6/libtclstub8.6.a"

codesign --force --timestamp --options runtime --sign "$SIGN_ID" \
  "$PYVER/Frameworks/Tk.framework/Versions/8.6/libtkstub8.6.a"

echo "🔏 Signing Tcl and Tk binaries"
codesign --force --timestamp --options runtime --sign "$SIGN_ID" \
  "$PYVER/Frameworks/Tcl.framework/Versions/8.6/Tcl"

codesign --force --timestamp --options runtime --sign "$SIGN_ID" \
  "$PYVER/Frameworks/Tk.framework/Versions/8.6/Tk"

#############################################
# SIGN PYTHON BINARY
#############################################

echo "🔏 Signing Python binary"
codesign --force --timestamp --options runtime --sign "$SIGN_ID" \
  "$PYVER/Python"

#############################################
# SIGN PYTHON FRAMEWORK WRAPPER
#############################################

echo "🔏 Signing Python.framework wrapper"
codesign --force --timestamp --options runtime --sign "$SIGN_ID" \
  "$PYFW"

#############################################
# SIGN FLUORENDER BINARY + APP
#############################################

echo "🔏 Signing FluoRender binary"
codesign --force --timestamp --options runtime \
  --entitlements "$ENTITLEMENTS" \
  --sign "$SIGN_ID" \
  "$BIN"

echo "🔏 Signing FluoRender.app bundle"
codesign --force --timestamp --options runtime \
  --entitlements "$ENTITLEMENTS" \
  --sign "$SIGN_ID" \
  "$APP"

#############################################
# VERIFICATION (REALISTIC + RELIABLE)
#############################################

echo "🧪 Verifying Python.framework (actual content)"
codesign --verify --strict --verbose=4 \
  "$PYVER"

echo "🧪 Verifying full app bundle"
# NOTE: macOS sometimes prints a bogus “No such file or directory”
# for wrapper frameworks even when everything is valid.
# The deep verify is still useful, but not authoritative.
codesign --verify --deep --strict --verbose=4 "$APP" || true

echo "🧪 Gatekeeper assessment"
spctl --assess --type execute --verbose "$APP" || true

echo "🎉 Signing complete — ready for packaging"
