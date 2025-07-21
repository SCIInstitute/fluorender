![logo_crop_03-01](https://github.com/user-attachments/assets/ecc3051b-d977-4fd6-a0e1-32637a0c7e59)

[GitHub Page](https://github.com/SCIInstitute/fluorender)

This is the open-source repository for FluoRender, an interactive tool for flourescence microscopy data visualization and analysis.

<h1 id="download">Download</h1>

Download the installer package for your operating system (OS).

**Windows:** [Version 2.33](https://github.com/SCIInstitute/fluorender/releases/download/v2.33/FluoRender2.33_win64.exe)

**Mac OS:** [Version 2.32](https://github.com/SCIInstitute/fluorender/releases/download/v2.32/FluoRender2.32_mac64.pkg)

**Ubuntu:** [Version 2.32](https://github.com/SCIInstitute/fluorender/releases/download/v2.32/fluorender2.32_ubuntu22.04_amd64.deb)

<h1 id="features">Release Highlights for Version 2.33</h1>
This release marks a major leap forward in both functionality and usability. With a focus on clarity, speed, and intuitive interaction, FluoRender now offers a richer toolkit for analyzing volume data from fluorescence microscopy. Users can dive into tasks like painting, filtering, and color mapping with minimal setup — or no manual fine-tuning at all — and still achieve precise, insightful results.
Key updates streamline workflows, reduce the learning curve, and make advanced features more accessible. Whether you're visualizing time-dependent properties, customizing your color map, or leveraging smart automation, FluoRender adapts to your data and goals with surprising agility.
The goal remains constant: to empower researchers with interactive tools that feel natural to use, yet remain powerful under the hood.

- **Looking Glass Support**

  FluoRender now seamlessly supports the latest Looking Glass holographic displays, expanding your visualization toolkit with intuitive controls and immersive clarity.
   - **Hologram Modes:** Choose from three distinct modes to configure camera and lens shifts—whether for depth emphasis, parallax refinement, or artistic rotation.
   - **Projection Flexibility:** Easily toggle between orthographic and perspective projections to match your analytical or illustrative needs.
   - **Camera Control Options:** Navigate with precision using either Globe Mode for orbital rotation or Flight Mode for dynamic pathing.
   - **Auto Focusing:** Automatically focus on the scene’s center or snap to a user-specified point with a simple click.
   - **Hologram Snapshot:** Capture and share high-resolution holographic snapshots—perfect for documentation, collaboration, or presentation.
 - **Visualization & Volume Properties**
    - Overhauled **volume property settings**, with reordered layout
    - New UI displays:
      - **Intensity distribution**
      - **Color map range**
    - Updated **multi-function buttons** and introduced **min–max/boundary high controls**
    - Filter updates: **Lanczos-bicubic** scaling with zoom-aware window sizing
    - Added **4D color maps** featuring time, intensity delta, and speed options
    - Keyframe animation:
      - Now supports **volume property changes**
      - Enabled only when keyframes are present
  - **Interactive Tools & Painting**
    - New brush tools: **Segment** and **Isolate**
    - Brushes support **fine-grained grow rate**
    - Automatic **threshold estimation** for paint and component generation
    - Isolate brush works with ruler tools to locate **center points**
  - **Automation & Scripting**
    - OpenCL filter script now runs **last-used parameters** from UI when left empty
    - Added `script stop` command for one-time scripts
    - Introduced **automation options** in configuration dialog, supporting:
      - Histogram generation
      - Paint selected size computing
      - Component generation
      - Colocalization
      - Ruler relaxation
  - **Volume Filtering**
    - OpenCL editor renamed to **Volume Filter**
    - Added **deconvolution filters**, including Richardson-Lucy and Wiener filters
    - Updated **Gaussian** filters for smoother results
    - Added other commonly used filters
  - **UI & Usability Enhancements**
    - Refreshed **icons and text** throughout UI
    - Adopted **notebook-style tabs** for improved dialog navigation
    - **Dialog layouts** can now be saved and restored
    - Added **dark mode** support on Windows
    - Reordered settings to prioritize frequent tasks
    - Frequently used interactive tools now available in the **workspace panel**
    - Added dialog buttons to the **project panel**
    - Interactive tool states now **sync across dialogs**
  - **File & Data Management**
      - Added support for **INI**, **XML**, and **JSON** config file formats
      - Refactored memory handling using **smart pointers**
      - Updated **volume cache** system for time-sequenced data
      - Introduced **movie playback caching** for smoother experience
      - Enhanced capture capabilities: Support for **JPEG** and **PNG**
      - Ability to read **JPEG/PNG sequences**
  - **Core System & Build Improvements**
      - Reorganized **CMake** structure and third-party libraries
      - Switched from `wxString` and legacy path utilities to **`std::string`** and **`std::filesystem`**
      - Resolved **type cast warnings**
      - Updated **FFmpeg** integration to use current API
      - Forward-declared third-party types for cleaner compilation

<h1 id="documentation">Documentation</h1>

**User Manual:** [Version 2.32](https://github.com/SCIInstitute/fluorender/releases/download/v2.32/FluoRender2.32_Manual.pdf)

**Tutorials:** [Version 2.32](https://github.com/SCIInstitute/fluorender/releases/download/v2.32/FluoRender2.32_Tutorials.pdf)

**Video Tutorials:** [YouTube Playlist](https://youtu.be/zq41x-Q7LU0?feature=shared)

<h1 id="contact">Contact</h1>

Contact the developer for any questions or suggestions:

**Email:** yong.wan@utah.edu

**Facebook Page:** [FluoRender on Facebook](https://www.facebook.com/fluorender)

<h1>Hardware Requirements</h1>

FluoRender can run on most personal computers including desktops and laptops. The key component is a modern GPU (Graphic Processing Unit). A powerful GPU is helpful to process large data.

<h1>Aknowledgments</h1>

<strong>Code Contributors: </strong> Yong Wan, Brig Bagley, Takashi Kawase, Remaldeep Singh, etc.<br/>
If you use FluoRender in work that leads to published research, we humbly ask that you add the following to the 'Acknowledgments' section of your paper: 
"This work was made possible in part by software funded by the NIH: Fluorender: Visualization-Based and Interactive Analysis for Multichannel Microscopy Data, R01EB023947."
If you would like to cite FluoRender, you may reference the following publication:
Wan, Y., et al. (2017). FluoRender: joint free-hand segmentation and visualization for many-channel fluorescence data analysis. BMC Bioinformatics, 18:280.

<h1>Building FluoRender</h1>

<h2>Common Dependencies</h2>

Some third-party dependencies are included in the FluoRender source code. Other dependencies need to be prepared before building FluoRender.
This is especially true after recent reoganization of FluoRender source code, as many dependencies are moved out of the project. They need to be built or installed before building.
 - Windows 10 and later : Visual Studio 2017 and later<br/>
   - Or Mac OS 11 (Big Sur) and later : Updated Xcode and command line tools<br/>
   - Or Ubuntu Linux 22.04 (tested to work)
 - Git (https://git-scm.com/) for managing the source code
 - CMake (http://www.cmake.org/) for generating building projects
 - Boost (http://www.boost.org/users/download/#live) for computing using the graph library
   - Needs building before FluoRender
 - Dlib (http://dlib.net) for deep neural network calculations
   - No need to build before FluoRender for the template library
 - FFmpeg for reading and writing video files
   - https://github.com/FFmpeg/FFmpeg.git
   - x264 codec (https://code.videolan.org/videolan/x264.git)
   - x265 codec (https://bitbucket.org/multicoreware/x265_git.git)
   - May need MSYS2 to build on Windows
   - Build codecs first and then FFmpeg
 - FreeType for managing type fonts
   - https://github.com/freetype/freetype.git
   - Needs building before FluoRender
 - GLEW (https://glew.sourceforge.net) for OpenGL extension management
   - https://github.com/nigels-com/glew/releases/tag/glew-2.2.0
   - Needs building before FluoRender
 - GLM for mathematics like vectors, matrices, and quaternions
   - https://github.com/g-truc/glm.git
   - No need to build for header-only library
 - HDF5 (https://www.hdfgroup.org/download-hdf5/) for managing HDF files
   - https://github.com/HDFGroup/hdf5/releases/tag/hdf5_1.14.5
   - Needs building before FluoRender
 - JDK (https://www.oracle.com/java/technologies/downloads/) for linking to ImageJ functions
   - Needs installation
 - OpenBLAS for linear algebra computations
 - - https://github.com/OpenMathLib/OpenBLAS.git
   - Needs building before FluoRender
 - OpenCL SDK for cross-platform GPU computing
   - There are multiple providers. On Windows, I use NVIDIA CUDA Toolkit:
   - https://developer.nvidia.com/cuda-toolkit
   - Needs installation
 - OpenCV (https://opencv.org/) for computer vision calculations
   - https://github.com/opencv/opencv.git
   - Needs building before FluoRender
 - OpenVR (https://steamvr.com) for SteamVR headset support
   - https://github.com/ValveSoftware/openvr.git
   - Needs building before FluoRender
 - OpenXR (https://www.khronos.org/openxr/) for OpenXR headset support
   - https://github.com/KhronosGroup/OpenXR-SDK.git
   - Needs building before FluoRender
 - Python (https://www.python.org/downloads/) for linking to Python functions
   - Needs installation
 - Teem (https://teem.sourceforge.net/) for reading and writing Nrrd format files
   - Needs building before FluoRender
 - wxWidgets (https://github.com/wxWidgets/wxWidgets) for user-interface library
   - I made some changes to the wxWidgets code. Use my branch: https://github.com/basisunus/wxWidgets/tree/wxWidgets-v3.2.8
   - I generally use the built-in libs in wxWidgets, including Jpeg, Png, Tiff, and Zlib
   - Needs building before FluoRender

Libraries that need building before FluoRender or included as head-only are placed at the same level of the FluoRender source code path so that they can be automatically found. See FluoRender's CMake file for more details. I usually prefer the source code of a released version instead of the master head.
 
<h2>Linux</h2> 

1) Make sure OpenGL and OpenCL drivers are correctly installed and configured. This is OS and hardware dependent.

   Libs needed: libOpenCL1, glu-devel; headers needed: opencl-headers.

2) Other dependencies include: gcc, g++, gtk3-devel, ffmpeg-4-libavcodec-devel, ffmpeg-4-libavformat-devel, ffmpeg-4-libavutil-devel, ffmpeg-4-libswscale-devel, ffmpeg-4-libswresample-devel, etc.

3) Clone and build boost.

   * <code>git clone --recursive https://github.com/boostorg/boost.git</code>

   * <code>cd boost</code>

   * <code>./bootstrap.sh</code>

   * <code>./b2</code>

4) Clone and build wxWidgets.

   * <code>git clone --branch wxwidgets-3.2.6 --recursive https://github.com/basisunus/wxWidgets.git</code>

   * <code>cd wxwidgets</code>

   * <code>mkdir mybuild</code>

   * <code>cd mybuild</code>

   * <code>../configure --disable-shared --enable-cxx11 --with-cxx=11 --enable-stl --enable-std_containers --enable-std_iostreams --with-libpng=builtin --with-libtiff=builtin --with-libjpeg=builtin --with-zlib=builtin --with-opengl</code>

   * <code>make</code>

5) Use CMake to generate FluoRender project.

6) Build FluoRender. An IDE such as CodeBlocks can be used.

<h2>Mac OS</h2> 

1) Clone the latest wxWidgets using GIT (<code>git clone --branch wxwidgets-3.2.6 --recursive https://github.com/basisunus/wxWidgets.git</code>).
   
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

   * To avoid blurry user interface on high-resolution displays, add this setting to info.plist: <code>NSHighResolutionCapable</code> and set it to <code>YES</code>.

<h2>Windows</h2> 

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
    
