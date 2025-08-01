#!/bin/bash

set -e

# === CONFIGURATION ===
APP_NAME="FluoRender.app"
APP_ZIP="FluoRender.zip"
PKG_NAME="FluoRender.pkg"
FINAL_PKG="FluoRender2.33_mac64.pkg"
APP_SOURCE="../build/bin/Release/$APP_NAME"
APP_DEST="Applications/$APP_NAME"
NOTARY_PROFILE="your-notarytool-profile"  # Fill this in
APP_BUNDLE_ID="edu.utah.sci.fluorender"  # Adjust if needed

# === CHECK WORKING DIRECTORY ===
echo "🔍 Verifying working directory..."

REQUIRED_FILES=("distribution.xml")
REQUIRED_DIRS=("resources" "scripts")

MISSING=false
for file in "${REQUIRED_FILES[@]}"; do
    if [ ! -f "$file" ]; then
        echo "❌ Missing required file: $file"
        MISSING=true
    fi
done

for dir in "${REQUIRED_DIRS[@]}"; do
    if [ ! -d "$dir" ]; then
        echo "❌ Missing required directory: $dir"
        MISSING=true
    fi
done

if [ "$MISSING" = true ]; then
    echo ""
    echo "⚠️  Please run this script from the 'Install/Mac/' directory containing:"
    echo "    - distribution.xml"
    echo "    - resources/"
    echo "    - scripts/"
    echo ""
    exit 1
fi

# === PREPARE APP ===
echo "📁 Preparing Applications directory..."
mkdir -p Applications
if [ -d "$APP_SOURCE" ]; then
    echo "📦 Copying FluoRender.app from build directory..."
    cp -R "$APP_SOURCE" "$APP_DEST"
else
    echo "❌ FluoRender.app not found at expected location: $APP_SOURCE"
    exit 1
fi

# === ZIP APP FOR NOTARIZATION ===
echo "🗜️ Zipping app for notarization..."
rm -f "$APP_ZIP"
ditto -c -k --keepParent "$APP_DEST" "$APP_ZIP"

# === SUBMIT APP FOR NOTARIZATION ===
echo "📤 Submitting app for notarization..."
notarytool submit "$APP_ZIP" --keychain-profile "$NOTARY_PROFILE" --wait

# === STAPLE APP ===
echo "📎 Stapling notarized app..."
xcrun stapler staple "$APP_DEST"

# === MAKE POSTINSTALL SCRIPT EXECUTABLE ===
chmod +x scripts/postinstall

# === BUILD COMPONENT PACKAGE ===
echo "🔧 Building component package..."
pkgbuild \
    --root Applications \
    --install-location /Applications \
    --scripts scripts \
    --identifier "$APP_BUNDLE_ID" \
    --version 2.33 \
    --output "$PKG_NAME"

# === SUBMIT PACKAGE FOR NOTARIZATION ===
echo "📤 Submitting installer package for notarization..."
notarytool submit "$PKG_NAME" --keychain-profile "$NOTARY_PROFILE" --wait

# === STAPLE FINAL PACKAGE ===
echo "📎 Stapling notarized installer..."
xcrun stapler staple "$PKG_NAME"

# === BUILD FINAL INSTALLER ===
echo "📦 Building final installer package..."
productbuild \
    --distribution distribution.xml \
    --resources resources \
    --package-path . \
    --scripts scripts \
    --output "$FINAL_PKG"

echo "✅ Final installer created and notarized: $FINAL_PKG"
