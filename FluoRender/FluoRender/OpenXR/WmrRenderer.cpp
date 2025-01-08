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

#include <Framebuffer.h>
#include <WmrRenderer.h>
#include <Global.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstring>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <string>
#include <fstream>
#ifdef _DEBUG
#include <Debug.h>
#endif

WmrRenderer::WmrRenderer() :
	OpenXrRenderer()
{
	m_app_name = "FluoRender";
	m_eng_name = "FLWMROPENXR";

	m_preferred_color_formats =
	{
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_B8G8R8A8_UNORM,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		DXGI_FORMAT_B8G8R8A8_UNORM_SRGB
	};
	m_preferred_depth_formats =
	{
		DXGI_FORMAT_D32_FLOAT,
		DXGI_FORMAT_D16_UNORM
	};
}

WmrRenderer::~WmrRenderer()
{
}

bool WmrRenderer::Init(void* hdc, void* hglrc)
{
	if (m_initialized)
		return m_initialized;

	SetExtensions();

	// OpenXR initialization
	if (!CreateInstance())
		return false;

	//load opengl-d3d interop functions
	LoadFunctions();

#ifdef _DEBUG
	CreateDebugMessenger();
	GetInstanceProperties();
#endif

	if (!GetSystemID())
		return false;

	CreateActionSet();
	SuggestBindings();

	CreateD3DDevice();

	// Get render size
	if (!GetViewConfigurationViews())
		return false;

	GetEnvironmentBlendModes();

	if (!CreateSession(hdc, hglrc))
		return false;

	CreateActionPoses();
	AttachActionSet();

	if (!CreateReferenceSpace())
		return false;

	if (!CreateSwapchains())
		return false;

	//create shared tex if not present
	if (m_d3d_tex == nullptr)
		CreateSharedTex();

	m_initialized = true;

	return m_initialized;
}

void WmrRenderer::Close()
{
	DestroySharedTex();
	DestroyActions();
	DestroySwapchains();
	DestroyReferenceSpace();
	DestroySession();
#ifdef _DEBUG
	DestroyDebugMessenger();
#endif
	DestroyInstance();
	DestroyD3DDevice();
}

void WmrRenderer::Draw(const std::vector<flvr::Framebuffer*> &fbos)
{
	if (!m_app_running || !m_session_running)
		return;
	if (!m_frame_state.shouldRender)
		return;

	uint32_t viewCount = m_render_layer_info.layerProjectionViews.size();
	// Per view in the view configuration:
	for (uint32_t i = 0; i < viewCount; i++)
	{
		// Get the width and height and construct the viewport and scissors.
		const uint32_t &width = m_view_config_views[i].recommendedImageRectWidth;
		const uint32_t &height = m_view_config_views[i].recommendedImageRectHeight;

		SwapchainInfo& colorSwapchainInfo = m_swapchain_infos_color[i];
		SwapchainInfo& depthSwapchainInfo = m_swapchain_infos_depth[i];

		// Acquire and wait for an image from the swapchains.
		// Get the image index of an image in the swapchains.
		// The timeout is infinite.
		uint32_t colorImageIndex = 0;
		uint32_t depthImageIndex = 0;
		XrSwapchainImageAcquireInfo acquireInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
		OPENXR_CHECK(xrAcquireSwapchainImage(colorSwapchainInfo.swapchain, &acquireInfo, &colorImageIndex), "Failed to acquire Image from the Color Swapchian");
		if (m_use_depth)
			OPENXR_CHECK(xrAcquireSwapchainImage(depthSwapchainInfo.swapchain, &acquireInfo, &depthImageIndex), "Failed to acquire Image from the Depth Swapchian");

		XrSwapchainImageWaitInfo waitInfo = { XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
		waitInfo.timeout = XR_INFINITE_DURATION;
		OPENXR_CHECK(xrWaitSwapchainImage(colorSwapchainInfo.swapchain, &waitInfo), "Failed to wait for Image from the Color Swapchain");
		if (m_use_depth)
			OPENXR_CHECK(xrWaitSwapchainImage(depthSwapchainInfo.swapchain, &waitInfo), "Failed to wait for Image from the Depth Swapchain");

		// Fill out the XrCompositionLayerProjectionView structure specifying the pose and fov from the view.
		// This also associates the swapchain image with this layer projection view.
		m_render_layer_info.layerProjectionViews[i].subImage.swapchain = colorSwapchainInfo.swapchain;

		//copy buffer
		if (fbos.size() > i)
		{
			//test
			//float clearColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
			//m_im_context->ClearRenderTargetView((ID3D11RenderTargetView*)(colorSwapchainInfo.imageViews[colorImageIndex]), clearColor);

			// Share the texture with Direct3D
			wglDXLockObjectsNV(m_interop, 1, &m_gl_d3d_tex);

			glBindFramebuffer(GL_READ_FRAMEBUFFER, fbos[i]->id());
			// Read pixels to PBO
			GLuint dest_fbo = (GLuint)(uint64_t)(m_gl_fbo);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest_fbo);
			glBlitFramebuffer(
				0, 0, width, height,
				0, height, width, 0,
				GL_COLOR_BUFFER_BIT, GL_NEAREST);
			// Unbind the framebuffers
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

			// Unlock the OpenGL texture
			wglDXUnlockObjectsNV(m_interop, 1, &m_gl_d3d_tex);

			// Copy the shared texture to the swap chain image view
			ID3D11RenderTargetView* target_view = static_cast<ID3D11RenderTargetView*>(colorSwapchainInfo.imageViews[colorImageIndex]);
			// Get the underlying texture from the render target view
			ID3D11Resource* target_res;
			target_view->GetResource(&target_res);
			
			try
			{
				m_im_context->CopyResource(target_res, m_d3d_tex);
			}
			catch (const std::exception& e)
			{
#ifdef _DEBUG
				DBGPRINT(L"%s\n", e.what());
#endif
			}

			// Release the resource when done
			target_res->Release();
		}

		// Give the swapchain image back to OpenXR, allowing the compositor to use the image.
		XrSwapchainImageReleaseInfo releaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
		OPENXR_CHECK(xrReleaseSwapchainImage(colorSwapchainInfo.swapchain, &releaseInfo), "Failed to release Image back to the Color Swapchain");
		if (m_use_depth)
			OPENXR_CHECK(xrReleaseSwapchainImage(depthSwapchainInfo.swapchain, &releaseInfo), "Failed to release Image back to the Depth Swapchain");
	}

	// Fill out the XrCompositionLayerProjection structure for usage with xrEndFrame().
	m_render_layer_info.layerProjection.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT | XR_COMPOSITION_LAYER_CORRECT_CHROMATIC_ABERRATION_BIT;
	m_render_layer_info.layerProjection.space = m_space;
	m_render_layer_info.layerProjection.viewCount = static_cast<uint32_t>(m_render_layer_info.layerProjectionViews.size());
	m_render_layer_info.layerProjection.views = m_render_layer_info.layerProjectionViews.data();
	m_render_layer_info.layers.push_back(
		reinterpret_cast<XrCompositionLayerBaseHeader*>(
			&m_render_layer_info.layerProjection));
}

void WmrRenderer::SetExtensions()
{
	// Add additional instance layers/extensions that the application wants.
	// Add both required and requested instance extensions.
	m_instanceExtensions.push_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);
	// D3D11 extension is required for this sample, so check if it's supported.
	m_instanceExtensions.push_back(XR_KHR_D3D11_ENABLE_EXTENSION_NAME);
}

bool WmrRenderer::CreateSession(void* hdc, void* hdxrc)
{
	XrResult result;
	// Create an XrSessionCreateInfo structure.
	XrSessionCreateInfo sessionCI{ XR_TYPE_SESSION_CREATE_INFO };
	XrGraphicsBindingD3D11KHR graphicsBindingD3D11 = { XR_TYPE_GRAPHICS_BINDING_D3D11_KHR };
	graphicsBindingD3D11.device = m_device;

	// Fill out the XrSessionCreateInfo structure and create an XrSession.
	sessionCI.next = (void*)&graphicsBindingD3D11;
	sessionCI.createFlags = 0;
	sessionCI.systemId = m_sys_id;
	result = xrCreateSession(m_instance, &sessionCI, &m_session);
	if (result != XR_SUCCESS) return false;

	return true;
}

bool WmrRenderer::CreateSwapchainImages(int type, uint32_t count, SwapchainInfo& info)
{
	if (!count)
		return false;
	std::vector<XrSwapchainImageD3D11KHR> swapchain_images;
	swapchain_images.resize(count, { XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR });
	XrSwapchainImageBaseHeader* colorSwapchainImages = reinterpret_cast<XrSwapchainImageBaseHeader*>(swapchain_images.data());
	XrResult result = xrEnumerateSwapchainImages(info.swapchain,
		count, &count, colorSwapchainImages);
	if (result != XR_SUCCESS) return false;

	// Per image in the swapchains, fill out a GraphicsAPI::ImageViewCreateInfo structure and create a color/depth image view.
	for (uint32_t j = 0; j < count; j++)
	{
		info.imageViews.push_back(
			CreateImageView(type,
				(void*)(uint64_t)info.swapchainFormat,
				(void*)(uint64_t)swapchain_images[j].texture));
	}
	return true;
}

void* WmrRenderer::CreateImageView(int type, void* format, void* tid)
{
	switch (type)
	{
		case 0://color
		{
			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
			rtvDesc.Format = (DXGI_FORMAT)(uint64_t)format;
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture2D.MipSlice = 0;
			ID3D11RenderTargetView* rtv = nullptr;
			m_device->CreateRenderTargetView((ID3D11Resource*)(uint64_t)tid, &rtvDesc, &rtv);
			return rtv;
		}
		case 1://depth
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
			dsvDesc.Format = (DXGI_FORMAT)(uint64_t)format;
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Texture2D.MipSlice = 0;
			ID3D11DepthStencilView* dsv = nullptr;
			m_device->CreateDepthStencilView((ID3D11Resource*)(uint64_t)tid, &dsvDesc, &dsv);
			return dsv;
		}
	}
	return 0;
}

void WmrRenderer::DestroyImageView(void*& imageView)
{
	ID3D11View* d3d11ImageView = (ID3D11View*)imageView;
	if (d3d11ImageView)
	{
		d3d11ImageView->Release();
	}
	imageView = nullptr;
}

bool WmrRenderer::CreateD3DDevice()
{
	XrResult result;

	PFN_xrGetD3D11GraphicsRequirementsKHR xrGetD3D11GraphicsRequirementsKHR = nullptr;
	result = xrGetInstanceProcAddr(m_instance, "xrGetD3D11GraphicsRequirementsKHR", (PFN_xrVoidFunction*)&xrGetD3D11GraphicsRequirementsKHR);
	if (result != XR_SUCCESS) return false;
	XrGraphicsRequirementsD3D11KHR graphicsRequirements{ XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR };
	result = xrGetD3D11GraphicsRequirementsKHR(m_instance, m_sys_id, &graphicsRequirements);
	if (result != XR_SUCCESS) return false;

	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&m_factory));
	if (!SUCCEEDED(hr)) return false;

	IDXGIAdapter* adapter = nullptr;
	DXGI_ADAPTER_DESC adapterDesc = {};
	for (UINT i = 0; m_factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		adapter->GetDesc(&adapterDesc);
		if (memcmp(&graphicsRequirements.adapterLuid, &adapterDesc.AdapterLuid, sizeof(LUID)) == 0) {
			break;  // We have the matching adapter that OpenXR wants.
		}
		// If we don't get a match reset adapter to nullptr to force a throw.
		adapter = nullptr;
	}
	if (adapter == nullptr)
		return false;

	D3D_DRIVER_TYPE driverType = adapter == nullptr ? D3D_DRIVER_TYPE_HARDWARE : D3D_DRIVER_TYPE_UNKNOWN;
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	std::vector<D3D_FEATURE_LEVEL> featureLevels = { D3D_FEATURE_LEVEL_12_1,
													D3D_FEATURE_LEVEL_12_0,
													D3D_FEATURE_LEVEL_11_1,
													D3D_FEATURE_LEVEL_11_0,
													D3D_FEATURE_LEVEL_10_1,
													D3D_FEATURE_LEVEL_10_0 };
	featureLevels.erase(std::remove_if(featureLevels.begin(),
		featureLevels.end(),
		[&](D3D_FEATURE_LEVEL fl) { return fl < graphicsRequirements.minFeatureLevel; }),
		featureLevels.end());
	if (featureLevels.empty())
	{
#ifdef _DEBUG
		DBGPRINT(L"Unsupported minimum feature level!\n");
#endif
		return false;
	}

	hr = D3D11CreateDevice(
		adapter,
		driverType,
		0,
		creationFlags,
		featureLevels.data(),
		(UINT)featureLevels.size(),
		D3D11_SDK_VERSION,
		&m_device,
		nullptr,
		&m_im_context);

	if (!SUCCEEDED(hr)) return false;
	return true;
}

void WmrRenderer::DestroyD3DDevice()
{
	if (m_im_context)
	{
		m_im_context->Release();
		m_im_context = nullptr;
	}
	if (m_device)
	{
		m_device->Release();
		m_device = nullptr;
	}
	if (m_factory)
	{
		m_factory->Release();
		m_factory = nullptr;
	}
}

GLFormat WmrRenderer::TranslateD3D11ToGLFormat(DXGI_FORMAT d3dFormat)
{
	static const std::unordered_map<DXGI_FORMAT, GLFormat> formatMap =
	{
		{ DXGI_FORMAT_R8G8B8A8_UNORM, { GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE } },
		{ DXGI_FORMAT_B8G8R8A8_UNORM, { GL_RGBA8, GL_BGRA, GL_UNSIGNED_BYTE } },
		{ DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, { GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE } },
		{ DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, { GL_SRGB8_ALPHA8, GL_BGRA, GL_UNSIGNED_BYTE } },
		{ DXGI_FORMAT_R8_UNORM, { GL_R8, GL_RED, GL_UNSIGNED_BYTE } },
		{ DXGI_FORMAT_R16_FLOAT, { GL_R16F, GL_RED, GL_HALF_FLOAT } },
		{ DXGI_FORMAT_R32_FLOAT, { GL_R32F, GL_RED, GL_FLOAT } },
		{ DXGI_FORMAT_R16G16B16A16_FLOAT, { GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT } },
		{ DXGI_FORMAT_R32G32B32A32_FLOAT, { GL_RGBA32F, GL_RGBA, GL_FLOAT } },
		{ DXGI_FORMAT_D32_FLOAT, { GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT } },
		{ DXGI_FORMAT_D24_UNORM_S8_UINT, { GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8 } },
		// Add more formats as needed
	};

	auto it = formatMap.find(d3dFormat);
	if (it != formatMap.end())
	{
		return it->second;
	}
	else
	{
		// Handle unsupported format
		return { GL_NONE, GL_NONE, GL_NONE };
	}
}

bool WmrRenderer::CreateSharedTex()
{
	// Describe the shared texture
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = m_size[0];
	desc.Height = m_size[1];
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = static_cast<DXGI_FORMAT>(m_swapchain_infos_color[0].swapchainFormat);
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

	//desc.MipLevels = 0;
	//desc.ArraySize = 1;
	//desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	//desc.SampleDesc.Count = 1;
	//desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	// Create the shared texture
	m_device->CreateTexture2D(&desc, nullptr, &m_d3d_tex);

	// Get the shared handle
	IDXGIResource* dxgiResource = nullptr;
	m_d3d_tex->QueryInterface(__uuidof(IDXGIResource), (void**)&dxgiResource);
	dxgiResource->GetSharedHandle(&m_shared_hdl);
	dxgiResource->Release();

	// Create an OpenGL texture
	glGenTextures(1, &m_gl_tex);
	glBindTexture(GL_TEXTURE_2D, m_gl_tex);

	// Create the OpenGL texture from the shared handle
	GLFormat glFormat = TranslateD3D11ToGLFormat(desc.Format);
	glTexImage2D(GL_TEXTURE_2D, 0,
		glFormat.internalFormat,
		m_size[0], m_size[1], 0,
		glFormat.format,
		glFormat.type,
		nullptr);

	// Register the texture with OpenGL
	m_interop = wglDXOpenDeviceNV(m_device);
	if (!m_interop)
	{
#ifdef _DEBUG
		DBGPRINT(L"wglDXOpenDeviceNV failed!\n");
#endif
		return false;
	}

	m_gl_d3d_tex = wglDXRegisterObjectNV(m_interop, m_d3d_tex, m_gl_tex, GL_TEXTURE_2D, WGL_ACCESS_READ_WRITE_NV);
	if (!m_gl_d3d_tex)
	{
#ifdef _DEBUG
		DBGPRINT(L"wglDXRegisterObjectNV failed!\n");
#endif
		return false;
	}

	wglDXLockObjectsNV(m_interop, 1, &m_gl_d3d_tex);
	//create destination fbo for receiving images
	glGenFramebuffers(1, &m_gl_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_gl_fbo);
	// Attach the shared texture to the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gl_tex, 0);
	// Check if the framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		// Handle incomplete framebuffer status
#ifdef _DEBUG
		DBGPRINT(L"Framebuffer is not complete!\n");
#endif
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	wglDXUnlockObjectsNV(m_interop, 1, &m_gl_d3d_tex);

	return true;
}

void WmrRenderer::DestroySharedTex()
{
	// Unregister the OpenGL texture
	if (m_gl_d3d_tex)
	{
		wglDXUnregisterObjectNV(m_interop, m_gl_d3d_tex);
	}

	// Close OpenGL-D3D Interop
	if (m_interop)
	{
		wglDXCloseDeviceNV(m_interop);
	}

	// Delete the OpenGL texture
	if (m_gl_tex)
	{
		glDeleteTextures(1, &m_gl_tex);
		m_gl_tex = 0;
	}

	// delete opengl frambuffer
	if (m_gl_fbo)
	{
		glDeleteFramebuffers(1, &m_gl_fbo);
		m_gl_fbo = 0;
	}

	// Release the Direct3D texture
	if (m_d3d_tex)
	{
		m_d3d_tex->Release();
		m_d3d_tex = nullptr;
	}
}

void WmrRenderer::LoadFunctions()
{
}