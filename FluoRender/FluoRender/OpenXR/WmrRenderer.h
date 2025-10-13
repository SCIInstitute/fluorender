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

#ifndef WmrRenderer_h
#define WmrRenderer_h

#include <OpenXrRenderer.h>

typedef unsigned int GLenum;
typedef unsigned int GLuint;
struct GLFormat
{
	GLenum internalFormat;
	GLenum format;
	GLenum type;
};

class WmrRenderer : public OpenXrRenderer
{
public:
	WmrRenderer();
	virtual ~WmrRenderer();

	bool Init(void*, void*, uint64_t) override;
	void Close() override;

	void Draw(const std::vector<std::shared_ptr<flvr::Framebuffer>> &fbos) override;

protected:
	void SetExtensions() override;
	bool CreateSession(void*, void*, uint64_t) override;
	bool CreateSwapchainImages(int type, uint32_t count, SwapchainInfo& info) override;
	void* CreateImageView(int type, int eye, void* format, void* tid) override;//0:color 1:depth
	void DestroyImageView(void*& imageView) override;
	virtual void LoadFunctions() override;

	virtual bool CreateD3DDevice();
	virtual void DestroyD3DDevice();

	virtual bool CreateSharedTex();
	virtual void DestroySharedTex();

#ifdef _WIN32
	GLFormat TranslateD3D11ToGLFormat(DXGI_FORMAT d3dFormat);

protected:
	IDXGIFactory4* m_factory = nullptr;
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_im_context = nullptr;

	//shared tex
	GLuint m_gl_tex = 0;
	GLuint m_gl_fbo = 0;
	ID3D11Texture2D* m_d3d_tex = nullptr;
	HANDLE m_shared_hdl = nullptr;
	HANDLE m_interop = nullptr;
	HANDLE m_gl_d3d_tex = nullptr;
#endif
};

#endif//WmrRenderer_h