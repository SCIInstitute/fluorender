![logo_crop_03-01](https://github.com/user-attachments/assets/ecc3051b-d977-4fd6-a0e1-32637a0c7e59)

Download
========

This is the open-source repository for FluoRender, an interactive tool for flourescence microscopy data visualization and analysis. Download the installer package for your operating system (OS).

**Windows:** [Version 2.31](https://github.com/SCIInstitute/fluorender/releases/download/v2.31/FluoRender2.31_win64.exe)

**Mac OS:** [Version 2.31](https://github.com/SCIInstitute/fluorender/releases/download/v2.31/FluoRender2.31_mac64.pkg)

**Ubuntu:** [Version 2.31](https://github.com/SCIInstitute/fluorender/releases/download/v2.31/fluorender2.31_ubuntu22.04_amd64.deb)

Documentation
========

**User Manual:** [Version 2.30](https://github.com/SCIInstitute/fluorender/releases/download/v2.30/FluoRender2.30_Manual.pdf)

**Tutorials:** [Version 2.30](https://github.com/SCIInstitute/fluorender/releases/download/v2.30/FluoRender2.30_Tutorials.pdf)

**Video Tutorials:** [YouTube Playlist](https://youtu.be/zq41x-Q7LU0?feature=shared)

Contact
========

Contact the developer for any questions or suggestions:

**Email:** yong.wan@utah.edu

**Facebook Page:** https://www.facebook.com/fluorender

Hardware Requirements
========

FluoRender can run on most personal computers including desktops and laptops. The key component is a modern GPU (Graphic Processing Unit). A powerful GPU is helpful to process large data.

Aknowledgements
========
If you use FluoRender in work that leads to published research, we humbly ask that you add the following to the 'Acknowledgments' section of your paper: 
"This work was made possible in part by software funded by the NIH: Fluorender: Visualization-Based and Interactive Analysis for Multichannel Microscopy Data, R01EB023947."
If you would like to cite FluoRender, you may reference the following publication:
Wan, Y., et al. (2017). FluoRender: joint free-hand segmentation and visualization for many-channel fluorescence data analysis. BMC Bioinformatics, 18:280.

<strong>Author: </strong> Yong Wan<br/>
<strong>Developer: </strong> Brig Bagley<br/>

Building FluoRender
========
Requirements
========
 * Windows 10 and later : Visual Studio 15.0 2017 and later

   Or Mac OS 10.11 and later : Latest Xcode and command line tools

   Or Ubuntu Linux 22.04 and later
 * Git (https://git-scm.com/)
 * CMake 3.5+ (http://www.cmake.org/)
 * wxWidgets (https://github.com/wxWidgets/wxWidgets)
 * Boost 1.78.0+ (http://www.boost.org/users/download/#live)
 * OpenCV (https://opencv.org/)
 * Dlib (http://dlib.net)


<h1>Linux</h1> 

1) Make sure OpenGL and OpenCL drivers are correctly installed and configured. This is OS and hardware dependent.

   Libs needed: libOpenCL1, glu-devel; headers needed: opencl-headers.

2) Other dependencies include: gcc, g++, git, cmake, jdk, gtk3-devel, ffmpeg-4-libavcodec-devel, ffmpeg-4-libavformat-devel, ffmpeg-4-libavutil-devel, ffmpeg-4-libswscale-devel, ffmpeg-4-libswresample-devel, etc.

3) Clone and build boost.

   * <code>git clone --recursive https://github.com/boostorg/boost.git</code>

   * <code>cd boost</code>

   * <code>./bootstrap.sh</code>

   * <code>./b2</code>

4) Clone and build wxWidgets.

   * <code>git clone --recursive https://github.com/wxWidgets/wxWidgets.git</code>

   * <code>cd wxwidgets</code>

   * <code>mkdir mybuild</code>

   * <code>cd mybuild</code>

   * <code>../configure --disable-shared --enable-cxx11 --with-cxx=11 --enable-stl --enable-std_containers --enable-std_iostreams --with-libpng=builtin --with-libtiff=builtin --with-libjpeg=builtin --with-zlib=builtin --with-opengl</code>

   * <code>make</code>

5) Use CMake to generate FluoRender project.

6) Build FluoRender. An IDE such as CodeBlocks can be used.

<h1>Mac OS</h1> 

1) Clone the latest wxWidgets using GIT (<code>git clone git@github.com:wxWidgets/wxWidgets.git</code>).
   
   * The steps following will assume the wxWidgets root directory is at <code>/wxWidgets</code>

2) Build wxWidgets from the command line.
   * <code>cd /wxWidgets/</code>
   
   * <code>mkdir mybuild</code>
   
   * <code>cd mybuild</code>
   
   * <code>../configure --disable-shared --enable-macosx_arch=x86_64 --with-cocoa --with-macosx-version-min=10.15 --enable-cxx11 --with-cxx=11 --enable-stl --enable-std_containers --enable-std_iostreams --with-libpng=builtin --with-libtiff=builtin --with-libjpeg=builtin --with-zlib=builtin</code>
   
   * <code>make</code>

3) Download and build boost.

   * Download boost (http://www.boost.org/users/download/#live) and extract onto your machine.
   
   * Build boost using <code>./bootstrap.sh</code> and <code>./b2</code> in the boost directory.
   
   * The steps following will assume the boost root directory is at <code>/boost_1_xx_0</code> (your version might differ).

4) Get homebrew, libtiff, and freetype

   * <code>/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"</code>
   
   * <code>brew install libtiff</code>

   * <code>brew install freetype</code>

5) Get and build FluoRender

   * <code>git clone git@github.com:SCIInstitute/fluorender.git</code><br/>
   
   * <code>cd fluorender</code><br/>
   
   * <code>mkdir build</code><br/>
   
   * <code>cd build</code><br/>

   * <code>cmake -G Xcode -DwxWidgets_CONFIG_EXECUTABLE="/wxWidgets/mybuild/wx-config" -DwxWidgets_wxrc_EXECUTABLE="/wxWidgets/mybuild/utils/wxrc/wxrc" -DwxWidgets_USE_DEBUG=ON -DwxWidgets_ROOT_DIR="/wxWidgets" -DBoost_INCLUDE_DIR="/Users/YourUserName/boost_1_xx_0" -DJAVA_AWT_INCLUDE_PATH="/Library/Java/JavaVirtualMachines/jdk-xx.x.x.jdk/Contents/Home/include" -DJAVA_INCLUDE_PATH="/Library/Java/JavaVirtualMachines/jdk-xx.x.x.jdk/Contents/Home/include" -DJAVA_INCLUDE_PATH2="/Library/Java/JavaVirtualMachines/jdk-xx.x.x.jdk/Contents/Home/include/darwin" -DCMAKE_BUILD_TYPE="Debug" ..</code> (replace directories with your versions)

5) Open the Xcode file generated to build and run FluoRender.

<h1>Windows</h1> 

1) Clone the latest wxWidgets using GIT (<code>git clone git@github.com:wxWidgets/wxWidgets.git</code>).
   
   * The steps following will assume the wxWidgets repository is at <code>C:\wxWidgets</code>

2) Open a 64 bit Visual Studio command prompt to build wxWidgets. (make sure you use the prompt version you wish to build all dependencies, IE , MSVC 15.0 2017 x64)

   * Go to directory <code>C:\wxWidgets\build\msw</code>
  
   * Type <code>nmake /f makefile.vc TARGET_CPU=x64 BUILD=debug</code> to build debug libraries.

   * Type <code>nmake /f makefile.vc TARGET_CPU=x64 BUILD=release</code> to build release libraries.
   
3) Download and build boost.

   * Download boost (http://www.boost.org/users/download/#live) and extract onto your machine.
   
   * Build boost using <code>bootstrap.exe</code> and <code>b2.exe --toolset=msvc-15.0 --build-type=complete architecture=x86 address-model=64 stage</code> in the boost directory in a MSVC prompt. (change the toolset to the version of MSVC you are using, and omit address-model and architecture for 32-bit)
   
   * The steps following will assume the boost root directory is at <code>C:\boost_1_xx_0</code> (your version might differ).

4) You may need to add lines to <code>C:\Program Files (x86)\CMake X.X\share\cmake-x.x\Modules\FindwxWidgets.cmake</code> (x's are your version) for wxWidgets 3.* if it still complains that you haven't installed wxWidgets.
   
   * Starting about line 277, you will have listed a few sets of library versions to search for like <code>wxbase29${_UCD}${_DBG}</code> <br/>
   
   * In 4 places, you will need to add above each line with a "29" a new line that is exactly the same, but with a "31" instead, assuming your version of wxWidgets is 3.1.*). <br/>

5) Other dependencies: OpenCV, JDK, Python, HDF5.

6) Download FluoRender using Git <code>git clone git@github.com:SCIInstitute/fluorender.git</code>

7) Use the <code>C:\Program Files(x86)\CMake2.8\bin\cmake-gui.exe</code> program to configure build properties and generate your Visual Studio Solution file. (Remember to keep your MSVC version consistent)
   
   * Select your FluoRender source and build directories (create a new folder for building), and add the locations of boost and wxWidgets. <br/>
   	- Choose the FluoRender main folder for source and create a new folder for the build. <br/>
   	
   	- Click Configure.  NOTE: You may need to display advanced options to set below options. <br/>
   	
   	- Choose the build type <code>CMAKE_BUILD_TYPE</code> to be "Debug" or "Release" <br/>

   	- Be sure to set <code>wxWidgets_LIB_DIR</code> to <code>C:\wxWidgets\lib\vc_x64_lib</code>. (this will differ from 32 bit)
   	
   	- Be sure to set <code>wxWidgets_ROOT_DIR</code> to <code>C:\wxWidgets</code>.
   	
   	- Be sure to set <code>Boost_INCLUDE_DIR</code> to <code>C:\boost_1_xx_0</code> (x's are your version). <br/>
   	
   	- Click Generate. 

   * You may also generate using the command prompt, but you must explicitly type the paths for the cmake command. <br/>
   
    - Open Visual Studio Command Prompt. Go to the CMakeLists.txt directory. <br/>
    	
    - Type <code> cmake -G "Visual Studio 15 2017 Win64" -DwxWidgets_LIB_DIR="C:\wxWidgets\lib\vc_x64_lib" -DwxWidgets_ROOT_DIR="C:\wxWidgets" -DBoost_INCLUDE_DIR="C:\boost_1_xx_0" -DCMAKE_BUILD_TYPE="Debug" ..</code> in your build directory (again assuming these are your directory locations / Generator versions, and the build folder is in the FluoRender root directory). <br/>
    	
   * Open the Visual Studio SLN file generated by CMake (found in your "build" directory). <br/>
   
   * Build the solution. Use CMake to generate both "Release" and "Debug" configurations if you wish to build both in Visual Studio.<br/><br/>
    	**Notes for Visual Studio**
    - Visual Studio may not set the correct machine target when building 64 bit. 
     Check <code>Project Properties -> Configuration Properties -> Linker -> Command line</code>. Make sure "Additional Options" is <code>/machine:X64</code> NOT <code>/machine:X86</code>. <br/>
    	
    - You may need to right-click FluoRender project on the Solution Explorer to "Set as StartUp Project" for it to run. <br/>
    - If you are building on Windows 8 or later, you will need to set a Visual Studio Graphics Option. This enables the application to build in higher definition.<br/>
      <code>Project Properties -> Manifest Tool -> Input and Output -> Enable DPI Awareness -> Yes </code> <br/>
    - On Mac OS, add this setting to info.plist:
    <key>NSHighResolutionCapable</key>
    <true/>
