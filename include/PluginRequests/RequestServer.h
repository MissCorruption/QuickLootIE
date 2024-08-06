#pragma once

#include "PluginRequests.h"

namespace PluginRequests
{
	class RequestServer
	{
		bool _isInitialized = false;
		uint16_t _apiMajorVersion = 0;
		uint16_t _apiMinorVersion = 0;

		struct HandlerInfo
		{
			const char* name;
			size_t requestDataSize;
			size_t responseDataSize;
			std::function<bool(const char* sender, QueryMessage* message)> handler;
			std::function<bool(const char* sender, QueryMessage* message)> arrayHandler;
		};

		std::unordered_map<uint32_t, HandlerInfo> _handlers{};

		void HandleVersionMessage([[maybe_unused]] const char* sender, VersionMessage* message) const
		{
			message->apiMajorVersion = _apiMajorVersion;
			message->apiMinorVersion = _apiMinorVersion;
		}

		void HandleQueryMessage(const char* sender, QueryMessage* message, bool isArrayQuery) const
		{
			if (message->apiMajorVersion != _apiMajorVersion ||
				message->apiMinorVersion > _apiMinorVersion) {
				SKSE::log::error("Received request for unsupported api version {}.{} (own version is {}.{})",
					message->apiMajorVersion, message->apiMinorVersion, _apiMajorVersion, _apiMinorVersion);
				message->responseType = kInvalidApiVersion;
				return;
			}

			const auto it = _handlers.find(message->requestType);

			if (it == _handlers.end()) {
				SKSE::log::error("No handler found for request type {}", message->requestType);
				message->responseType = kUnknownRequestType;
				return;
			}

			const auto& info = it->second;

			SKSE::log::trace("Invoking handler for request type {} ({})", message->requestType, info.name);

			if (message->requestDataSize != info.requestDataSize) {
				SKSE::log::error("Request data size {} doesn't match handler expectation {}",
					message->requestDataSize, info.requestDataSize);
				message->responseType = kInvalidArguments;
				return;
			}

			if (message->responseDataSize != info.responseDataSize) {
				SKSE::log::error("Response data size {} doesn't match handler expectation {}",
					message->responseDataSize, info.responseDataSize);
				message->responseType = kInvalidArguments;
				return;
			}

			bool success;
			if (isArrayQuery) {
				if (!info.arrayHandler) {
					SKSE::log::error("Invoked array query handler as if it were a regular handler");
					message->responseType = kInvalidArguments;
					return;
				}

				if (!message->responseCallback) {
					SKSE::log::error("Invoked array query handler without response callback");
					message->responseType = kInvalidArguments;
					return;
				}

				success = info.arrayHandler(sender, message);

			} else {
				if (!info.handler) {
					SKSE::log::error("Invoked regular query handler as if it were an array handler");
					message->responseType = kInvalidArguments;
					return;
				}

				if (!message->responseData) {
					SKSE::log::error("Invoked regular query handler without response data pointer");
					message->responseType = kInvalidArguments;
					return;
				}

				success = info.handler(sender, message);
			}

			if (!success) {
				SKSE::log::error("Processing error");
				message->responseType = kProcessingError;
				return;
			}

			message->responseType = kSuccess;
		}

	public:
		void Init(uint16_t apiMajorVersion, uint16_t apiMinorVersion)
		{
			if (_isInitialized) {
				return;
			}

			_isInitialized = true;
			_apiMajorVersion = apiMajorVersion;
			_apiMinorVersion = apiMinorVersion;

			SKSE::log::info("Initializing request server with api version {}.{}", apiMajorVersion, apiMinorVersion);
		}

		template <typename TRequest, typename TResponse>
		void RegisterHandler(uint32_t requestType, const char* name, QueryHandler<TRequest, TResponse> handler)
		{
			static_assert(std::is_trivial_v<TRequest>, "Request data must be trivial.");
			static_assert(std::is_trivial_v<TResponse>, "Regular handlers can only handle trivial response data. Use string handlers to return strings.");

			if (!_isInitialized) {
				SKSE::log::error("Tried to register handler before initializing server");
				return;
			}

			if (_handlers.contains(requestType)) {
				SKSE::log::error("A handler for request type {} is already registered", requestType);
				return;
			}

			const auto handlerWrapper = [handler](const char* sender, QueryMessage* message) {
				SKSE::log::trace("Invoking wrapped handler");
				return handler(sender, static_cast<const TRequest*>(message->requestData), static_cast<TResponse*>(message->responseData));
			};

			HandlerInfo info{
				.name = name,
				.requestDataSize = sizeof(TRequest),
				.responseDataSize = sizeof(TResponse),
				.handler = handlerWrapper,
				.arrayHandler = nullptr,
			};

			_handlers.insert_or_assign(requestType, info);

			SKSE::log::info("Registered query handler for request type {} ({}), request data [{}:{}], response data [{}:{}]",
				requestType, name,
				sizeof(TRequest), typeid(TRequest).name(),
				sizeof(TResponse), typeid(TResponse).name());
		}

		template <typename TRequest, typename TResponse>
		void RegisterArrayHandler(uint32_t requestType, const char* name, ArrayQueryHandler<TRequest, TResponse> handler)
		{
			static_assert(std::is_trivial_v<TRequest>, "Request data must be trivial.");

			if (!_isInitialized) {
				SKSE::log::error("Tried to register handler before initializing server");
				return;
			}

			if (_handlers.contains(requestType)) {
				SKSE::log::error("A handler for request type {} is already registered", requestType);
				return;
			}

			const auto handlerWrapper = [handler](const char* sender, QueryMessage* message) {
				const auto callbackWrapper = [message](size_t count, const TResponse* data) {
					SKSE::log::trace("Server-side callback translation");
					message->responseCallback(message->responseCallbackUserPtr, count, data);
				};

				SKSE::log::trace("Invoking wrapped handler");
				return handler(sender, static_cast<const TRequest*>(message->requestData), callbackWrapper);
			};

			HandlerInfo info{
				.name = name,
				.requestDataSize = sizeof(TRequest),
				.responseDataSize = sizeof(TResponse),
				.handler = nullptr,
				.arrayHandler = handlerWrapper,
			};

			_handlers.insert_or_assign(requestType, info);

			SKSE::log::info("Registered array query handler for request type {} ({}), request data [{}:{}], response data [{}:{}]",
				requestType, name,
				sizeof(TRequest), typeid(TRequest).name(),
				sizeof(TResponse), typeid(TResponse).name());
		}

		void Handle(const SKSE::MessagingInterface::Message* message) const
		{
			if (!_isInitialized) {
				SKSE::log::error("Tried to handle message before initializing server");
				return;
			}

			if (message->type == kVersion && message->dataLen == sizeof(VersionMessage)) {
				SKSE::log::trace("Server handling XVER message from {}", message->sender);
				HandleVersionMessage(message->sender, static_cast<VersionMessage*>(message->data));
				return;
			}

			if (message->type == kQuery && message->dataLen == sizeof(QueryMessage)) {
				SKSE::log::trace("Server handling XQRY message from {}", message->sender);
				HandleQueryMessage(message->sender, static_cast<QueryMessage*>(message->data), false);
				return;
			}

			if (message->type == kQueryArray && message->dataLen == sizeof(QueryMessage)) {
				SKSE::log::trace("Server handling XARR message from {}", message->sender);
				HandleQueryMessage(message->sender, static_cast<QueryMessage*>(message->data), true);
				return;
			}

			SKSE::log::trace("Server ignored message {} from {}", message->type, message->sender);
		}
	};
}
