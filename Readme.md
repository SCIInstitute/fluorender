# Fluorender


## Build from Source Instructions 

The following Libraries will be installed if CMake cannot find them:

Boost Version 1.70

### Requirements for Linux
Cmake will need OpenSSL support in order to download the Superbuild external libraries. In order to have that you will need to download the latest CMake Release by source. Extract the source to a specified location and then run the following:

```
./bootstrap --system-curl
make
sudo make install
```

if you are given errors about Curl not found, install `sudo apt install libcurl4-openssl-dev`. 

Java is also required to compile this program. Either the Java JDK from the official Java Source or OpenJDK will work. More work will be needed if the official JDK is installed. The easiest solution is:

```
sudo apt install openjdk
```
nasm is required for x264 to be installed. This is easily installed with:

```
sudo apt install nasm
```
OpenCL needs to be installed as well, (This may be temporary as we can supply the OpenCL Headers), the following need to be run in your terminal:

```
sudo apt install ocl-icd-libopencl1
sudo apt install ocl-icd-opencl-dev
sudo apt install opencl-headers
```
Once they are installed, you can simply Open Qt Creator and open the CMakeLists.txt file and build the program.
