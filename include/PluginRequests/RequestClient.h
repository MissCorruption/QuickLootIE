#pragma once

#include "PluginRequests.h"

namespace PluginRequests
{
	class RequestClient
	{
		friend class RequestServer;

	private:
		std::string _serverPluginName;
		uint16_t _apiMajorVersion;
		uint16_t _apiMinorVersion;

		const SKSE::MessagingInterface* _messenger = nullptr;
		bool _isReady = false;

		static constexpr bool debug = false;

		bool TryConnect()
		{
			VersionMessage message{};

			const bool result = _messenger->Dispatch(kVersion, &message, sizeof(message), _serverPluginName.c_str());

			if (!result) {
				SKSE::log::error("Failed to connect to plugin {} (failed to deliver)", _serverPluginName);
				return false;
			}

			if (message.apiMajorVersion == 0 && message.apiMinorVersion == 0) {
				SKSE::log::error("Failed to connect to plugin {} (no response)", _serverPluginName);
				return false;
			}

			if (message.apiMajorVersion != _apiMajorVersion) {
				SKSE::log::error("Failed to connect to plugin {} (got version {}.{}, need major version {})",
					_serverPluginName, message.apiMajorVersion, message.apiMinorVersion, _apiMajorVersion);
				return false;
			}

			if (message.apiMinorVersion < _apiMinorVersion) {
				SKSE::log::error("Failed to connect to plugin {} (got version {}.{}, need at least minor version {})",
					_serverPluginName, message.apiMajorVersion, message.apiMinorVersion, _apiMinorVersion);
				return false;
			}

			SKSE::log::info("Successfully connected to plugin {} (client version {}.{}, server version {}.{})",
				_serverPluginName, _apiMajorVersion, _apiMinorVersion, message.apiMajorVersion, message.apiMinorVersion);

			return true;
		}

		template <typename TRequest, typename TResponse>
		ResponseType QueryInternal(uint32_t requestType, const TRequest* request, TResponse* response)
		{
			return QueryImplementation(requestType, request, response, static_cast<ResponseCallback<TResponse>>(nullptr));
		}

		template <typename TRequest, typename TResponse>
		ResponseType QueryArrayInternal(uint32_t requestType, const TRequest* request, ResponseCallback<TResponse> responseCallback)
		{
			return QueryImplementation(requestType, request, static_cast<TResponse*>(nullptr), responseCallback);
		}

		template <typename TRequest, typename TResponse>
		ResponseType QueryImplementation(uint32_t requestType, const TRequest* request, TResponse* response, ResponseCallback<TResponse> responseCallback)
		{
			static_assert(std::is_trivial_v<TRequest>, "Request data type must be trivial.");
			static_assert(std::is_trivial_v<TResponse>, "Response data type must be trivial.");

			if (!_isReady) {
				SKSE::log::error("Client is not ready");
				return kNotReady;
			}

			if (!response && !responseCallback) {
				SKSE::log::error("Specified no response pointer and no response callback");
				return kInvalidArguments;
			}

			if (response && responseCallback) {
				SKSE::log::error("Specified both a response pointer and a response callback");
				return kInvalidArguments;
			}

			CallbackInfo callbackInfo{
				.callback = [responseCallback, this](size_t count, const void* data) {
					if (debug) {
						SKSE::log::trace("Invoking unwrapped callback");
					}

					responseCallback(count, static_cast<const TResponse*>(data));
				},
				.wasInvoked = false,
			};

			QueryMessage message{
				.apiMajorVersion = _apiMajorVersion,
				.apiMinorVersion = _apiMinorVersion,

				.requestType = requestType,
				.requestDataSize = sizeof(TRequest),
				.requestData = request,

				.responseType = kNoResponse,
				.responseDataSize = sizeof(TResponse),
				.responseData = response,
				.responseCallback = InvokeResponseCallback,
				.responseCallbackUserPtr = &callbackInfo,
			};

			if (debug) {
				SKSE::log::trace("Dispatching request type {}, request data [{}:{}], response data [{}:{}]",
				   requestType,
				   sizeof(TRequest), typeid(TRequest).name(),
				   sizeof(TResponse), typeid(TResponse).name());
			}

			const bool isArrayQuery = responseCallback != nullptr;
			const bool success = _messenger->Dispatch(isArrayQuery ? kQueryArray : kQuery, &message, sizeof(message), _serverPluginName.c_str());

			if (!success) {
				SKSE::log::error("Dispatch failed");
				return kDispatchError;
			}

			if (message.responseType == kSuccess && isArrayQuery && !callbackInfo.wasInvoked) {
				SKSE::log::error("Callback was not invoked");
				return kCallbackNotInvoked;
			}

			return static_cast<ResponseType>(message.responseType);
		}

		static void InvokeResponseCallback(void* userPtr, size_t count, const void* data)
		{
			if (!userPtr) {
				SKSE::log::error("InvokeResponseCallback received no userPtr");
				return;
			}

			if (debug) {
				SKSE::log::trace("Client-side callback translation");
			}

			const auto callbackInfo = static_cast<CallbackInfo*>(userPtr);
			const auto& callback = callbackInfo->callback;

			callback(count, data);
			callbackInfo->wasInvoked = true;
		}

	public:
		RequestClient() = default;
		~RequestClient() = default;

		RequestClient(RequestClient const&) = delete;
		RequestClient(RequestClient const&&) = delete;
		RequestClient operator=(RequestClient&) = delete;
		RequestClient operator=(RequestClient&&) = delete;

		// This must be called when the server plugin is ready (for example on kPostLoad or kDataLoaded)
		void Init(const char* serverPluginName, uint16_t apiMajorVersion, uint16_t apiMinorVersion)
		{
			_serverPluginName = serverPluginName;
			_apiMajorVersion = apiMajorVersion;
			_apiMinorVersion = apiMinorVersion;

			_messenger = SKSE::GetMessagingInterface();
			_isReady = TryConnect();
		}

		bool IsReady() const
		{
			return _isReady;
		}

		// ReSharper disable once CppMemberFunctionMayBeStatic
		const char* GetErrorString(ResponseType response)
		{
			if (response < 0 || response >= ResponseErrorStrings.size()) {
				return "kUnknownError";
			}

			return ResponseErrorStrings[response];
		}

		template <typename TRequest, typename TResponse>
		ResponseType Query(uint32_t requestType, const TRequest* request, TResponse* response)
		{
			return QueryInternal(requestType, request, response);
		}

		template <typename TRequest>
		ResponseType Query(uint32_t requestType, const TRequest* request, nullptr_t)
		{
			std::monostate response{};
			return QueryInternal(requestType, request, &response);
		}

		template <typename TResponse>
		ResponseType Query(uint32_t requestType, nullptr_t, TResponse* response)
		{
			constexpr std::monostate request{};
			return QueryInternal(requestType, &request, response);
		}

		ResponseType Query(uint32_t requestType, nullptr_t, nullptr_t)
		{
			constexpr std::monostate request{};
			std::monostate response{};
			return QueryInternal(requestType, &request, &response);
		}

		template <typename TRequest, typename TResponse>
		ResponseType QueryArray(uint32_t requestType, const TRequest* request, ResponseCallback<TResponse> responseCallback)
		{
			return QueryArrayInternal(requestType, request, responseCallback);
		}

		template <typename TResponse>
		ResponseType QueryArray(uint32_t requestType, nullptr_t, ResponseCallback<TResponse> responseCallback)
		{
			constexpr std::monostate request{};
			return QueryArrayInternal(requestType, &request, responseCallback);
		}

		template <typename TRequest>
		ResponseType QueryString(uint32_t requestType, const TRequest* request, std::string& response)
		{
			const ResponseCallback<char> callback =
				[&response](size_t count, const char* str) {
					response.assign(str, count);
				};

			return QueryArrayInternal(requestType, request, callback);
		}
	};
}
