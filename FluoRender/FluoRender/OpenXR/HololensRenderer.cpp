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
#include <HololensRenderer.h>
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

HololensRenderer::HololensRenderer(const HololensOptions& options) :
	WmrRenderer(),
	m_options(options),
	m_secureConnectionCallbacks(m_options.authenticationToken,
		m_options.allowCertificateNameMismatch,
		m_options.allowUnverifiedCertificateChain,
		m_options.keyPassphrase,
		m_options.subjectName,
		m_options.certificateStore,
		m_options.listen)
{
	m_app_name = "FluoRender";
	m_eng_name = "FLHOLOLENS";

}

HololensRenderer::~HololensRenderer()
{
}

bool HololensRenderer::Init(void* hdc, void* hglrc)
{
	if (m_initialized)
		return m_initialized;

	if (!m_options.isStandalone)
	{
		m_usingRemotingRuntime = EnableRemotingXR();

		if (m_usingRemotingRuntime)
		{
			if (m_options.secureConnection)
			{
				m_secureConnectionCallbacks.InitializeSecureConnection();
			}
		}
		else
		{
#ifdef _DEBUG
			DBGPRINT(L"RemotingXR runtime not available. Running with default OpenXR runtime.\n");
#endif
		}
	}

	SetExtensions();

	// OpenXR initialization
	if (!CreateInstance())
		return false;

	CheckExtensions();

	//load msft specific functions
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

	//PollEvents();

	ConnectOrListen();

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

void HololensRenderer::Close()
{
	DestroySharedTex();
	Disconnect();
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

void HololensRenderer::EndFrame()
{
	if (!m_app_running || !m_session_running)
		return;

	// End the frame
	XrFrameEndInfo frameEndInfo{ XR_TYPE_FRAME_END_INFO };
	frameEndInfo.displayTime = m_frame_state.predictedDisplayTime;
	frameEndInfo.environmentBlendMode = m_env_blend_mode;
	frameEndInfo.layerCount = static_cast<uint32_t>(m_render_layer_info.layers.size());
	frameEndInfo.layers = m_render_layer_info.layers.data();

	//mirror image, may not be necessary
	XrRemotingFrameMirrorImageD3D11MSFT mirrorImageD3D11{
		static_cast<XrStructureType>(XR_TYPE_REMOTING_FRAME_MIRROR_IMAGE_D3D11_MSFT) };
	ID3D11Texture2D* texture;
	m_mirror_sch->GetBuffer(0, IID_PPV_ARGS(&texture));
	mirrorImageD3D11.texture = texture;
	XrRemotingFrameMirrorImageInfoMSFT mirrorImageEndInfo{
		static_cast<XrStructureType>(XR_TYPE_REMOTING_FRAME_MIRROR_IMAGE_INFO_MSFT) };
	mirrorImageEndInfo.image = reinterpret_cast<const XrRemotingFrameMirrorImageBaseHeaderMSFT*>(&mirrorImageD3D11);
	frameEndInfo.next = 0;// &mirrorImageEndInfo;

	XrResult result = xrEndFrame(m_session, &frameEndInfo);
	if (result != XR_SUCCESS)
	{
#ifdef _DEBUG
		DBGPRINT(L"xrEndFrame failed.\n");
#endif
	}
}

void HololensRenderer::SetExtensions()
{
	// Add additional instance layers/extensions that the application wants.
	// Add both required and requested instance extensions.
	m_instanceExtensions.push_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);
	// D3D11 extension is required for this sample, so check if it's supported.
	m_instanceExtensions.push_back(XR_KHR_D3D11_ENABLE_EXTENSION_NAME);
	if (m_usingRemotingRuntime)
	{
		// If using the remoting runtime, the remoting extension must be present as well
		m_instanceExtensions.push_back(XR_MSFT_HOLOGRAPHIC_REMOTING_EXTENSION_NAME);
		m_instanceExtensions.push_back(XR_MSFT_HOLOGRAPHIC_REMOTING_FRAME_MIRRORING_EXTENSION_NAME);
		m_instanceExtensions.push_back(XR_MSFT_HOLOGRAPHIC_REMOTING_SPEECH_EXTENSION_NAME);
	}
	// Additional optional extensions for enhanced functionality. Track whether enabled in m_optionalExtensions.
	m_instanceExtensions.push_back(XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME);
	m_instanceExtensions.push_back(XR_MSFT_UNBOUNDED_REFERENCE_SPACE_EXTENSION_NAME);
	m_instanceExtensions.push_back(XR_MSFT_SPATIAL_ANCHOR_EXTENSION_NAME);
}

void HololensRenderer::LoadFunctions()
{
	WmrRenderer::LoadFunctions();

	XrResult result;
	// Get the function address
	result = xrGetInstanceProcAddr(m_instance, "xrRemotingSetContextPropertiesMSFT", (PFN_xrVoidFunction*)&xrRemotingSetContextPropertiesMSFT);
	result = xrGetInstanceProcAddr(m_instance, "xrRemotingConnectMSFT", (PFN_xrVoidFunction*)&xrRemotingConnectMSFT);
	result = xrGetInstanceProcAddr(m_instance, "xrRemotingListenMSFT", (PFN_xrVoidFunction*)&xrRemotingListenMSFT);
	result = xrGetInstanceProcAddr(m_instance, "xrRemotingDisconnectMSFT", (PFN_xrVoidFunction*)&xrRemotingDisconnectMSFT);
	result = xrGetInstanceProcAddr(m_instance, "xrRemotingGetConnectionStateMSFT", (PFN_xrVoidFunction*)&xrRemotingGetConnectionStateMSFT);
	result = xrGetInstanceProcAddr(m_instance, "xrRemotingSetSecureConnectionClientCallbacksMSFT", (PFN_xrVoidFunction*)&xrRemotingSetSecureConnectionClientCallbacksMSFT);
	result = xrGetInstanceProcAddr(m_instance, "xrRemotingSetSecureConnectionServerCallbacksMSFT", (PFN_xrVoidFunction*)&xrRemotingSetSecureConnectionServerCallbacksMSFT);
	result = xrGetInstanceProcAddr(m_instance, "xrInitializeRemotingSpeechMSFT", (PFN_xrVoidFunction*)&xrInitializeRemotingSpeechMSFT);
	result = xrGetInstanceProcAddr(m_instance, "xrRetrieveRemotingSpeechRecognizedTextMSFT", (PFN_xrVoidFunction*)&xrRetrieveRemotingSpeechRecognizedTextMSFT);
}

bool HololensRenderer::CreateReferenceSpace()
{
	XrResult result;
	// Fill out an XrReferenceSpaceCreateInfo structure and create a reference XrSpace, specifying a Local space with an identity pose as the origin.
	XrReferenceSpaceCreateInfo referenceSpaceCI{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
	if (m_optionalExtensions.UnboundedRefSpaceSupported)
	{
		// Unbounded reference space provides the best app space for world-scale experiences.
		referenceSpaceCI.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT;
	}
	else
	{
		// If running on a platform that does not support world-scale experiences, fall back to local space.
		referenceSpaceCI.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
	}
	referenceSpaceCI.poseInReferenceSpace = { {0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f} };
	result = xrCreateReferenceSpace(m_session, &referenceSpaceCI, &m_space);
	if (result != XR_SUCCESS) return false;

	return true;
}

void HololensRenderer::PollEvents()
{
	// Poll OpenXR for a new event.
	XrEventDataBuffer eventData{ XR_TYPE_EVENT_DATA_BUFFER };
	auto XrPollEvents = [&]() -> bool
	{
		eventData = { XR_TYPE_EVENT_DATA_BUFFER };
		return xrPollEvent(m_instance, &eventData) == XR_SUCCESS;
	};

	while (XrPollEvents())
	{
		switch (eventData.type)
		{
		case XR_TYPE_EVENT_DATA_EVENTS_LOST:
		{
			XrEventDataEventsLost* eventsLost =
				reinterpret_cast<XrEventDataEventsLost*>(&eventData);
#ifdef _DEBUG
			DBGPRINT(L"OPENXR: Events Lost: %d\n", eventsLost->lostEventCount);
#endif
			break;
		}
		// Log that an instance loss is pending and shutdown the application.
		case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
		{
			XrEventDataInstanceLossPending* instanceLossPending =
				reinterpret_cast<XrEventDataInstanceLossPending*>(&eventData);
#ifdef _DEBUG
			DBGPRINT(L"OPENXR: Instance Loss Pending at: %llu\n", instanceLossPending->lossTime);
#endif
			m_session_running = false;
			m_app_running = false;
			break;
		}
		// Log that the interaction profile has changed.
		case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
		{
			XrEventDataInteractionProfileChanged* interactionProfileChanged =
				reinterpret_cast<XrEventDataInteractionProfileChanged*>(&eventData);
#ifdef _DEBUG
			DBGPRINT(L"OPENXR: Interaction Profile changed for Session: %llu\n", interactionProfileChanged->session);
#endif
			if (interactionProfileChanged->session != m_session)
			{
#ifdef _DEBUG
				DBGPRINT(L"XrEventDataInteractionProfileChanged for unknown Session\n");
#endif
				break;
			}
			RecordCurrentBindings();
			break;
		}
		// Log that there's a reference space change pending.
		case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING:
		{
			XrEventDataReferenceSpaceChangePending* referenceSpaceChangePending =
				reinterpret_cast<XrEventDataReferenceSpaceChangePending*>(&eventData);
#ifdef _DEBUG
			DBGPRINT(L"OPENXR: Reference Space Change pending for Session: %llu\n", referenceSpaceChangePending->session);
#endif
			if (referenceSpaceChangePending->session != m_session)
			{
#ifdef _DEBUG
				DBGPRINT(L"XrEventDataReferenceSpaceChangePending for unknown Session\n");
#endif
				break;
			}
			break;
		}
		// Session State changes:
		case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
		{
			XrEventDataSessionStateChanged* sessionStateChanged =
				reinterpret_cast<XrEventDataSessionStateChanged*>(&eventData);
			if (sessionStateChanged->session != m_session)
			{
#ifdef _DEBUG
				DBGPRINT(L"XrEventDataSessionStateChanged for unknown Session\n");
#endif
				break;
			}

			if (sessionStateChanged->state == XR_SESSION_STATE_READY)
			{
				// SessionState is ready. Begin the XrSession using the XrViewConfigurationType.
				XrSessionBeginInfo sessionBeginInfo{ XR_TYPE_SESSION_BEGIN_INFO };
				sessionBeginInfo.primaryViewConfigurationType = m_view_config;
				xrBeginSession(m_session, &sessionBeginInfo);
				m_session_running = true;
			}
			if (sessionStateChanged->state == XR_SESSION_STATE_STOPPING)
			{
				// SessionState is stopping. End the XrSession.
				xrEndSession(m_session);
				m_session_running = false;
			}
			if (sessionStateChanged->state == XR_SESSION_STATE_EXITING)
			{
				// SessionState is exiting. Exit the application.
				m_session_running = false;
				m_app_running = false;
			}
			if (sessionStateChanged->state == XR_SESSION_STATE_LOSS_PENDING)
			{
				// SessionState is loss pending. Exit the application.
				// It's possible to try a reestablish an XrInstance and XrSession, but we will simply exit here.
				m_session_running = false;
				m_app_running = false;
			}
			// Store state for reference across the application.
			m_session_state = sessionStateChanged->state;
			break;
		}
		case XR_TYPE_REMOTING_EVENT_DATA_LISTENING_MSFT:
		{
#ifdef _DEBUG
			DBGPRINT(L"Holographic Remoting: Listening on port %d",
				reinterpret_cast<const XrRemotingEventDataListeningMSFT*>(&eventData)->listeningPort);
#endif
			break;
		}
		case XR_TYPE_REMOTING_EVENT_DATA_CONNECTED_MSFT:
		{
#ifdef _DEBUG
			DBGPRINT(L"Holographic Remoting: Connected.");
#endif

			// If remoting speech extension is enabled
			if (m_usingRemotingRuntime)
			{
				XrRemotingSpeechInitInfoMSFT speechInitInfo{ static_cast<XrStructureType>(XR_TYPE_REMOTING_SPEECH_INIT_INFO_MSFT) };
				InitializeSpeechRecognition(speechInitInfo);

				XrResult result = xrInitializeRemotingSpeechMSFT(m_session, &speechInitInfo);
			}

			break;
		}
		case XR_TYPE_REMOTING_EVENT_DATA_DISCONNECTED_MSFT:
		{
#ifdef _DEBUG
			DBGPRINT(L"Holographic Remoting: Disconnected - Reason: %d",
				reinterpret_cast<const XrRemotingEventDataDisconnectedMSFT*>(&eventData)->disconnectReason);
#endif
			break;
		}
		case XR_TYPE_EVENT_DATA_REMOTING_SPEECH_RECOGNIZED_MSFT:
		{
			auto speechEventData = reinterpret_cast<const XrEventDataRemotingSpeechRecognizedMSFT*>(&eventData);
			std::string text;
			uint32_t dataBytesCount = 0;
			XrResult result = xrRetrieveRemotingSpeechRecognizedTextMSFT(
				m_session, speechEventData->packetId, 0, &dataBytesCount, nullptr);
			text.resize(dataBytesCount);
			result = xrRetrieveRemotingSpeechRecognizedTextMSFT(
				m_session, speechEventData->packetId, static_cast<uint32_t>(text.size()), &dataBytesCount, text.data());
			HandleRecognizedSpeechText(text);
			break;
		}
		case XR_TYPE_EVENT_DATA_REMOTING_SPEECH_RECOGNIZER_STATE_CHANGED_MSFT:
		{
			auto recognizerStateEventData =
				reinterpret_cast<const XrEventDataRemotingSpeechRecognizerStateChangedMSFT*>(&eventData);
			auto state = recognizerStateEventData->speechRecognizerState;
			if (strlen(recognizerStateEventData->stateMessage) > 0)
			{
#ifdef _DEBUG
				DBGPRINT(L"Speech recognizer initialization error: %s.", recognizerStateEventData->stateMessage);
#endif
			}

			if (state == XR_REMOTING_SPEECH_RECOGNIZER_STATE_INITIALIZATION_FAILED_MSFT)
			{
#ifdef _DEBUG
				DBGPRINT(L"Remoting speech recognizer initialization failed.");
#endif
			}
			break;
		}
		default:
		{
			break;
		}
		}
	}
}

// Window procedure function
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool HololensRenderer::CreateSharedTex(const XrSwapchainCreateInfo& scci)
{
	//WmrRenderer::CreateSharedTex(scci);

	DXGI_SWAP_CHAIN_DESC1 desc{};
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.Stereo = false;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = 2;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	desc.Flags = 0;
	desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	desc.Scaling = DXGI_SCALING_STRETCH;

	IDXGIDevice1* dxgiDevice;
	m_device->QueryInterface(&dxgiDevice);

	IDXGIAdapter* dxgiAdapter;
	dxgiDevice->GetAdapter(&dxgiAdapter);

	IDXGIFactory2* dxgiFactory;
	dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));

	// Function to create a dummy window
	const wchar_t CLASS_NAME[] = L"DummyWindowClass";
	HINSTANCE hInstance = GetModuleHandle(NULL);

	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		CLASS_NAME,                     // Window class
		L"Dummy Window",                // Window text
		WS_OVERLAPPEDWINDOW,            // Window style
		CW_USEDEFAULT, CW_USEDEFAULT,   // Size and position
		CW_USEDEFAULT, CW_USEDEFAULT,   // Width and height
		NULL,                           // Parent window    
		NULL,                           // Menu
		hInstance,                      // Instance handle
		NULL                            // Additional application data
	);

	if (hwnd == NULL) {
		return NULL;
	}

	// Hide the window
	ShowWindow(hwnd, SW_HIDE);
	dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
	dxgiFactory->CreateSwapChainForHwnd(m_device, hwnd, &desc, nullptr, nullptr, &m_mirror_sch);

	return true;
}

void HololensRenderer::DestroySharedTex()
{
	WmrRenderer::DestroySharedTex();
	//if (m_mirror_tex)
	//{
	//	m_mirror_tex->Release();
	//	m_mirror_tex = nullptr;
	//}
}

bool HololensRenderer::EnableRemotingXR()
{
	wchar_t executablePath[MAX_PATH];
	if (GetModuleFileNameW(NULL, executablePath, ARRAYSIZE(executablePath)) == 0)
	{
		return false;
	}

	std::filesystem::path filename(executablePath);
	filename = filename.replace_filename("RemotingXR.json");

	if (std::filesystem::exists(filename))
	{
		SetEnvironmentVariableW(L"XR_RUNTIME_JSON", filename.c_str());
		return true;
	}

	return false;
}

void HololensRenderer::CheckExtensions()
{
	// Add a specific extension to the list of extensions to be enabled, if it is supported.
	auto CheckExtension = [&](const char* extensionName)
	{
		for (uint32_t i = 0; i < m_extensionCount; i++)
		{
			if (strcmp(m_extensionProperties[i].extensionName, extensionName) == 0)
			{
				//m_instanceExtensions.push_back(extensionName);
				return true;
			}
		}
		return false;
	};

	m_optionalExtensions.DepthExtensionSupported = CheckExtension(XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME);
	m_optionalExtensions.UnboundedRefSpaceSupported = CheckExtension(XR_MSFT_UNBOUNDED_REFERENCE_SPACE_EXTENSION_NAME);
	m_optionalExtensions.SpatialAnchorSupported = CheckExtension(XR_MSFT_SPATIAL_ANCHOR_EXTENSION_NAME);
}

void HololensRenderer::Disconnect()
{
	XrRemotingDisconnectInfoMSFT disconnectInfo{ static_cast<XrStructureType>(XR_TYPE_REMOTING_DISCONNECT_INFO_MSFT) };
	XrResult result = xrRemotingDisconnectMSFT(m_instance, m_sys_id, &disconnectInfo);
}

void HololensRenderer::ConnectOrListen()
{
	if (!m_usingRemotingRuntime)
	{
		return;
	}

	XrResult result;
	XrRemotingConnectionStateMSFT connectionState;
	result = xrRemotingGetConnectionStateMSFT(m_instance, m_sys_id, &connectionState, nullptr);
	if (connectionState != XR_REMOTING_CONNECTION_STATE_DISCONNECTED_MSFT)
	{
		return;
	}

	// Apply remote context properties while disconnected.
	{
		XrRemotingRemoteContextPropertiesMSFT contextProperties;
		contextProperties =
			XrRemotingRemoteContextPropertiesMSFT{ static_cast<XrStructureType>(XR_TYPE_REMOTING_REMOTE_CONTEXT_PROPERTIES_MSFT) };
		contextProperties.enableAudio = false;
		contextProperties.maxBitrateKbps = 20000;
		contextProperties.videoCodec = XR_REMOTING_VIDEO_CODEC_H265_MSFT;
		contextProperties.depthBufferStreamResolution = XR_REMOTING_DEPTH_BUFFER_STREAM_RESOLUTION_HALF_MSFT;
		result = xrRemotingSetContextPropertiesMSFT(m_instance, m_sys_id, &contextProperties);
	}

	if (m_options.listen)
	{
		if (m_options.secureConnection)
		{
			XrRemotingSecureConnectionServerCallbacksMSFT serverCallbacks;
			serverCallbacks.context = this;
			serverCallbacks.requestServerCertificateCallback = CertificateValidationCallbackStatic;
			serverCallbacks.validateAuthenticationTokenCallback = AuthenticationValidationCallbackStatic;
			serverCallbacks.authenticationRealm = m_options.authenticationRealm.c_str();
			result = xrRemotingSetSecureConnectionServerCallbacksMSFT(m_instance, m_sys_id, &serverCallbacks);
		}

		XrRemotingListenInfoMSFT listenInfo{ static_cast<XrStructureType>(XR_TYPE_REMOTING_LISTEN_INFO_MSFT) };
		listenInfo.listenInterface = m_options.host.empty() ? "0.0.0.0" : m_options.host.c_str();
		listenInfo.handshakeListenPort = m_options.port != 0 ? m_options.port : 8265;
		listenInfo.transportListenPort = m_options.transportPort != 0 ? m_options.transportPort : 8266;
		listenInfo.secureConnection = m_options.secureConnection;
		result = xrRemotingListenMSFT(m_instance, m_sys_id, &listenInfo);
	}
	else
	{
		if (m_options.secureConnection)
		{
			XrRemotingSecureConnectionClientCallbacksMSFT clientCallbacks;
			clientCallbacks.context = this;
			clientCallbacks.requestAuthenticationTokenCallback = AuthenticationRequestCallbackStatic;
			clientCallbacks.validateServerCertificateCallback = CertificateValidationCallbackStatic;
			clientCallbacks.performSystemValidation = true;
			result = xrRemotingSetSecureConnectionClientCallbacksMSFT(m_instance, m_sys_id, &clientCallbacks);
		}

		XrRemotingConnectInfoMSFT connectInfo{ static_cast<XrStructureType>(XR_TYPE_REMOTING_CONNECT_INFO_MSFT) };
		connectInfo.remoteHostName = m_options.host.empty() ? "127.0.0.1" : m_options.host.c_str();
		connectInfo.remotePort = m_options.port != 0 ? m_options.port : 8265;
		connectInfo.secureConnection = m_options.secureConnection;
		result = xrRemotingConnectMSFT(m_instance, m_sys_id, &connectInfo);
	}
}

XrResult HololensRenderer::AuthenticationRequestCallback(XrRemotingAuthenticationTokenRequestMSFT* authenticationTokenRequest)
{
	const std::string tokenUtf8 = m_options.authenticationToken;
	const uint32_t tokenSize = static_cast<uint32_t>(tokenUtf8.size() + 1); // for null-termination
	if (authenticationTokenRequest->tokenCapacityIn >= tokenSize)
	{
		memcpy(authenticationTokenRequest->tokenBuffer, tokenUtf8.c_str(), tokenSize);
		authenticationTokenRequest->tokenSizeOut = tokenSize;
		return XR_SUCCESS;
	}
	else
	{
		authenticationTokenRequest->tokenSizeOut = tokenSize;
		return XR_ERROR_SIZE_INSUFFICIENT;
	}
}

XrResult XRAPI_CALL
HololensRenderer::AuthenticationRequestCallbackStatic(XrRemotingAuthenticationTokenRequestMSFT* authenticationTokenRequest)
{
	if (!authenticationTokenRequest->context)
	{
		return XR_ERROR_RUNTIME_FAILURE;
	}

	return reinterpret_cast<HololensRenderer*>(authenticationTokenRequest->context)
		->AuthenticationRequestCallback(authenticationTokenRequest);
}

XrResult HololensRenderer::AuthenticationValidationCallback(XrRemotingAuthenticationTokenValidationMSFT* authenticationTokenValidation)
{
	const std::string tokenUtf8 = m_options.authenticationToken;
	authenticationTokenValidation->tokenValidOut =
		(authenticationTokenValidation->token != nullptr && tokenUtf8 == authenticationTokenValidation->token);
	return XR_SUCCESS;
}

XrResult XRAPI_CALL
HololensRenderer::AuthenticationValidationCallbackStatic(XrRemotingAuthenticationTokenValidationMSFT* authenticationTokenValidation)
{
	if (!authenticationTokenValidation->context)
	{
		return XR_ERROR_RUNTIME_FAILURE;
	}

	return reinterpret_cast<HololensRenderer*>(authenticationTokenValidation->context)
		->AuthenticationValidationCallback(authenticationTokenValidation);
}

XrResult HololensRenderer::CertificateRequestCallback(XrRemotingServerCertificateRequestMSFT* serverCertificateRequest)
{
	const std::string subjectNameUtf8 = m_options.subjectName;
	const std::string passPhraseUtf8 = m_options.keyPassphrase;

	const uint32_t certStoreSize = static_cast<uint32_t>(m_certificateStore.size());
	const uint32_t subjectNameSize = static_cast<uint32_t>(subjectNameUtf8.size() + 1); // for null-termination
	const uint32_t passPhraseSize = static_cast<uint32_t>(passPhraseUtf8.size() + 1);   // for null-termination

	serverCertificateRequest->certStoreSizeOut = certStoreSize;
	serverCertificateRequest->subjectNameSizeOut = subjectNameSize;
	serverCertificateRequest->keyPassphraseSizeOut = passPhraseSize;
	if (serverCertificateRequest->certStoreCapacityIn < certStoreSize ||
		serverCertificateRequest->subjectNameCapacityIn < subjectNameSize ||
		serverCertificateRequest->keyPassphraseCapacityIn < passPhraseSize)
	{
		return XR_ERROR_SIZE_INSUFFICIENT;
	}

	// All buffers have sufficient size, so fill in the data
	memcpy(serverCertificateRequest->certStoreBuffer, m_certificateStore.data(), certStoreSize);
	memcpy(serverCertificateRequest->subjectNameBuffer, subjectNameUtf8.c_str(), subjectNameSize);
	memcpy(serverCertificateRequest->keyPassphraseBuffer, passPhraseUtf8.c_str(), passPhraseSize);

	return XR_SUCCESS;
}

XrResult XRAPI_CALL HololensRenderer::CertificateValidationCallbackStatic(XrRemotingServerCertificateRequestMSFT* serverCertificateRequest)
{
	if (!serverCertificateRequest->context)
	{
		return XR_ERROR_RUNTIME_FAILURE;
	}

	return reinterpret_cast<HololensRenderer*>(serverCertificateRequest->context)
		->CertificateRequestCallback(serverCertificateRequest);
}

XrResult HololensRenderer::CertificateValidationCallback(XrRemotingServerCertificateValidationMSFT* serverCertificateValidation)
{
	if (!serverCertificateValidation->systemValidationResult)
	{
		return XR_ERROR_RUNTIME_FAILURE; // We requested system validation to be performed
	}

	serverCertificateValidation->validationResultOut = *serverCertificateValidation->systemValidationResult;
	if (m_options.allowCertificateNameMismatch && serverCertificateValidation->validationResultOut.nameValidationResult ==
		XR_REMOTING_CERTIFICATE_NAME_VALIDATION_RESULT_MISMATCH_MSFT)
	{
		serverCertificateValidation->validationResultOut.nameValidationResult =
			XR_REMOTING_CERTIFICATE_NAME_VALIDATION_RESULT_MATCH_MSFT;
	}
	if (m_options.allowUnverifiedCertificateChain)
	{
		serverCertificateValidation->validationResultOut.trustedRoot = true;
	}

	return XR_SUCCESS;
}

XrResult XRAPI_CALL
HololensRenderer::CertificateValidationCallbackStatic(XrRemotingServerCertificateValidationMSFT* serverCertificateValidation)
{
	if (!serverCertificateValidation->context)
	{
		return XR_ERROR_RUNTIME_FAILURE;
	}

	return reinterpret_cast<HololensRenderer*>(serverCertificateValidation->context)
		->CertificateValidationCallback(serverCertificateValidation);
}

void HololensRenderer::InitializeSpeechRecognition(XrRemotingSpeechInitInfoMSFT& speechInitInfo)
{
	// Specify the speech recognition language.
	strcpy_s(speechInitInfo.language, "en-US");

	// Initialize the dictionary.
	m_dictionaryEntries = { "Red", "Blue", "Green", "Aquamarine", "Default" };
	speechInitInfo.dictionaryEntries = m_dictionaryEntries.data();
	speechInitInfo.dictionaryEntriesCount = static_cast<uint32_t>(m_dictionaryEntries.size());

	// Initialize the grammar file if it exists.
	if (LoadGrammarFile(m_grammarFileContent))
	{
		speechInitInfo.grammarFileSize = static_cast<uint32_t>(m_grammarFileContent.size());
		speechInitInfo.grammarFileContent = m_grammarFileContent.data();
	}
}

bool HololensRenderer::LoadGrammarFile(std::vector<uint8_t>& grammarFileContent)
{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	char executablePath[MAX_PATH];
	if (GetModuleFileNameA(NULL, executablePath, ARRAYSIZE(executablePath)) == 0)
	{
		return false;
	}

	std::filesystem::path filename(executablePath);
	filename = filename.replace_filename("OpenXRSpeechGrammar.xml");

	if (!std::filesystem::exists(filename))
	{
		return false;
	}

	std::string grammarFilePath{ filename.string() };
	std::ifstream grammarFileStream(grammarFilePath, std::ios::binary);
	const size_t grammarFileSize = std::filesystem::file_size(filename);
	if (!grammarFileStream.good() || grammarFileSize == 0)
	{
		return false;
	}

	grammarFileContent.resize(grammarFileSize);
	grammarFileStream.read(reinterpret_cast<char*>(grammarFileContent.data()), grammarFileSize);
	if (grammarFileStream.fail())
	{
		return false;
	}

	return true;
#else
	return false;
#endif
}

void HololensRenderer::HandleRecognizedSpeechText(const std::string& text)
{

}