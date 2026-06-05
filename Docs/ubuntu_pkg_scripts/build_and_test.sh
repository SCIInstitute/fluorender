#!/bin/bash
set -e

PKG_NAME="fluorender"
DIST="ubuntu:20.04"

echo "=== Cleaning old builds ==="
rm -rf build-area
mkdir build-area

echo "=== Copy project (excluding build-area) ==="
rsync -a --exclude=build-area ./ build-area/$PKG_NAME

echo "=== Running build in Docker ==="

docker run --rm -it \
    -v $(pwd)/build-area:/workspace \
    -w /workspace/$PKG_NAME \
    $DIST \
    bash -c "
        export DEBIAN_FRONTEND=noninteractive

        apt update &&
        apt install -y \
            build-essential \
            devscripts \
            debhelper \
            dh-make \
            cmake \
            pkg-config \
            patchelf \
            lintian \
            lsb-release &&

        echo '=== Build package ===' &&
        dpkg-buildpackage -b -us -uc &&

        echo '=== Lintian check ===' &&
        cd .. &&
        lintian fluorender_*.deb || true &&

        echo '=== Install package ===' &&
        apt install -y ./fluorender_*.deb || apt -f install -y &&

        echo '=== Check binary exists ===' &&
        ls -l /usr/bin/FluoRender &&

        echo '=== Check runtime dependencies ===' &&
        ldd /usr/bin/FluoRender || true &&

        echo '=== Detect missing libraries ===' &&
        if ldd /usr/bin/FluoRender | grep -q 'not found'; then
            echo 'ERROR: Missing shared libraries!'
            exit 1
        else
            echo 'All dependencies resolved ✅'
        fi
    "

echo "=== DONE ==="
