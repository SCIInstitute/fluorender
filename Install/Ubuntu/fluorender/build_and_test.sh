#!/bin/bash
set -e

PKG_NAME="fluorender"
DIST="ubuntu:22.04"

echo "=== Cleaning previous host install ==="
sudo apt remove --purge -y fluorender 2>/dev/null || true
sudo apt autoremove -y 2>/dev/null || true

# remove leftovers (safe)
sudo rm -f /usr/bin/FluoRender || true
sudo rm -f /usr/share/applications/fluorender.desktop || true
sudo rm -rf /usr/share/icons/hicolor/*/apps/fluorender.png || true

echo "=== Cleaning old builds ==="
[ -d build-area ] && sudo rm -rf build-area
mkdir build-area

echo "=== Copy project (excluding build-area) ==="
rsync -a --exclude=build-area ./ build-area/$PKG_NAME

echo "=== Pre-check: verify binary exists ==="
if [ ! -f build/bin/FluoRender ]; then
    echo "ERROR: build/bin/FluoRender not found"
    exit 1
fi

echo "=== Pre-check: inspect OpenCV linkage ==="
ldd build/bin/FluoRender | grep opencv || true

echo "=== Running build in Docker ==="

docker run --rm -it \
    -v $(pwd)/build-area:/workspace \
    -w /workspace/$PKG_NAME \
    $DIST \
    bash -c "
        set -e
        export DEBIAN_FRONTEND=noninteractive

        echo '=== Install dependencies ==='
        apt update &&
        apt install -y \
            build-essential \
            devscripts \
            debhelper \
            debhelper-compat \
            dh-make \
            cmake \
            pkg-config \
            patchelf \
            lintian \
            libgl1 \
            libegl1 \
            libx11-6 \
            libxxf86vm1 \
            libsm6 \
            libxkbcommon0 \
            libgtk-3-0 \
            libgdk-pixbuf-2.0-0 \
            libpango-1.0-0 \
            libcairo2 \
            libopenblas0 \
            libpcre2-32-0 \
            libavcodec58 \
            libavformat58 \
            libavutil56 \
            libswscale5 \
            libswresample3 \
            ocl-icd-libopencl1 \
            libwayland-client0 \
            libwayland-egl1 \
            libopencv-dev \
            libopencv-core4.5d \
            libopencv-imgproc4.5d \
            libopencv-calib3d4.5d \
            libopencv-features2d4.5d \
            libopencv-flann4.5d \
            libopencv-highgui4.5d \
            libopencv-imgcodecs4.5d \
            libopencv-videoio4.5d \
            libopencv-video4.5d \
            libopencv-dnn4.5d

        echo '=== Verify packaging inputs ==='
        test -f debian/control
        test -f debian/rules
        test -f debian/install
        test -f debian/changelog
        test -f build/bin/FluoRender

        echo '=== Build package ==='
        dpkg-buildpackage -b -us -uc

        echo '=== Locate package ==='
        cd ..
        DEB=\$(ls fluorender_*.deb | head -n 1)

        if [ -z \"\$DEB\" ]; then
            echo 'ERROR: .deb package not created'
            exit 1
        fi

        echo \"Found package: \$DEB\"

	echo '=== Clean previous install ==='
	apt remove --purge -y fluorender || true
	apt autoremove -y || true

       echo '=== Install package ==='
        apt install -y ./\$DEB || apt -f install -y

        echo '=== Verify binary installed ==='
        if ! command -v FluoRender >/dev/null; then
            echo 'ERROR: FluoRender not installed'
            exit 1
        fi

        ls -l /usr/bin/FluoRender

        echo '=== Check runtime dependencies ==='
        ldd /usr/bin/FluoRender || true

        echo '=== Detect missing libraries ==='
        if ldd /usr/bin/FluoRender | grep -q 'not found'; then
            echo 'ERROR: Missing shared libraries detected!'
            exit 1
        else
            echo 'All dependencies resolved ✅'
        fi

        echo '=== Run lintian ==='
        lintian \$DEB || true

        echo '=== SUCCESS: Package is valid ==='
    "

echo "=== DONE ==="
