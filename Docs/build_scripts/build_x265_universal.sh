#!/bin/bash
set -e

# Define variables
REPO_URL="https://bitbucket.org/multicoreware/x265_git.git"
X86_DIR="x265_x86_64"
ARM_DIR="x265_arm64"
UNIVERSAL_LIB="libx265_universal.a"

# Clone x265 repository
git clone ${REPO_URL} ${X86_DIR}
git clone ${REPO_URL} ${ARM_DIR}

# Build for x86_64
mkdir -p ${X86_DIR}/build
cd ${X86_DIR}/build
cmake ../source \
    -DCMAKE_OSX_ARCHITECTURES=x86_64 \
    -DENABLE_SHARED=OFF \
    -DENABLE_CLI=OFF \
    -DENABLE_ASSEMBLY=ON
make -j$(sysctl -n hw.logicalcpu)
cd ../../

# Build for arm64
mkdir -p ${ARM_DIR}/build
cd ${ARM_DIR}/build
cmake ../source \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DENABLE_SHARED=OFF \
    -DENABLE_CLI=OFF \
    -DENABLE_ASSEMBLY=OFF \
    -DHIGH_BIT_DEPTH=ON
make -j$(sysctl -n hw.logicalcpu)
cd ../../

# Combine static libraries
lipo -create \
    ${X86_DIR}/build/libx265.a \
    ${ARM_DIR}/build/libx265.a \
    -output ${UNIVERSAL_LIB}

# Verify universal binary
lipo -info ${UNIVERSAL_LIB}
