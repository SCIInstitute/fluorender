#!/bin/bash

version=$(lsb_release -sr)

sudo apt update
sudo apt install build-essential

sudo apt install libxkbcommon-x11-0

sudo apt install gcc-8
sudo apt install g++-8

sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 700 --slave /usr/bin/g++ g++ /usr/bin/g++-7
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 800 --slave /usr/bin/g++ g++ /usr/bin/g++-8

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
sudo apt install libglu1-mesa-dev
sudo apt install freeglut3-dev

sudo apt install ocl-icd-libopencl1
sudo apt install ocl-icd-opencl-dev
sudo apt install opencl-headers

sudo apt install qtcreator

case $version in
16.04)
  echo "Version 16.04 detected. Installing Qt 5.12 for Xenial."
	sudo add-apt-repository -y ppa:beineri/opt-qt-5.12.0-xenial
  ;;
18.04)
  echo "Version 18.04 detected. Installing Qt 5.12 for Bionic."  
	sudo add-apt-repository -y ppa:beineri/opt-qt-5.12.0-bionic
esac

sudo apt update
sudo apt install qt512-meta-minimal
sudo mkdir /etc/xdg/qtchooser
sudo cp default.conf /etc/xdg/qtchooser
