#!/bin/bash

version=$(lsb_release -sr)

sudo apt update
sudo apt install build-essential

sudo apt install libxkbcommon-x11-0

sudo apt install gcc-8
sudo apt install g++-8

sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 700 --slave /usr/bin/g++ g++ /usr/bin/g++-7
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 800 --slave /usr/bin/g++ g++ /usr/bin/g++-8

#sudo apt install libcurl4-openssl-dev
#wget https://github.com/Kitware/CMake/releases/download/v3.17.0/cmake-3.17.0.tar.gz
#tar xvzf cmake-3.17.0.tar.gz
#cd cmake-3.17.0
#./bootstrap --system-curl
#make
#sudo make install

#cd ..
#rm -rf cmake-3.17.0
#rm -rf cmake-3.17.0.tar.gz

sudo apt install openjdk

case $version in
16.04)
  echo "Version 16.04 detected. Installing nasm by source."
  wget https://www.nasm.us/pub/nasm/releasebuilds/2.14.02/nasm-2.14.02.tar.gz
  tar xvzf nasm-2.14.02.tar.gz
  cd nasm-2.14.02
  ./autogen.sh
  ./configure --prefix=/usr/
  make
  sudo make install
	cd ..
  ;;
18.04)
  echo "Version 18.04 detected. Installing nasm by apt-get."  
  sudo apt install nasm
esac

sudo apt install mesa-common-dev
sudo apt install libgl1-mesa-dev
sudo apt install mesa-utils-extra
sudo apt install libglapi-mesa

sudo apt install ocl-icd-libopencl1
sudo apt install ocl-icd-opencl-dev
sudo apt install opencl-headers

#sudo apt install qt5-qmake qtbase5-dev libqt5opengl5-dev libqt5svg5-dev
sudo apt install xz-utils
wget https://download.qt.io/archive/qt/5.14/5.14.1/single/qt-everywhere-src-5.14.1.tar.xz
tar -xf qt-everywhere-src-5.14.1.tar.xz
cd qt-everywhere-src-5.14.1
export QT5PREFIX=/opt/qt5

sed -i 's/python /python3 /' qtdeclarative/qtdeclarative.pro \
                             qtdeclarative/src/3rdparty/masm/masm.pri &&

./configure -prefix $QT5PREFIX                        \
            -sysconfdir /etc/xdg                      \
            -confirm-license                          \
            -opensource                               \
            -openssl-linked                           \
            -nomake examples                          \
            -no-rpath                                 \
            -skip qtwebengine                         &&
make

sudo make install
