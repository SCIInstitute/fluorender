/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include <GL/glew.h>
#include <GraphicsQuery.h>

using namespace flvr;

std::string GraphicsQuery::getVersion() {
	return reinterpret_cast<const char*>(glGetString(GL_VERSION));
}

int GraphicsQuery::getVersionMajor()
{
	GLint val;
	glGetIntegerv(GL_MAJOR_VERSION, &val);
	return val;
}

int GraphicsQuery::getVersionMinor()
{
	GLint val;
	glGetIntegerv(GL_MINOR_VERSION, &val);
	return val;
}

std::string GraphicsQuery::getShadingLanguageVersion() {
	return reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
}

std::string GraphicsQuery::getVendor() {
	return reinterpret_cast<const char*>(glGetString(GL_VENDOR));
}

std::string GraphicsQuery::getRenderer() {
	return reinterpret_cast<const char*>(glGetString(GL_RENDERER));
}

int GraphicsQuery::getMaxTextureSize() {
	GLint value;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value);
	return value;
}

int GraphicsQuery::getMaxCombinedTextureUnits() {
	GLint value;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &value);
	return value;
}

int GraphicsQuery::getMaxVertexAttribs() {
	GLint value;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &value);
	return value;
}

int GraphicsQuery::getMaxUniformBufferBindings() {
	GLint value;
	glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &value);
	return value;
}

int GraphicsQuery::getMaxDrawBuffers() {
	GLint value;
	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &value);
	return value;
}

int GraphicsQuery::getMaxSamples() {
	GLint value;
	glGetIntegerv(GL_MAX_SAMPLES, &value);
	return value;
}

int GraphicsQuery::getCurrentGPUMemoryMB() {
	GLint mem_info[4] = { 0, 0, 0, 0 };
	GLenum error;

	// NVIDIA
	glGetIntegerv(0x9049 /*GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX*/, mem_info);
	error = glGetError();
	if (error != GL_INVALID_ENUM && mem_info[0] > 0)
		return mem_info[0] / 1024;

	// AMD
	glGetIntegerv(0x87FC /*GL_TEXTURE_FREE_MEMORY_ATI*/, mem_info);
	error = glGetError();
	if (error != GL_INVALID_ENUM && mem_info[0] > 0)
		return mem_info[0] / 1024;

	// Unsupported
	return 0;
}

int GraphicsQuery::getTotalGPUMemoryMB() {
	GLint mem_info[4] = { 0, 0, 0, 0 };
	GLenum error;

	// NVIDIA
	glGetIntegerv(0x9048 /*GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX*/, mem_info);
	error = glGetError();
	if (error != GL_INVALID_ENUM && mem_info[0] > 0)
		return mem_info[0] / 1024;

	// AMD fallback: treat current as total
	glGetIntegerv(0x87FC /*GL_TEXTURE_FREE_MEMORY_ATI*/, mem_info);
	error = glGetError();
	if (error != GL_INVALID_ENUM && mem_info[0] > 0)
		return mem_info[0] / 1024;

	// Unsupported
	return 0;
}

fluo::Vector4i GraphicsQuery::getViewport() {
	GLint v[4];
	glGetIntegerv(GL_VIEWPORT, v);
	return { v[0], v[1], v[2], v[3] };
}

bool GraphicsQuery::isExtensionSupported(const std::string& name) {
	GLint numExtensions = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
	for (GLint i = 0; i < numExtensions; ++i) {
		const char* ext = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
		if (name == ext)
			return true;
	}
	return false;
}
