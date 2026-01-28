#!/bin/bash

set -e

echo "🔍 Verifying working directory..."

# Required files and directories
REQUIRED_FILES=("distribution.xml")
REQUIRED_DIRS=("resources" "scripts")

# Check for required files and directories
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

# Create Applications directory if missing
if [ ! -d "Applications" ]; then
    echo "📁 Creating Applications directory..."
    mkdir Applications
fi

# Copy FluoRender.app from build directory
APP_SOURCE="../../build/bin/Release/FluoRender.app"
APP_DEST="Applications/FluoRender.app"

if [ -d "$APP_SOURCE" ]; then
    echo "📦 Copying FluoRender.app from build directory..."
    cp -R "$APP_SOURCE" "$APP_DEST"
else
    echo "❌ FluoRender.app not found at expected location: $APP_SOURCE"
    echo "Please build and sign the app before running this script."
    exit 1
fi

# Make postinstall script executable
chmod +x scripts/postinstall

# Build component package
echo "🔧 Building component package..."
pkgbuild \
    --root Applications \
    --install-location /Applications \
    --scripts scripts \
    --identifier edu.utah.sci.fluorender \
    --version 2.34 \
    --output FluoRender.pkg

# Build final installer package
echo "📦 Building final installer package..."
productbuild \
    --distribution distribution.xml \
    --resources resources \
    --package-path . \
    --scripts scripts \
    --output FluoRender2.34_mac64.pkg

echo "✅ Installer package created: FluoRender2.34_mac64.pkg"
