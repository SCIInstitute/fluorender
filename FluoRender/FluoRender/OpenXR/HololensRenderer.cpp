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
	WmrRenderer()
	,m_options(options)
#ifdef _WIN32
	,m_secureConnectionCallbacks(m_options.authenticationToken,
		m_options.allowCertificateNameMismatch,
		m_options.allowUnverifiedCertificateChain,
		m_options.keyPassphrase,
		m_options.subjectName,
		m_options.certificateStore,
		m_options.listen)
#endif
{
	m_app_name = "FluoRender";
	m_eng_name = "FLHOLOLENS";

	m_dead_zone = 0.0f;
	m_scaler = 1.0f;

	m_use_depth = true;

#ifdef _WIN32
	m_preferred_color_formats =
	{
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		DXGI_FORMAT_B8G8R8A8_UNORM,
		DXGI_FORMAT_R8G8B8A8_UNORM,
	};
	m_preferred_depth_formats =
	{
		DXGI_FORMAT_D32_FLOAT,
		DXGI_FORMAT_D16_UNORM,
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
	};
#endif
}

HololensRenderer::~HololensRenderer()
{
}

bool HololensRenderer::Init(void* hdc, void* hglrc)
{
	if (m_initialized)
		return m_initialized;

#ifdef _WIN32
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
#endif

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

#ifdef _WIN32
	if (!ConnectOrListen())
		return false;
#endif

	if (!CreateSession(hdc, hglrc))
		return false;

	CreateActionPoses();
	AttachActionSet();

	if (!CreateReferenceSpace())
		return false;

	if (!CreateSwapchains())
		return false;

	//create shared tex if not present
	CreateSharedTex();

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

void HololensRenderer::SetExtensions()
{
#ifdef _WIN32
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
#endif
	//hand tracking
	m_instanceExtensions.push_back(XR_EXT_HAND_TRACKING_EXTENSION_NAME);
}

void HololensRenderer::LoadFunctions()
{
	WmrRenderer::LoadFunctions();

#ifdef _WIN32
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
#endif
}

bool HololensRenderer::CreateReferenceSpace()
{
#ifdef _WIN32
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
#endif
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
#ifdef _WIN32
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
#endif
		default:
		{
			break;
		}
		}
	}
}

#ifdef _WIN32
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

bool HololensRenderer::ConnectOrListen()
{
	if (!m_usingRemotingRuntime)
	{
		return false;
	}

	XrResult result;
	XrRemotingConnectionStateMSFT connectionState;
	result = xrRemotingGetConnectionStateMSFT(m_instance, m_sys_id, &connectionState, nullptr);
	if (result != XR_SUCCESS)
		return false;
	if (connectionState != XR_REMOTING_CONNECTION_STATE_DISCONNECTED_MSFT)
	{
		return false;
	}

	// Apply remote context properties while disconnected.
	XrRemotingRemoteContextPropertiesMSFT contextProperties;
	contextProperties =
		XrRemotingRemoteContextPropertiesMSFT{ static_cast<XrStructureType>(XR_TYPE_REMOTING_REMOTE_CONTEXT_PROPERTIES_MSFT) };
	contextProperties.enableAudio = false;
	contextProperties.maxBitrateKbps = 20000;
	contextProperties.videoCodec = XR_REMOTING_VIDEO_CODEC_H265_MSFT;
	contextProperties.depthBufferStreamResolution = XR_REMOTING_DEPTH_BUFFER_STREAM_RESOLUTION_HALF_MSFT;
	result = xrRemotingSetContextPropertiesMSFT(m_instance, m_sys_id, &contextProperties);
	if (result != XR_SUCCESS)
		return false;

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
		if (result != XR_SUCCESS)
			return false;
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
		if (result != XR_SUCCESS)
			return false;
	}

	return true;
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
	m_command_table = {
		//move closer
		{"closer", "move_closer"},
		{"move closer", "move_closer"},
		{"come", "move_closer"},
		{"come nearer", "move_closer"},
		{"nearer", "move_closer"},
		{"near", "move_closer"},
		{"go near", "move_closer"},
		{"approach", "move_closer"},
		{"come closer", "move_closer"},
		{"get closer", "move_closer"},
		{"move near", "move_closer"},
		{"step closer", "move_closer"},
		{"step near", "move_closer"},
		//move away
		{"away", "move_away"},
		{"move away", "move_away"},
		{"go", "move_away"},
		{"go farther", "move_away"},
		{"farther", "move_away"},
		{"far", "move_away"},
		{"go far", "move_away"},
		{"retreat", "move_away"},
		{"back", "move_away"},
		{"move back", "move_away"},
		{"step back", "move_away"},
		{"back off", "move_away"},
		{"get away", "move_away"},
		{"move farther", "move_away"},
		//stop
		{"stop", "stop"},
		{"stay", "stop"},
		{"halt", "stop"},
		{"cease", "stop"},
		{"pause", "stop"},
		{"hold", "stop"},
		{"freeze", "stop"},
		//rotate left
		{"left", "rotate_left"},
		{"rotate left", "rotate_left"},
		{"roll left", "rotate_left"},
		{"turn left", "rotate_left"},
		{"spin left", "rotate_left"},
		//rotate right
		{"right", "rotate_right"},
		{"rotate right", "rotate_right"},
		{"roll right", "rotate_right"},
		{"turn right", "rotate_right"},
		{"spin right", "rotate_right"},
		{"rotate", "rotate_right"},
		{"roll", "rotate_right"},
		{"turn", "rotate_right"},
		{"spin", "rotate_right"},
		//rotate up
		{"up", "rotate_up"},
		{"rotate up", "rotate_up"},
		{"roll up", "rotate_up"},
		{"turn up", "rotate_up"},
		{"spin up", "rotate_up"},
		//rotate down
		{"down", "rotate_down"},
		{"rotate down", "rotate_down"},
		{"roll down", "rotate_down"},
		{"turn down", "rotate_down"},
		{"spin down", "rotate_down"}
	};
	for (const auto& pair : m_command_table)
	{
		m_dictionaryEntries.push_back(pair.first.c_str());
	}
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
	std::string cstr = text;
	std::transform(cstr.begin(), cstr.end(), cstr.begin(), ::tolower);

	auto it = m_command_table.find(cstr);
	if (it != m_command_table.end())
	{
		if (it->second == "move_closer")
		{
			if (m_left_y > 0.0f)
				m_left_y += 1.0f;
			else
				m_left_y = 1.0f;
		}
		else if (it->second == "move_away")
		{
			if (m_left_y < 0.0f)
				m_left_y -= 1.0f;
			else
				m_left_y = -1.0f;
		}
		else if (it->second == "stop")
		{
			m_left_y = 0.0f;
			m_right_x = 0.0f;
			m_right_y = 0.0f;
		}
		else if (it->second == "rotate_left")
		{
			if (m_right_x < 0.0f)
				m_right_x -= 1.0f;
			else
				m_right_x = -1.0f;
		}
		else if (it->second == "rotate_right")
		{
			if (m_right_x > 0.0f)
				m_right_x += 1.0f;
			else
				m_right_x = 1.0f;
		}
		else if (it->second == "rotate_up")
		{
			if (m_right_y > 0.0f)
				m_right_y += 1.0f;
			else
				m_right_y = 1.0f;
		}
		else if (it->second == "rotate_down")
		{
			if (m_right_y < 0.0f)
				m_right_y -= 1.0f;
			else
				m_right_y = -1.0f;
		}
	}
	else
	{
#ifdef _DEBUG
		DBGPRINT(L"Unrecognized command: %s\n", s2ws(text));
#endif
	}
}
#endif