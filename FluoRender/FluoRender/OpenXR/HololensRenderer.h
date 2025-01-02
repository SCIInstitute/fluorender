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

#include <WmrRenderer.h>
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

class SecureConnectionCallbacks
{
public:
	SecureConnectionCallbacks(const std::string& authenticationToken,
		bool allowCertificateNameMismatch,
		bool allowUnverifiedCertificateChain,
		const std::string& keyPassphrase,
		const std::string& subjectName,
		const std::string& certificateStore,
		bool listen)
		: m_authenticationToken(authenticationToken)
		, m_allowCertificateNameMismatch(allowCertificateNameMismatch)
		, m_allowUnverifiedCertificateChain(allowUnverifiedCertificateChain)
		, m_keyPassphrase(keyPassphrase)
		, m_subjectName(subjectName)
		, m_certificateStoreName(certificateStore)
		, m_listen(listen) {
	}

	void InitializeSecureConnection() {
		if (m_authenticationToken.empty()) {
			throw std::logic_error("Authentication token must be specified for secure connections.");
		}

		if (m_listen) {
			if (m_certificateStoreName.empty() || m_subjectName.empty()) {
				throw std::logic_error("Certificate store and subject name must be specified for secure listening.");
			}

			constexpr size_t maxCertStoreSize = 1 << 20;
			std::ifstream certStoreStream(m_certificateStoreName, std::ios::binary);
			certStoreStream.seekg(0, std::ios_base::end);
			const size_t certStoreSize = certStoreStream.tellg();
			if (!certStoreStream.good() || certStoreSize == 0 || certStoreSize > maxCertStoreSize) {
				throw std::logic_error("Error reading certificate store.");
			}
			certStoreStream.seekg(0, std::ios_base::beg);
			m_certificateStore.resize(certStoreSize);
			certStoreStream.read(reinterpret_cast<char*>(m_certificateStore.data()), certStoreSize);
			if (certStoreStream.fail()) {
				throw std::logic_error("Error reading certificate store.");
			}
		}
	}

	// Static callbacks
	static XrResult XRAPI_CALL
		RequestAuthenticationTokenStaticCallback(XrRemotingAuthenticationTokenRequestMSFT* authenticationTokenRequest) {
		if (!authenticationTokenRequest->context) {
			return XR_ERROR_RUNTIME_FAILURE;
		}
		return reinterpret_cast<SecureConnectionCallbacks*>(authenticationTokenRequest->context)
			->RequestAuthenticationToken(authenticationTokenRequest);
	}

	static XrResult XRAPI_CALL
		ValidateServerCertificateStaticCallback(XrRemotingServerCertificateValidationMSFT* serverCertificateValidation) {
		if (!serverCertificateValidation->context) {
			return XR_ERROR_RUNTIME_FAILURE;
		}
		return reinterpret_cast<SecureConnectionCallbacks*>(serverCertificateValidation->context)
			->ValidateServerCertificate(serverCertificateValidation);
	}

	static XrResult XRAPI_CALL
		ValidateAuthenticationTokenStaticCallback(XrRemotingAuthenticationTokenValidationMSFT* authenticationTokenValidation) {
		if (!authenticationTokenValidation->context) {
			return XR_ERROR_RUNTIME_FAILURE;
		}
		return reinterpret_cast<SecureConnectionCallbacks*>(authenticationTokenValidation->context)
			->ValidateAuthenticationToken(authenticationTokenValidation);
	}

	static XrResult XRAPI_CALL RequestServerCertificateStaticCallback(XrRemotingServerCertificateRequestMSFT* serverCertificateRequest) {
		if (!serverCertificateRequest->context) {
			return XR_ERROR_RUNTIME_FAILURE;
		}
		return reinterpret_cast<SecureConnectionCallbacks*>(serverCertificateRequest->context)
			->RequestServerCertificate(serverCertificateRequest);
	}

private:
	XrResult RequestAuthenticationToken(XrRemotingAuthenticationTokenRequestMSFT* authenticationTokenRequest) {
		const std::string tokenUtf8 = m_authenticationToken;
		const uint32_t tokenSize = static_cast<uint32_t>(tokenUtf8.size() + 1); // for null-termination
		if (authenticationTokenRequest->tokenCapacityIn >= tokenSize) {
			memcpy(authenticationTokenRequest->tokenBuffer, tokenUtf8.c_str(), tokenSize);
			authenticationTokenRequest->tokenSizeOut = tokenSize;
			return XR_SUCCESS;
		}
		else {
			authenticationTokenRequest->tokenSizeOut = tokenSize;
			return XR_ERROR_SIZE_INSUFFICIENT;
		}
	}

	XrResult ValidateServerCertificate(XrRemotingServerCertificateValidationMSFT* serverCertificateValidation) {
		if (!serverCertificateValidation->systemValidationResult) {
			return XR_ERROR_RUNTIME_FAILURE; // We requested system validation to be performed
		}

		serverCertificateValidation->validationResultOut = *serverCertificateValidation->systemValidationResult;
		if (m_allowCertificateNameMismatch && serverCertificateValidation->validationResultOut.nameValidationResult ==
			XR_REMOTING_CERTIFICATE_NAME_VALIDATION_RESULT_MISMATCH_MSFT) {
			serverCertificateValidation->validationResultOut.nameValidationResult =
				XR_REMOTING_CERTIFICATE_NAME_VALIDATION_RESULT_MATCH_MSFT;
		}
		if (m_allowUnverifiedCertificateChain) {
			serverCertificateValidation->validationResultOut.trustedRoot = true;
		}

		return XR_SUCCESS;
	}

	XrResult ValidateAuthenticationToken(XrRemotingAuthenticationTokenValidationMSFT* authenticationTokenValidation) {
		const std::string tokenUtf8 = m_authenticationToken;
		authenticationTokenValidation->tokenValidOut =
			(authenticationTokenValidation->token != nullptr && tokenUtf8 == authenticationTokenValidation->token);
		return XR_SUCCESS;
	}

	XrResult RequestServerCertificate(XrRemotingServerCertificateRequestMSFT* serverCertificateRequest) {
		const std::string subjectNameUtf8 = m_subjectName;
		const std::string passPhraseUtf8 = m_keyPassphrase;

		const uint32_t certStoreSize = static_cast<uint32_t>(m_certificateStore.size());
		const uint32_t subjectNameSize = static_cast<uint32_t>(subjectNameUtf8.size() + 1); // for null-termination
		const uint32_t passPhraseSize = static_cast<uint32_t>(passPhraseUtf8.size() + 1);   // for null-termination

		serverCertificateRequest->certStoreSizeOut = certStoreSize;
		serverCertificateRequest->subjectNameSizeOut = subjectNameSize;
		serverCertificateRequest->keyPassphraseSizeOut = passPhraseSize;
		if (serverCertificateRequest->certStoreCapacityIn < certStoreSize ||
			serverCertificateRequest->subjectNameCapacityIn < subjectNameSize ||
			serverCertificateRequest->keyPassphraseCapacityIn < passPhraseSize) {
			return XR_ERROR_SIZE_INSUFFICIENT;
		}

		// All buffers have sufficient size, so fill in the data
		memcpy(serverCertificateRequest->certStoreBuffer, m_certificateStore.data(), certStoreSize);
		memcpy(serverCertificateRequest->subjectNameBuffer, subjectNameUtf8.c_str(), subjectNameSize);
		memcpy(serverCertificateRequest->keyPassphraseBuffer, passPhraseUtf8.c_str(), passPhraseSize);

		return XR_SUCCESS;
	}

private:
	const std::string m_authenticationToken;
	const bool m_allowCertificateNameMismatch;
	const bool m_allowUnverifiedCertificateChain;
	const std::string m_keyPassphrase;
	const std::string m_subjectName;
	const std::string m_certificateStoreName;
	std::vector<uint8_t> m_certificateStore;
	const bool m_listen;
};

class HololensRenderer : public WmrRenderer
{
public:
	HololensRenderer(const AppOptions& options);
	virtual ~HololensRenderer();

	bool Init(void*, void*) override;
	void Close() override;

	//void Draw(const std::vector<flvr::Framebuffer*> &fbos) override;

protected:
	void SetExtensions() override;
	bool CreateSession(void* hdc, void* hdxrc) override;
	void LoadFunctions() override;

	void PollEvents() override;

private:
	bool EnableRemotingXR();
	void PrepareRemotingEnvironment();

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
	void HandleRecognizedSpeechText(const std::string& text);

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
	PFN_xrRetrieveRemotingSpeechRecognizedTextMSFT xrRetrieveRemotingSpeechRecognizedTextMSFT;

	SecureConnectionCallbacks m_secureConnectionCallbacks;
};

#endif//HololensRenderer_h