#!/bin/bash

set -e

# Define architecture-specific build directories
BUILD_X86=build_x86_64
BUILD_ARM=build_arm64
UNIVERSAL_DIR=ffmpeg_universal/lib

# Create build directories
mkdir -p $BUILD_X86 $BUILD_ARM $UNIVERSAL_DIR

# Set common configuration flags
COMMON_FLAGS="--disable-shared --enable-static --disable-programs --disable-doc --enable-libx264 --enable-libx265 --enable-swresample"

# Build for x86_64
echo "Building FFmpeg for x86_64..."
make distclean || true
./configure $COMMON_FLAGS --arch=x86_64 --target-os=darwin --cc=clang \
  --extra-cflags="-arch x86_64 -I../x264 -I../x265/include" \
  --extra-ldflags="-arch x86_64 ../x264/libx264_universal.a ../x265/libx265_universal.a" \
  --prefix=$(pwd)/$BUILD_X86
make -j$(sysctl -n hw.logicalcpu)
make install

# Build for arm64
echo "Building FFmpeg for arm64..."
make distclean
./configure $COMMON_FLAGS --arch=arm64 --target-os=darwin --cc=clang \
  --extra-cflags="-arch arm64 -I../x264 -I../x265/include" \
  --extra-ldflags="-arch arm64 ../x264/libx264_universal.a ../x265/libx265_universal.a" \
  --prefix=$(pwd)/$BUILD_ARM
make -j$(sysctl -n hw.logicalcpu)
make install

# Combine static libraries into universal binaries
echo "Combining static libraries into universal binaries..."
for lib in libavcodec libavformat libavutil libswscale libswresample; do
  lipo -create $BUILD_X86/lib/$lib.a $BUILD_ARM/lib/$lib.a -output $UNIVERSAL_DIR/$lib.a
  lipo -info $UNIVERSAL_DIR/$lib.a
done

echo "Universal FFmpeg static libraries are available in $UNIVERSAL_DIR"
