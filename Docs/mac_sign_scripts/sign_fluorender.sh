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
BIN="$APP/Contents/MacOS/FluoRender"
ENTITLEMENTS="../../../entitlements.plist"
SIGN_ID="Developer ID Application: SCIENTIFIC COMPUTING AND IMAGING INSTITUTE (SCI INSTITUTE) (WG354P62PL)"

echo "🧹 Ensuring correct rpaths on FluoRender binary"

install_name_tool -delete_rpath \
  /Users/fluorender/Documents/FLUORENDER/wxWidgets/mybuild/lib \
  "$BIN" || true

install_name_tool -add_rpath \
  @executable_path/../Frameworks \
  "$BIN" || true

echo "🔏 Signing Python.framework (recursively)"
codesign --force --options runtime --timestamp \
  --sign "$SIGN_ID" \
  "$PYFW"

echo "🔏 Signing FluoRender binary"
codesign --force --options runtime --timestamp \
  --entitlements "$ENTITLEMENTS" \
  --sign "$SIGN_ID" \
  "$BIN"

echo "🔏 Signing FluoRender.app bundle"
codesign --force --options runtime --timestamp \
  --entitlements "$ENTITLEMENTS" \
  --sign "$SIGN_ID" \
  "$APP"

echo "🧪 Verifying signature"
codesign --verify --deep --strict --verbose=4 "$APP"
spctl --assess --type execute --verbose "$APP"

echo "🎉 Signing complete — ready for pkg + notarization"
