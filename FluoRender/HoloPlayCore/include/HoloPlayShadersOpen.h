/*
 *
 *
    HoloPlayCore 0.1.1

    include/HoloPlayShadersOpen.h

    author: Evan Kahn
    contact: evan@lookingglassfactory.com

     If you cannot include HoloPlayShaders.h in your codebase for licensing reasons,
    please include this file instead: it refers to the shader strings embedded in
    the HoloPlayCore dynamic library, rather than statically linking the shaders
    directly from the header file.
	
	Copyright 2020 Looking Glass Factory

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software
    and associated documentation files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or
    substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
    BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
    DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef IMPORT_DECL
#ifdef _WIN32
#ifdef HPC_STATIC
#define IMPORT_DECL 
#else
#define IMPORT_DECL __declspec(dllimport)
#endif
#else
#define IMPORT_DECL
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif
	IMPORT_DECL const extern char* hpc_LightfieldVertShaderGLSLExported;
	IMPORT_DECL const extern char* hpc_LightfieldFragShaderGLSLExported;
#ifdef __cplusplus
}
#endif

#define hpc_LightfieldVertShaderGLSL hpc_LightfieldVertShaderGLSLExported
#define hpc_LightfieldFragShaderGLSL hpc_LightfieldFragShaderGLSLExported
