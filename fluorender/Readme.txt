To build FluoRender, download the latest source code of WxWidgets (MSW). Copy the WxWidgets project folder {usually named [wxWidgets-(version number)]} into the same directory of this Readme file, and rename it [wxWidgets] (delete the version number). Compile it using VisualStudio. Go to .\wxWidgets\build\msw\, and type in the following command to build wxWidgets.

Replace libtiff headers.

Search for and set this value:
#define STRIPCHOP_DEFAULT 0

64-bit Release compile: nmake -f makefile.vc BUILD=release TARGET_CPU=AMD64
64-bit Debug compile: nmake -f makefile.vc BUILD=debug TARGET_CPU=AMD64
32-bit Release compile: nmake -f makefile.vc BUILD=release
32-bit Debug compile: nmake -f makefile.vc BUILD=debug

Then go to .\FluoRender\ and open the FluoRender VisualStudio Solution file (FluoRender.sln) to build.