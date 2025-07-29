#!/bin/bash

set -e

# Define variables
REPO_URL="https://code.videolan.org/videolan/x264.git"
X264_DIR="x264_src"
BUILD_X86_64="x264_x86_64"
BUILD_ARM64="x264_arm"
UNIVERSAL_LIB="libx264_universal.a"

# Clean up any previous builds
rm -rf $X264_DIR $BUILD_X86_64 $BUILD_ARM64 $UNIVERSAL_LIB

# Clone the x264 repository
git clone --depth 1 $REPO_URL $X264_DIR

# Build for x86_64
cp -r $X264_DIR $BUILD_X86_64
pushd $BUILD_X86_64
./configure --host=x86_64-apple-darwin --enable-static --disable-cli --extra-cflags="-arch x86_64" --extra-ldflags="-arch x86_64"
make -j$(sysctl -n hw.logicalcpu)
popd

# Build for arm64
cp -r $X264_DIR $BUILD_ARM64
pushd $BUILD_ARM64
./configure --host=aarch64-apple-darwin --enable-static --disable-cli --extra-cflags="-arch arm64" --extra-ldflags="-arch arm64" --disable-asm
make -j$(sysctl -n hw.logicalcpu)
popd

# Combine the static libraries into a universal binary
lipo -create   $BUILD_X86_64/libx264.a   $BUILD_ARM64/libx264.a   -output $UNIVERSAL_LIB

# Verify the result
lipo -info $UNIVERSAL_LIB
