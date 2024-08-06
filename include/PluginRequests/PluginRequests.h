#pragma once

namespace PluginRequests
{
	class RequestClient;
	class RequestServer;

	template <typename TResponse>
	using ResponseCallback = std::function<void(size_t count, const TResponse* data)>;
	using WrappedResponseCallback = void (*)(void* userPtr, size_t count, const void* data);

	template <typename TRequest, typename TResponse>
	using QueryHandler = bool (*)(const char* sender, const TRequest* request, TResponse* response);

	template <typename TRequest, typename TResponse>
	using ArrayQueryHandler = bool (*)(const char* sender, const TRequest* request, const ResponseCallback<TResponse>& responseCallback);

	namespace detail
	{
		enum MessageType : uint32_t
		{
			kVersion = 'XVER',
			kQuery = 'XQRY',
			kQueryArray = 'XARR',
		};

		struct VersionMessage
		{
			uint16_t apiMajorVersion;
			uint16_t apiMinorVersion;
		};

		struct QueryMessage
		{
			RequestClient* client;
			uint16_t apiMajorVersion;
			uint16_t apiMinorVersion;

			uint32_t requestType;
			uint32_t requestDataSize;
			const void* requestData;

			uint32_t responseType;
			uint32_t responseDataSize;
			void* responseData;
			WrappedResponseCallback responseCallback;
			void* responseCallbackUserPtr;
		};

		struct CallbackInfo
		{
			std::function<void(size_t count, const void* data)> callback;
			bool wasInvoked;
		};
	}

	using namespace detail;

	enum ResponseType : uint32_t
	{
		kSuccess = 0,
		kNotReady = 1,
		kDispatchError = 2,
		kNoResponse = 3,
		kInvalidApiVersion = 4,
		kUnknownRequestType = 5,
		kInvalidArguments = 6,
		kCallbackNotInvoked = 7,
		kProcessingError = 8,
	};

	static inline std::array ResponseErrorStrings{
		"kSuccess",
		"kNotReady",
		"kDispatchError",
		"kNoResponse",
		"kInvalidApiVersion",
		"kUnknownRequestType",
		"kInvalidArguments",
		"kCallbackNotInvoked",
		"kProcessingError",
	};
}
