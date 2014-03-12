To build FluoRender:
12 March 2014

1) Download the latest wxWidgets (currently 3.0.0).
2) Install the headers and libraries to your system.
3) In the main directory, (only clean.sh, CMakeLists.txt, and fluorender folder),
   type "ccmake ." to configure build properties.
4) If all properties are set as desired, type "cmake ." . 
5) If cmake generated Makefiles successfully, type "make" .

Using cmake, you can generate XCode (MacOS X) and MS Visual Studio (Windows)
project files. Simply type "cmake" to find the proper options.

If there are any problems, email: fluorender@sci.utah.edu
