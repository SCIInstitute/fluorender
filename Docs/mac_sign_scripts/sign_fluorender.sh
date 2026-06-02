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

#############################################
# AUTO-DETECT SIGNING IDENTITY
#############################################

# Prefer Developer ID Application certificate
SIGN_ID=$(security find-identity -p codesigning -v \
    | grep "Developer ID Application" \
    | head -n 1 \
    | sed -E 's/^[[:space:]]*[0-9]+\) ([A-F0-9]+) .*/\1/')

if [ -z "$SIGN_ID" ]; then
    echo "❌ ERROR: No 'Developer ID Application' certificate found in keychain."
    echo "Install it or run: security find-identity -p codesigning -v"
    exit 1
fi

echo "🔐 Using Developer ID Application identity: $SIGN_ID"

#############################################
# RPATH CLEANUP (SAFE + TOLERANT)
#############################################

echo "🧹 Ensuring correct rpaths on FluoRender binary"

# Remove ANY wxWidgets rpath (Intel or ARM)
install_name_tool -delete_rpath \
  /Users/fluorender/Documents/FLUORENDER/wxWidgets/mybuild/lib \
  "$BIN" || true

install_name_tool -delete_rpath \
  /Users/basisunus/Documents/PROJECTS/FLUORENDER/wxWidgets/mybuild/lib \
  "$BIN" || true

# Ensure correct bundle rpath exists
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
# SIGN PYTHON BINARY + FRAMEWORK
#############################################

echo "🔏 Signing Python binary"
codesign --force --timestamp --options runtime --sign "$SIGN_ID" \
  "$PYVER/Python"

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
# VERIFICATION
#############################################

echo "🧪 Verifying Python.framework"
codesign --verify --strict --verbose=4 "$PYVER"

echo "🧪 Verifying full app bundle"
codesign --verify --deep --strict --verbose=4 "$APP" || true

echo "🧪 Gatekeeper assessment"
spctl --assess --type execute --verbose "$APP" || true

echo "🎉 Signing complete — ready for packaging"
