#!/bin/bash

set -e

# Define architectures and their Boost equivalents
ARCHS=("x86_64" "arm64")
BOOST_ARCH_MAP=(["x86_64"]="x86" ["arm64"]="arm")
MACOS_SDK=$(xcrun --sdk macosx --show-sdk-path)

# Step 1: Bootstrap
echo "🚀 Bootstrapping Boost..."
./bootstrap.sh

# Step 2: Build Boost for each architecture
for ARCH in "${ARCHS[@]}"; do
    BOOST_ARCH=${BOOST_ARCH_MAP[$ARCH]}
    echo "🔧 Building Boost for ${ARCH} (Boost arch: ${BOOST_ARCH})..."

    BUILD_DIR="build/${ARCH}"
    INSTALL_DIR="stage/${ARCH}"

    rm -rf "$BUILD_DIR" "$INSTALL_DIR"

    ./b2 --build-dir="$BUILD_DIR" --prefix="$INSTALL_DIR" toolset=clang \
        architecture=${BOOST_ARCH} address-model=64 \
        cxxflags="-arch ${ARCH} -isysroot ${MACOS_SDK}" \
        linkflags="-arch ${ARCH} -isysroot ${MACOS_SDK}" \
        install
done

# Step 3: Create universal lib directory
UNIVERSAL_LIB_DIR="stage/universal/lib"
mkdir -p "${UNIVERSAL_LIB_DIR}"

# Step 4: Merge static libraries using lipo
echo "🧬 Creating universal binaries..."
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

echo "🎉 Done! Universal libraries are in ${UNIVERSAL_LIB_DIR}"
