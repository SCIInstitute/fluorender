/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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

class WmrRenderer : public OpenXrRenderer
{
public:
	WmrRenderer();
	virtual ~WmrRenderer();

	bool Init(void*, void*) override;
	void Close() override;

	void Draw(const std::vector<flvr::Framebuffer*> &fbos) override;

protected:
	void SetExtensions() override;
	bool CreateSession(void* hdc, void* hdxrc) override;
	bool CreateSwapchains() override;
	void* CreateImageView(int type, void* format, void* tid) override;//0:color 1:depth
	void DestroyImageView(void*& imageView) override;

	virtual bool CreateD3DDevice();
	virtual void DestroyD3DDevice();

	virtual bool CreateSharedTex(const XrSwapchainCreateInfo& scci);
	virtual void DestroySharedTex();

	virtual void LoadFunctions();

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

	//func
	PFNWGLDXREGISTEROBJECTNVPROC glDXRegisterObjectNV;
	PFNWGLDXUNREGISTEROBJECTNVPROC glDXUnregisterObjectNV;
	PFNWGLDXLOCKOBJECTSNVPROC glDXLockObjectsNV;
	PFNWGLDXUNLOCKOBJECTSNVPROC glDXUnlockObjectsNV;
};

#endif//WmrRenderer_h