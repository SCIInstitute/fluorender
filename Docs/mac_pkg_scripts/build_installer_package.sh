#!/bin/bash
set -e

#############################################
# CONFIGURATION
#############################################

VERSION="2.34.1"
ARCH="${ARCH:-x86_64}"   # or arm64

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
INSTALL_DIR="$(pwd)"

APP_SOURCE="$SCRIPT_DIR/../../build/bin/Release/FluoRender.app"
APP_DEST="Applications/FluoRender.app"

PKG_ID="edu.utah.sci.fluorender"
PKG_NAME="FluoRender_${VERSION}_${ARCH}.pkg"
DIST_PKG_NAME="FluoRender_${VERSION}_${ARCH}_installer.pkg"

INSTALLER_ID="Developer ID Installer: SCIENTIFIC COMPUTING AND IMAGING INSTITUTE (SCI INSTITUTE) (WG354P62PL)"

#############################################
# VERIFY WORKING DIRECTORY
#############################################

echo "🔍 Verifying working directory..."

REQUIRED_FILES=("distribution.xml")
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
    echo "⚠️  Run this script from Install/Mac/"
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

echo "🧪 Verifying Python.framework (actual content)..."
if ! codesign --verify --strict --verbose=4 \
    "$APP_SOURCE/Contents/Frameworks/Python.framework/Versions/3.13"; then
    echo "❌ Python.framework is NOT properly signed."
    exit 1
fi

echo "🧪 Gatekeeper pre-check..."
# This will say "rejected" until notarization — that's OK.
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

echo "📦 Building final installer package..."
productbuild \
    --distribution distribution.xml \
    --resources resources \
    --package-path . \
    --sign "$INSTALLER_ID" \
    "$DIST_PKG_NAME"

echo "🎉 Final installer created: $DIST_PKG_NAME"
echo "👉 Next step: notarize and staple this PKG."
