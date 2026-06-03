#!/bin/bash
set -e

PKG_NAME="fluorender"
DIST="ubuntu:20.04"   # change if needed

echo "=== Cleaning old builds ==="
rm -rf build-area || true
mkdir build-area

echo "=== Copy project ==="
cp -r . build-area/$PKG_NAME

echo "=== Running build in Docker ==="

docker run --rm -it \
    -v $(pwd)/build-area:/workspace \
    -w /workspace/$PKG_NAME \
    $DIST \
    bash -c "
        apt update &&
        apt install -y build-essential devscripts debhelper dh-make cmake pkg-config patchelf lsb-release &&
        
        echo '=== Build package ===' &&
        dpkg-buildpackage -b -us -uc &&
        
        echo '=== Test package install ===' &&
        cd .. &&
        apt install -y ./fluorender_*.deb || apt -f install -y &&
        
        echo '=== Run ldd check ===' &&
        ldd /usr/bin/fluorender || true &&
        
        echo '=== Check for missing libraries ===' &&
        ldd /usr/bin/fluorender | grep 'not found' && exit 1 || echo 'All good!'
    "

echo "=== DONE ==="
