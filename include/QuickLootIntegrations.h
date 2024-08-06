#pragma once

#include "PluginRequests/RequestClient.h"

/*
	Header File for QuickLoot integration
*/

namespace QuickLoot::Integrations
{

	struct Element
	{
		RE::TESForm* object = nullptr;
		std::int32_t count = 0;
		RE::TESObjectREFR* container = nullptr;

		Element(RE::TESForm* object, std::int32_t count, RE::TESObjectREFR* container) :
			object(object), count(count), container(container){};

		Element(RE::TESForm* object, std::ptrdiff_t count, RE::TESObjectREFR* container) :
			object(object), count(static_cast<std::int32_t>(count)), container(container){};

		Element(RE::TESForm* object, std::ptrdiff_t count, RE::ObjectRefHandle container) :
			object(object), count(static_cast<std::int32_t>(count)), container(container.get() ? container.get().get() : nullptr){};

		Element(RE::TESObjectREFRPtr object, std::ptrdiff_t count, RE::TESObjectREFR* container) :
			object(object.get()), count(static_cast<std::int32_t>(count)), container(container){};

		Element(RE::TESObjectREFRPtr object, std::ptrdiff_t count) :
			object(object.get()), count(static_cast<std::int32_t>(count)){};
	};

	namespace TakeHandler
	{
		struct TakeEvent
		{
			RE::Actor* actor;
			RE::TESObjectREFR* container = nullptr;
			Element* elements;
			std::size_t elementsCount;
		};
		typedef void (*OnTakeHandler)(TakeEvent* evt);

		struct Request
		{
			OnTakeHandler handler;
		};
	};

	namespace SelectHandler
	{
		struct SelectEvent
		{
			RE::Actor* actor;
			RE::TESObjectREFR* container = nullptr;
			Element* elements;
			std::size_t elementsCount;
		};
		typedef void (*OnSelectHandler)(SelectEvent* evt);

		struct Request
		{
			OnSelectHandler handler;
		};
	};

	namespace InvalidateLootMenuHandler
	{
		struct InvalidateLootMenuEvent
		{
			RE::TESObjectREFR* container = nullptr;
			Element* elements;
			std::size_t elementsCount;
		};
		typedef void (*OnInvalidateLootMenuHandler)(InvalidateLootMenuEvent* evt);

		struct Request
		{
			OnInvalidateLootMenuHandler handler;
		};
	};

	namespace OpenLootMenuHandler
	{
		struct OpenLootMenuEvent
		{
			RE::TESObjectREFR* container;
		};
		typedef void (*OnOpenLootMenuHandler)(OpenLootMenuEvent* evt);

		struct Request
		{
			OnOpenLootMenuHandler handler;
		};
	};

	namespace OpeningLootMenuHandler
	{
		enum HandleResult : uint8_t
		{
			kContinue = 0,
			kStop = 1
		};

		struct OpeningLootMenuEvent
		{
			RE::TESObjectREFR* container;
			HandleResult result = HandleResult::kContinue;
		};
		typedef void (*OnOpeningLootMenuHandler)(OpeningLootMenuEvent* evt);

		struct Request
		{
			OnOpeningLootMenuHandler handler;
		};
	};

	namespace CloseLootMenuHandler
	{
		struct CloseLootMenuEvent
		{
		};
		typedef void (*OnCloseLootMenuHandler)(CloseLootMenuEvent* evt);

		struct Request
		{
			OnCloseLootMenuHandler handler;
		};
	};

	class PluginServer;

	class QuickLootAPI
	{
		friend class PluginServer;

	private:
		QuickLootAPI() = default;
		~QuickLootAPI() = default;

		// For a client request to be processed, both of the following must be true:
		//
		// - client major ver == server major ver
		// - client minor var <= server minor ver
		//
		// Minor version changes must be backwards compatible, so any breaking changes
		// to the api must increase the major version. New handlers may be added by
		// new minor versions.
		static constexpr const char* SERVER_PLUGIN_NAME = "QuickLootIE";
		static constexpr uint16_t API_MAJOR_VERSION = 1;
		static constexpr uint16_t API_MINOR_VERSION = 0;

		static inline PluginRequests::RequestClient _client{};

		// This is the list of request types.
		// Each of them is associated with a signature.
		// The client and server must agree on these signatures.
		enum RequestType : uint32_t
		{
			kRegisterTakeHandler = 0x100,
			kRegisterSelectHandler = 0x101,
			kRegisterInvalidateLootMenuHandler = 0x102,
			kRegisterOpenLootMenuHandler = 0x103,
			kRegisterCloseLootMenuHandler = 0x104,
			kRegisterOpeningLootMenuHandler = 0x105,
		};

	public:
		QuickLootAPI(QuickLootAPI const&) = delete;
		QuickLootAPI(QuickLootAPI const&&) = delete;
		QuickLootAPI operator=(QuickLootAPI&) = delete;
		QuickLootAPI operator=(QuickLootAPI&&) = delete;

		// The client initialization must happen at (or after) kPostLoad
		static void Init()
		{
			_client.Init(SERVER_PLUGIN_NAME, API_MAJOR_VERSION, API_MINOR_VERSION);
		}

		static bool IsReady()
		{
			return _client.IsReady();
		}

		static bool RegisterTakeHandler(TakeHandler::OnTakeHandler handler)
		{
			bool response = false;
			const TakeHandler::Request request{ handler };

			if (const auto error = _client.Query(kRegisterTakeHandler, &request, &response)) {
				logger::error("Query failed for {}: {}", __func__, _client.GetErrorString(error));
				return false;
			}

			return true;
		}

		static bool RegisterSelectHandler(SelectHandler::OnSelectHandler handler)
		{
			bool response = false;
			const SelectHandler::Request request{ handler };

			if (const auto error = _client.Query(kRegisterSelectHandler, &request, &response)) {
				logger::error("Query failed for {}: {}", __func__, _client.GetErrorString(error));
				return false;
			}

			return true;
		}

		static bool RegisterInvalidateLootMenuHandler(InvalidateLootMenuHandler::OnInvalidateLootMenuHandler handler)
		{
			bool response = false;
			const InvalidateLootMenuHandler::Request request{ handler };

			if (const auto error = _client.Query(kRegisterInvalidateLootMenuHandler, &request, &response)) {
				logger::error("Query failed for {}: {}", __func__, _client.GetErrorString(error));
				return false;
			}

			return true;
		}

		static bool RegisterOpenLootMenuHandler(OpenLootMenuHandler::OnOpenLootMenuHandler handler)
		{
			bool response = false;
			const OpenLootMenuHandler::Request request{ handler };

			if (const auto error = _client.Query(kRegisterOpenLootMenuHandler, &request, &response)) {
				logger::error("Query failed for {}: {}", __func__, _client.GetErrorString(error));
				return false;
			}

			return true;
		}

		static bool RegisterOpeningLootMenuHandler(OpeningLootMenuHandler::OnOpeningLootMenuHandler handler)
		{
			bool response = false;
			const OpeningLootMenuHandler::Request request{ handler };

			if (const auto error = _client.Query(kRegisterOpeningLootMenuHandler, &request, &response)) {
				logger::error("Query failed for {}: {}", __func__, _client.GetErrorString(error));
				return false;
			}

			return true;
		}

		static bool RegisterCloseLootMenuHandler(CloseLootMenuHandler::OnCloseLootMenuHandler handler)
		{
			bool response = false;
			const CloseLootMenuHandler::Request request{ handler };

			if (const auto error = _client.Query(kRegisterCloseLootMenuHandler, &request, &response)) {
				logger::error("Query failed for {}: {}", __func__, _client.GetErrorString(error));
				return false;
			}

			return true;
		}
	};
}
