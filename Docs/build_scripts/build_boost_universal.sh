#!/bin/bash

BOOST_SRC_DIR="$(pwd)"
BUILD_DIR="$BOOST_SRC_DIR/build"
STAGE_DIR="$BOOST_SRC_DIR/stage"
UNIVERSAL_DIR="$BOOST_SRC_DIR/universal"
ARCHS=("x86_64" "arm64")

# Clean old builds
rm -rf "$BUILD_DIR" "$STAGE_DIR" "$UNIVERSAL_DIR"
mkdir -p "$UNIVERSAL_DIR"

# Loop through architectures
for ARCH in "${ARCHS[@]}"; do
    echo "🧱 Building Boost for $ARCH..."
    ./b2 toolset=clang-darwin architecture=$ARCH address-model=64 link=static \
        cxxflags="-arch $ARCH" cflags="-arch $ARCH" linkflags="-arch $ARCH" \
        --build-dir="$BUILD_DIR/$ARCH" --stagedir="$STAGE_DIR/$ARCH" -a
done

# Merge static libs with lipo
echo "🔗 Merging static libraries..."
for LIB in "$STAGE_DIR/arm64"/lib*.a; do
    BASE=$(basename "$LIB")
    lipo -create \
        "$STAGE_DIR/arm64/$BASE" \
        "$STAGE_DIR/x86_64/$BASE" \
        -output "$UNIVERSAL_DIR/$BASE"
done

echo "✅ Universal static Boost libraries are ready in: $UNIVERSAL_DIR"
