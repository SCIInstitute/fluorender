#!/bin/bash

sudo apt update
sudo apt install build-essential

sudo apt install gcc-8
sudo apt install g++-8

sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 700 --slave /usr/bin/g++ g++ /usr/bin/g++-7
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 800 --slave /usr/bin/g++ g++ /usr/bin/g++-8

sudo apt install libcurl4-openssl-dev
wget https://github.com/Kitware/CMake/releases/download/v3.17.0/cmake-3.17.0.tar.gz
tar xvzf cmake-3.17.0.tar.gz
cd cmake-3.17.0
./boostrap --system-curl
make
sudo make install

cd ..
rm -rf cmake-3.17.0
rm -rf cmake-3.17.0.tar.gz

sudo apt install openjdk

sudo apt install nasm

sudo apt install mesa-common-dev
sudo apt install libgl1-mesa-dev
sudo apt install mesa-utils-extra
sudo apt install libglapi-mesa

sudo apt install ocl-icd-libopencl1
sudo apt install ocl-icd-opencl-dev
sudo apt install opencl-headers

sudo apt install qt5-qmake qtbase5-dev libqt5opengl5-dev libqt5svg5-dev
