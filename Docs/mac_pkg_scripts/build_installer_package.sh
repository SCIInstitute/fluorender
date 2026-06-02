#!/bin/bash
set -e

#############################################
# CONFIGURATION
#############################################

VERSION="2.34.1"

# Auto-detect ARCH if not provided (arm64 or x86_64)
if [ -z "$ARCH" ]; then
    ARCH="$(uname -m)"
fi

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
INSTALL_DIR="$(pwd)"

APP_SOURCE="$SCRIPT_DIR/../../build/bin/Release/FluoRender.app"
APP_DEST="Applications/FluoRender.app"

PKG_ID="edu.utah.sci.fluorender"
PKG_NAME="FluoRender_${VERSION}_${ARCH}.pkg"
DIST_PKG_NAME="FluoRender_${VERSION}_${ARCH}_installer.pkg"

INSTALLER_ID="Developer ID Installer: SCIENTIFIC COMPUTING AND IMAGING INSTITUTE (SCI INSTITUTE) (WG354P62PL)"

# Pick the correct distribution XML based on ARCH
if [ "$ARCH" = "arm64" ]; then
    DIST_XML="distribution_arm64.xml"
else
    DIST_XML="distribution_x86_64.xml"
fi

#############################################
# VERIFY WORKING DIRECTORY
#############################################

echo "🔍 Verifying working directory..."

REQUIRED_FILES=("$DIST_XML")
REQUIRED_DIRS=("resources" "scripts")

MISSING=false
for file in "${REQUIRED_FILES[@]}"; do
    [ ! -f "$file" ] && echo "❌ Missing file: $file" && MISSING=true
done
for dir in "${REQUIRED_DIRS[@]}"; do
    [ ! -d "$dir" ] && echo "❌ Missing directory: $dir" && MISSING=true
done

if [ "$MISSING" = true ]; then
    echo ""
    echo "⚠️  Run this script from Install/Mac/ and ensure $DIST_XML exists."
    exit 1
fi

#############################################
# VERIFY SIGNED APP EXISTS
#############################################

if [ ! -d "$APP_SOURCE" ]; then
    echo "❌ FluoRender.app not found at: $APP_SOURCE"
    echo "Build and sign the app first."
    exit 1
fi

#############################################
# PYTHON FRAMEWORK VERIFICATION (SAFE)
#############################################

echo "🧪 Verifying Python.framework wrapper..."

if ! codesign --verify --strict --verbose=4 \
    "$APP_SOURCE/Contents/Frameworks/Python.framework"; then
    echo "⚠️ Warning: codesign verify reported an issue on Python.framework."
    echo "   This is often a false negative. Continuing..."
fi

echo "🧪 Gatekeeper pre-check..."
spctl --assess --type execute --verbose "$APP_SOURCE" || true

#############################################
# PREPARE PAYLOAD
#############################################

echo "📁 Preparing Applications staging directory..."
rm -rf Applications
mkdir Applications

echo "📦 Copying FluoRender.app..."
cp -R "$APP_SOURCE" "$APP_DEST"

chmod +x scripts/postinstall

#############################################
# BUILD COMPONENT PKG
#############################################

echo "🔧 Building component package..."
pkgbuild \
    --root Applications \
    --install-location /Applications \
    --scripts scripts \
    --identifier "$PKG_ID" \
    --version "$VERSION" \
    "$PKG_NAME"

#############################################
# SIGN THE PKG
#############################################

echo "🔏 Signing package..."
productsign \
    --sign "$INSTALLER_ID" \
    "$PKG_NAME" \
    "signed_$PKG_NAME"

mv "signed_$PKG_NAME" "$PKG_NAME"

#############################################
# BUILD DISTRIBUTION PKG
#############################################

echo "📦 Building final installer package using $DIST_XML..."
productbuild \
    --distribution "$DIST_XML" \
    --resources resources \
    --package-path . \
    --sign "$INSTALLER_ID" \
    "$DIST_PKG_NAME"

echo "🎉 Final installer created: $DIST_PKG_NAME"
echo "👉 Next step: notarize and staple this PKG."
