# Fluorender
FluoRender Source Code

This is the open-source repository for FluoRender, an interactive rendering tool for confocal microscopy data visualization. It combines the renderings of multi-channel volume data and polygon mesh data, where the properties of each dataset can be adjusted independently and quickly. The tool is designed especially for neurobiologists, and it helps them better visualize the fluorescent-stained confocal samples.

# Aknowledgements
If you use FluoRender in work that leads to published research, we humbly ask that you add the following to the 'Acknowledgments' section of your paper: "This work was made possible in part by software funded by the NIH: Fluorender: Visualization-Based and Interactive Analysis for Multichannel Microscopy Data, R01EB023947." If you would like to cite FluoRender, you may reference the following publication: Wan, Y., et al. (2017). FluoRender: joint free-hand segmentation and visualization for many-channel fluorescence data analysis. BMC Bioinformatics, 18:280.

**Author:** Yong Wan

**Developers:** Brig Bagley, Raul Ramirez

## Build from Source Instructions 

The following Libraries will be installed if CMake cannot find them:

Boost Version 1.70

## Requirements for building:
Each OS will need an OpenCL compatible CPU and an OpenGL compatible GPU. 
This may or may not work with AMD CPU's or GPU's and is currently not supported by Fluorender developers. Everything had been built and run on Intel CPU's and NVIDIA GPU's.

C++17 (See individual build requirements for your OS)

Cmake Version greater than 3.0 (See individual build requirements for your OS)

Java JDK (See individual build requirements for your OS) 

[QT Creator 5](https://www.qt.io/download) or greater (Choose open source.)

### Requirements for Linux
To see if the developer has C++17 installed run `g++ --version` in the command line. If a version greater than 8 appears continue onwards. If a version less than 8 appears or you are given an error that g++ could not be found; [click here](Docs/linuxgccinstall.md) and follow the instructions. 

Cmake will need OpenSSL support in order to download the Superbuild external libraries. In order to have that you will need to download the latest [CMake Release](https://cmake.org/download/) by **source.** Extract the source to a specified location and then run the following:

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
Once all the required programs are install. Open up QT Creator and open the CMakeLists.txt file and build the program.
Once they are installed, you can simply Open Qt Creator and open the CMakeLists.txt file and build the program.

### Requirements for OSX
Download the latest [CMake Release](https://cmake.org/download/) and simply install the dmg image. The latest release will not add CMake to your path, so you will not be able to use cmake build commands in your terminal. To add to your path, you will need to run the following command in your terminal:  

`sudo /Applications/CMake.app/Contents/bin/cmake-gui --install` 

This will link cmake to your path and you will be able to use CMake in your command line.  

Java is required, so simply download the latest [Java JDK Release](https://www.oracle.com/technetwork/java/javase/downloads/index.html) (The current release at the time this readme was created is Java SE 13). Simply accept the license and download the dmg image file and install it. Or you can simply `brew cast install java` in order to get OpenJDK installed. Either works.

OpenCL and OpenGL by default are included with OSX (At least this is true on OSX 10.14). 

Once all the required programs are install. Open up QT Creator and open the CMakeLists.txt file and build the program. Or simply run from your build directory:  

`cmake <fluorender_location> -G "Xcode" -DCMAKE_PREFIX_PATH=<your Qt install folder, mine was /User/Sailanarmo/Qt/5.13.2/clang_64>"` 

And finally run: `cmake --build .`

### Requirements for Windows
Currently, MinGW is not supported since `std::codecvt_utf8_utf16` has been depricated in C++17. Unfortunately pole uses this function and as of right now, there is no fix for this. If there are any updates in the future there will be build instructions for MinGW.

Microsoft Visual Studio 2017 or greater will be required for this to build as it has C++17 support built inside.

Download the latest [CMake Release](https://cmake.org/download/) and simply install the x64 msi installer. (x86 if the developers machine is not a 64 bit operating system.) 

Java is required, so simply download the latest [Java JDK Release](https://www.oracle.com/technetwork/java/javase/downloads/index.html) (The current release at the time this readme was created is Java SE 13). Simply accept the license and download the exe installer and install it.

OpenCL and OpenGL should already be installed. However, it is possible that OpenGL will need to be manually installed via developer tools accompanied by your GPU. If the developer is given an error that OpenGL could not be found. Simply download the latest [OpenGL Drivers for NVIDIA](https://developer.nvidia.com/opengl-driver).

Once all the required programs are install. Open up QT Creator and open the CMakeLists.txt file and build the program.

Known issues with VS: 

`cl is not a full path` this issue appears when CMake cannot find cl.exe that comes with Visual Studio installation. One possible solution is to add cl.exe to your path. Which can generally be found in your VS install folder, typically in Program Files(x86). The install version, VC, and then inside of the bin. An example: `C:\Program Files(x86)\Microsoft Visual Studio 14.0\VC\bin\cl.exe`.

If that does not work try reinstalling Qt and see if it automatically picks up the compiler.  
