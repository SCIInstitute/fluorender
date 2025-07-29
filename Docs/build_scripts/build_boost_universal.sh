#!/bin/bash
set -e

echo "🚀 Bootstrapping Boost..."
./bootstrap.sh

MACOS_SDK=$(xcrun --sdk macosx --show-sdk-path)

# Build for x86_64
echo "🔧 Building Boost for x86_64..."
./b2 --build-dir=build/x86_64 --prefix=stage/x86_64 toolset=clang \
    architecture=x86 address-model=64 \
    cxxflags="-arch x86_64 -isysroot ${MACOS_SDK}" \
    linkflags="-arch x86_64 -isysroot ${MACOS_SDK}" \
    link=static runtime-link=static \
    install

# Build for arm64
echo "🔧 Building Boost for arm64..."
./b2 --build-dir=build/arm64 --prefix=stage/arm64 toolset=clang \
    architecture=arm address-model=64 \
    cxxflags="-arch arm64 -isysroot ${MACOS_SDK}" \
    linkflags="-arch arm64 -isysroot ${MACOS_SDK}" \
    link=static runtime-link=static \
    install

# Combine with lipo
echo "🧬 Creating universal binaries..."
UNIVERSAL_LIB_DIR="stage/universal/lib"
mkdir -p "${UNIVERSAL_LIB_DIR}"

for LIB in stage/x86_64/lib/*.a; do
    LIBNAME=$(basename "$LIB")
    if [ -f "stage/arm64/lib/${LIBNAME}" ]; then
        lipo -create \
            "stage/x86_64/lib/${LIBNAME}" \
            "stage/arm64/lib/${LIBNAME}" \
            -output "${UNIVERSAL_LIB_DIR}/${LIBNAME}"
        echo "✅ Created universal ${LIBNAME}"
    else
        echo "⚠️ Skipping ${LIBNAME} (missing in arm64)"
    fi
done

echo "🎉 Done! Universal Boost libraries are in ${UNIVERSAL_LIB_DIR}"
