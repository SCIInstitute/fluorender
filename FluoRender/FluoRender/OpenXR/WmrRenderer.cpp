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

			glBindFramebuffer(GL_READ_FRAMEBUFFER, fbos[i]->id());
			// Read pixels to PBO
			GLuint dest_fbo = (GLuint)(uint64_t)(m_gl_fbo);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest_fbo);
			glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			// Unbind the framebuffers
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

			// Share the texture with Direct3D
			glDXLockObjectsNV(m_device, 1, &m_interop);
			// Copy the shared texture to the swap chain image view
			m_im_context->CopyResource((ID3D11Resource*)(colorSwapchainInfo.imageViews[colorImageIndex]), m_d3d_tex);
			// Unlock the OpenGL texture
			glDXUnlockObjectsNV(m_device, 1, &m_interop);
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
	// Create a std::unique_ptr<GraphicsAPI_...> from the instance and system.
	// This call sets up a graphics API that's suitable for use with OpenXR.
	// Get D3D11 graphics requirements
	PFN_xrGetD3D11GraphicsRequirementsKHR xrGetD3D11GraphicsRequirementsKHR = nullptr;
	result = xrGetInstanceProcAddr(m_instance, "xrGetD3D11GraphicsRequirementsKHR", (PFN_xrVoidFunction*)&xrGetD3D11GraphicsRequirementsKHR);
	if (result != XR_SUCCESS) return false;
	XrGraphicsRequirementsD3D11KHR requirements = { XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR };
	result = xrGetD3D11GraphicsRequirementsKHR(m_instance, m_sys_id, &requirements);
	if (result != XR_SUCCESS) return false;
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

bool WmrRenderer::CreateSwapchains()
{
	XrResult result;
	// Get the supported swapchain formats as an array of int64_t and ordered by runtime preference.
	uint32_t formatCount = 0;
	result = xrEnumerateSwapchainFormats(m_session, 0, &formatCount, nullptr);
	if (result != XR_SUCCESS) return false;
	std::vector<int64_t> formats(formatCount);
	result = xrEnumerateSwapchainFormats(
		m_session, formatCount, &formatCount, formats.data());
	if (result != XR_SUCCESS) return false;

	std::vector<int64_t> supportSFColor = {
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_B8G8R8A8_UNORM,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		DXGI_FORMAT_B8G8R8A8_UNORM_SRGB };
	std::vector<int64_t> supportSFDepth = {
		DXGI_FORMAT_D32_FLOAT,
		DXGI_FORMAT_D16_UNORM };

	const std::vector<int64_t>::const_iterator& itor_color =
		std::find_first_of(formats.begin(), formats.end(),
			std::begin(supportSFColor), std::end(supportSFColor));
	if (itor_color == formats.end())
	{
#ifdef _DEBUG
		DBGPRINT(L"Failed to find color format for Swapchain.\n");
#endif
	}
	const std::vector<int64_t>::const_iterator& itor_depth =
		std::find_first_of(formats.begin(), formats.end(),
			std::begin(supportSFDepth), std::end(supportSFDepth));
	if (itor_depth == formats.end())
	{
#ifdef _DEBUG
		DBGPRINT(L"Failed to find depth format for Swapchain.\n");
#endif
	}

	//Resize the SwapchainInfo to match the number of view in the View Configuration.
	m_swapchain_infos_color.resize(m_view_config_views.size());
	m_swapchain_infos_depth.resize(m_view_config_views.size());

	// Per view, create a color and depth swapchain, and their associated image views.
	for (size_t i = 0; i < m_view_config_views.size(); i++)
	{
		SwapchainInfo& colorSwapchainInfo = m_swapchain_infos_color[i];
		SwapchainInfo& depthSwapchainInfo = m_swapchain_infos_depth[i];

		// Fill out an XrSwapchainCreateInfo structure and create an XrSwapchain.
		// Color.
		XrSwapchainCreateInfo swapchainCI{ XR_TYPE_SWAPCHAIN_CREATE_INFO };
		swapchainCI.createFlags = 0;
		swapchainCI.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainCI.format = *itor_color;
		swapchainCI.sampleCount = m_view_config_views[i].recommendedSwapchainSampleCount;  // Use the recommended values from the XrViewConfigurationView.
		swapchainCI.width = m_view_config_views[i].recommendedImageRectWidth;
		swapchainCI.height = m_view_config_views[i].recommendedImageRectHeight;
		swapchainCI.faceCount = 1;
		swapchainCI.arraySize = 1;
		swapchainCI.mipCount = 1;
		result = xrCreateSwapchain(m_session, &swapchainCI, &colorSwapchainInfo.swapchain);
		if (result != XR_SUCCESS) return false;
		colorSwapchainInfo.swapchainFormat = swapchainCI.format;  // Save the swapchain format for later use.

		// Get the number of images in the color/depth swapchain and allocate Swapchain image data via GraphicsAPI to store the returned array.
		uint32_t colorSwapchainImageCount = 0;
		result = xrEnumerateSwapchainImages(colorSwapchainInfo.swapchain, 0, &colorSwapchainImageCount, nullptr);
		if (result != XR_SUCCESS) return false;
		std::vector<XrSwapchainImageD3D11KHR> swapchain_images;
		swapchain_images.resize(colorSwapchainImageCount, { XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR });
		XrSwapchainImageBaseHeader* colorSwapchainImages = reinterpret_cast<XrSwapchainImageBaseHeader*>(swapchain_images.data());
		result = xrEnumerateSwapchainImages(colorSwapchainInfo.swapchain,
			colorSwapchainImageCount, &colorSwapchainImageCount, colorSwapchainImages);
		if (result != XR_SUCCESS) return false;

		// Per image in the swapchains, fill out a GraphicsAPI::ImageViewCreateInfo structure and create a color/depth image view.
		for (uint32_t j = 0; j < colorSwapchainImageCount; j++)
		{
			colorSwapchainInfo.imageViews.push_back(CreateImageView(0, (void*)(uint64_t)swapchainCI.format, (void*)(uint64_t)swapchain_images[j].texture));
		}

		//create shared tex if not present
		if (m_d3d_tex == nullptr)
		{
			CreateSharedTex(swapchainCI);
		}

		// Depth.
		if (m_use_depth)
		{
			swapchainCI.createFlags = 0;
			swapchainCI.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			swapchainCI.format = *itor_depth;
			swapchainCI.sampleCount = m_view_config_views[i].recommendedSwapchainSampleCount;  // Use the recommended values from the XrViewConfigurationView.
			swapchainCI.width = m_view_config_views[i].recommendedImageRectWidth;
			swapchainCI.height = m_view_config_views[i].recommendedImageRectHeight;
			swapchainCI.faceCount = 1;
			swapchainCI.arraySize = 1;
			swapchainCI.mipCount = 1;
			result = xrCreateSwapchain(m_session, &swapchainCI, &depthSwapchainInfo.swapchain);
			if (result != XR_SUCCESS) return false;
			depthSwapchainInfo.swapchainFormat = swapchainCI.format;  // Save the swapchain format for later use.

			uint32_t depthSwapchainImageCount = 0;
			result = xrEnumerateSwapchainImages(depthSwapchainInfo.swapchain, 0, &depthSwapchainImageCount, nullptr);
			swapchain_images.resize(depthSwapchainImageCount, { XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR });
			XrSwapchainImageBaseHeader* depthSwapchainImages = reinterpret_cast<XrSwapchainImageBaseHeader*>(swapchain_images.data());
			result = xrEnumerateSwapchainImages(depthSwapchainInfo.swapchain,
				depthSwapchainImageCount, &depthSwapchainImageCount, depthSwapchainImages);
			if (result != XR_SUCCESS) return false;

			for (uint32_t j = 0; j < depthSwapchainImageCount; j++)
			{
				depthSwapchainInfo.imageViews.push_back(CreateImageView(1, (void*)(uint64_t)swapchainCI.format, (void*)(uint64_t)swapchain_images[j].texture));
			}
		}
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

	hr = D3D11CreateDevice(
		adapter,
		D3D_DRIVER_TYPE_UNKNOWN,
		0,
		D3D11_CREATE_DEVICE_DEBUG,
		&graphicsRequirements.minFeatureLevel,
		1,
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

bool WmrRenderer::CreateSharedTex(const XrSwapchainCreateInfo& scci)
{
	// Describe the shared texture
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = scci.width;
	desc.Height = scci.height;
	desc.MipLevels = scci.mipCount;
	desc.ArraySize = scci.arraySize;
	desc.Format = static_cast<DXGI_FORMAT>(scci.format);
	desc.SampleDesc.Count = scci.sampleCount;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

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
	// assume the format is correct
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scci.width, scci.width, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	// Register the texture with OpenGL
	m_interop = glDXRegisterObjectNV(m_device, m_d3d_tex, m_gl_tex, GL_TEXTURE_2D, WGL_ACCESS_READ_WRITE_NV);

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

	return true;
}

void WmrRenderer::DestroySharedTex()
{
	// Unregister the OpenGL texture
	if (m_interop)
	{
		glDXUnregisterObjectNV(m_device, m_interop);
	}

	// Delete the OpenGL texture
	if (m_gl_tex)
	{
		glDeleteTextures(1, &m_gl_tex);
		m_gl_tex = 0;
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
	PFNWGLDXREGISTEROBJECTNVPROC glDXRegisterObjectNV =
		(PFNWGLDXREGISTEROBJECTNVPROC)wglGetProcAddress("wglDXRegisterObjectNV");
	PFNWGLDXUNREGISTEROBJECTNVPROC glDXUnregisterObjectNV =
		(PFNWGLDXUNREGISTEROBJECTNVPROC)wglGetProcAddress("wglDXUnregisterObjectNV");
	PFNWGLDXLOCKOBJECTSNVPROC glDXLockObjectsNV =
		(PFNWGLDXLOCKOBJECTSNVPROC)wglGetProcAddress("wglDXLockObjectsNV");
	PFNWGLDXUNLOCKOBJECTSNVPROC glDXUnlockObjectsNV =
		(PFNWGLDXUNLOCKOBJECTSNVPROC)wglGetProcAddress("wglDXUnlockObjectsNV");

}