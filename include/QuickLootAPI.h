#pragma once

#include "PluginRequests/RequestClient.h"

/*
	Header File for QuickLoot integration
*/

namespace QuickLoot
{
	struct Element
	{
		RE::TESForm* object = nullptr;
		std::int32_t count = 0;
		RE::TESObjectREFR* container = nullptr;

		Element(RE::TESForm* object, std::int32_t count, RE::TESObjectREFR* container) :
			object(object), count(count), container(container) {}

		Element(RE::TESForm* object, std::ptrdiff_t count, RE::TESObjectREFR* container) :
			object(object), count(static_cast<std::int32_t>(count)), container(container) {}

		Element(RE::TESForm* object, std::ptrdiff_t count, RE::ObjectRefHandle container) :
			object(object), count(static_cast<std::int32_t>(count)), container(container.get().get()) {}

		Element(RE::TESObjectREFRPtr object, std::ptrdiff_t count, RE::TESObjectREFR* container) :
			object(object.get()), count(static_cast<std::int32_t>(count)), container(container) {}

		Element(RE::TESObjectREFRPtr object, std::ptrdiff_t count) :
			object(object.get()), count(static_cast<std::int32_t>(count)) {}
	};

	namespace Events
	{
		enum class HandleResult : uint8_t
		{
			kContinue = 0,
			kStop = 1
		};

		struct TakingItemEvent
		{
			RE::Actor* actor;
			RE::TESObjectREFR* container;
			const Element* elements;
			std::size_t elementsCount;
			HandleResult result = HandleResult::kContinue;
		};

		struct TakeItemEvent
		{
			RE::Actor* actor;
			RE::TESObjectREFR* container;
			const Element* elements;
			std::size_t elementsCount;
		};

		struct SelectItemEvent
		{
			RE::Actor* actor;
			RE::TESObjectREFR* container;
			const Element* elements;
			std::size_t elementsCount;
		};

		struct OpeningLootMenuEvent
		{
			RE::TESObjectREFR* container;
			HandleResult result = HandleResult::kContinue;
		};

		struct OpenLootMenuEvent
		{
			RE::TESObjectREFR* container;
		};

		struct CloseLootMenuEvent
		{
			RE::TESObjectREFR* container;
		};

		struct InvalidateLootMenuEvent
		{
			RE::TESObjectREFR* container;
			const Element* elements;
			std::size_t elementsCount;
		};

		template <typename TEvent>
		using EventHandler = void (*)(TEvent* e);

		template <typename TEvent>
		struct HandlerRegistrationRequest
		{
			EventHandler<TEvent> handler;
		};

		using TakingItemHandler = EventHandler<TakingItemEvent>;
		using TakeItemHandler = EventHandler<TakeItemEvent>;
		using SelectItemHandler = EventHandler<SelectItemEvent>;
		using OpeningLootMenuHandler = EventHandler<OpeningLootMenuEvent>;
		using OpenLootMenuHandler = EventHandler<OpenLootMenuEvent>;
		using CloseLootMenuHandler = EventHandler<CloseLootMenuEvent>;
		using InvalidateLootMenuHandler = EventHandler<InvalidateLootMenuEvent>;
	}

	using namespace Events;

	class QuickLootAPI
	{
		static inline PluginRequests::RequestClient _client{};

		template <typename THandler>
		static bool RegisterInternal(const char* func, uint32_t requestType, THandler handler)
		{
			bool response = false;
			const HandlerRegistrationRequest request{ handler };

			if (const auto error = _client.Query(requestType, &request, &response)) {
				logger::error("Query failed for {}: {}", func, _client.GetErrorString(error));
				return false;
			}

			return response;
		}

	public:
		QuickLootAPI() = delete;
		~QuickLootAPI() = delete;
		QuickLootAPI(QuickLootAPI const&) = delete;
		QuickLootAPI(QuickLootAPI const&&) = delete;
		QuickLootAPI operator=(QuickLootAPI&) = delete;
		QuickLootAPI operator=(QuickLootAPI&&) = delete;

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

		// This is the list of request types.
		// Each of them is associated with a signature.
		// The client and server must agree on these signatures.
		enum RequestType : uint32_t
		{
			kDisableLootMenu = 0x000,
			kEnableLootMenu = 0x001,

			kRegisterTakingItemHandler = 0x100,
			kRegisterTakeItemHandler = 0x101,
			kRegisterSelectItemHandler = 0x102,
			kRegisterOpeningLootMenuHandler = 0x103,
			kRegisterOpenLootMenuHandler = 0x104,
			kRegisterCloseLootMenuHandler = 0x105,
			kRegisterInvalidateLootMenuHandler = 0x106,
		};

		// The client initialization must happen at (or after) kPostLoad
		static void Init()
		{
			_client.Init(SERVER_PLUGIN_NAME, API_MAJOR_VERSION, API_MINOR_VERSION);
		}

		static bool IsReady()
		{
			return _client.IsReady();
		}

		static void DisableLootMenu()
		{
			if (const auto error = _client.Query(kDisableLootMenu, nullptr, nullptr)) {
				logger::error("Query failed for DisableLootMenu: {}", _client.GetErrorString(error));
			}
		}

		static void EnableLootMenu()
		{
			if (const auto error = _client.Query(kEnableLootMenu, nullptr, nullptr)) {
				logger::error("Query failed for EnableLootMenu: {}", _client.GetErrorString(error));
			}
		}

		static bool RegisterTakingItemHandler(TakingItemHandler handler)
		{
			return RegisterInternal(__func__, kRegisterTakingItemHandler, handler);
		}

		static bool RegisterTakeItemHandler(TakeItemHandler handler)
		{
			return RegisterInternal(__func__, kRegisterTakeItemHandler, handler);
		}

		static bool RegisterSelectItemHandler(SelectItemHandler handler)
		{
			return RegisterInternal(__func__, kRegisterSelectItemHandler, handler);
		}

		static bool RegisterOpeningLootMenuHandler(OpeningLootMenuHandler handler)
		{
			return RegisterInternal(__func__, kRegisterOpeningLootMenuHandler, handler);
		}

		static bool RegisterOpenLootMenuHandler(OpenLootMenuHandler handler)
		{
			return RegisterInternal(__func__, kRegisterOpenLootMenuHandler, handler);
		}

		static bool RegisterCloseLootMenuHandler(CloseLootMenuHandler handler)
		{
			return RegisterInternal(__func__, kRegisterCloseLootMenuHandler, handler);
		}

		static bool RegisterInvalidateLootMenuHandler(InvalidateLootMenuHandler handler)
		{
			return RegisterInternal(__func__, kRegisterInvalidateLootMenuHandler, handler);
		}
	};
}
