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

#ifndef HololensRenderer_h
#define HololensRenderer_h

#include <OpenXrRenderer.h>
#include <openxr/openxr_msft_holographic_remoting.h>
#include <openxr/openxr_msft_remoting_frame_mirroring.h>
#include <openxr/openxr_msft_remoting_speech.h>

struct AppOptions
{
	bool listen{ false };
	std::string host;
	uint16_t port{ 0 };
	uint16_t transportPort{ 0 };
	bool isStandalone = false;
	bool noUserWait = false;
	bool useEphemeralPort = false;
	bool secureConnection{ false };
	std::string authenticationToken;
	bool allowCertificateNameMismatch{ false };
	bool allowUnverifiedCertificateChain{ false };
	std::string certificateStore;
	std::string keyPassphrase;
	std::string subjectName;
	std::string authenticationRealm{ "OpenXR Remoting" };
};

class HololensRenderer : public OpenXrRenderer
{
public:
	HololensRenderer();
	virtual ~HololensRenderer();

	bool Init(void*, void*) override;
	void Close() override;

	void GetControllerStates() override;

	void BeginFrame() override;
	void EndFrame() override;
	void Draw(const std::vector<flvr::Framebuffer*> &fbos) override;

private:
	void SetExtensions() override;
	bool CreateSession(void* hdc, void* hdxrc) override;

	void LoadFunctions();

	bool EnableRemotingXR();
	void PrepareRemotingEnvironment();

	void InitDevice();
	void Disconnect();
	void ConnectOrListen();

	XrResult AuthenticationRequestCallback(XrRemotingAuthenticationTokenRequestMSFT* authenticationTokenRequest);
	static XrResult XRAPI_CALL AuthenticationRequestCallbackStatic(XrRemotingAuthenticationTokenRequestMSFT* authenticationTokenRequest);
	XrResult AuthenticationValidationCallback(XrRemotingAuthenticationTokenValidationMSFT* authenticationTokenValidation);
	static XrResult XRAPI_CALL AuthenticationValidationCallbackStatic(XrRemotingAuthenticationTokenValidationMSFT* authenticationTokenValidation);
	XrResult CertificateRequestCallback(XrRemotingServerCertificateRequestMSFT* serverCertificateRequest);
	static XrResult XRAPI_CALL CertificateValidationCallbackStatic(XrRemotingServerCertificateRequestMSFT* serverCertificateRequest);
	XrResult CertificateValidationCallback(XrRemotingServerCertificateValidationMSFT* serverCertificateValidation);
	static XrResult XRAPI_CALL CertificateValidationCallbackStatic(XrRemotingServerCertificateValidationMSFT* serverCertificateValidation);

	void InitializeSpeechRecognition(XrRemotingSpeechInitInfoMSFT& speechInitInfo);
	bool LoadGrammarFile(std::vector<uint8_t>& grammarFileContent);

private:
	const AppOptions m_options;
	bool m_usingRemotingRuntime{ false };
	std::vector<uint8_t> m_certificateStore;
	std::vector<const char*> m_dictionaryEntries;
	std::vector<uint8_t> m_grammarFileContent;

	PFN_xrRemotingSetContextPropertiesMSFT xrRemotingSetContextPropertiesMSFT;
	PFN_xrRemotingConnectMSFT xrRemotingConnectMSFT;
	PFN_xrRemotingListenMSFT xrRemotingListenMSFT;
	PFN_xrRemotingDisconnectMSFT xrRemotingDisconnectMSFT;
	PFN_xrRemotingGetConnectionStateMSFT xrRemotingGetConnectionStateMSFT;
	PFN_xrRemotingSetSecureConnectionClientCallbacksMSFT xrRemotingSetSecureConnectionClientCallbacksMSFT;
	PFN_xrRemotingSetSecureConnectionServerCallbacksMSFT xrRemotingSetSecureConnectionServerCallbacksMSFT;

	PFN_xrInitializeRemotingSpeechMSFT xrInitializeRemotingSpeechMSFT;
};

#endif//HololensRenderer_h