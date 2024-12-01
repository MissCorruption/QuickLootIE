#include "APIServer.h"

#include "MenuVisibilityManager.h"

namespace QuickLoot::API
{
	template <typename TEvent, std::vector<EventHandler<TEvent>>& handlerList>
	bool RegisterHandler(const char*, const HandlerRegistrationRequest<TEvent>* request, bool* response)
	{
		handlerList.push_back(request->handler);
		return *response = true;
	}

	bool DisableLootMenu(const char* sender, const std::monostate*, std::monostate*)
	{
		MenuVisibilityManager::DisableLootMenu(sender);
		return true;
	}

	bool EnableLootMenu(const char* sender, const std::monostate*, std::monostate*)
	{
		MenuVisibilityManager::EnableLootMenu(sender);
		return true;
	}

	void APIServer::Init(const SKSE::MessagingInterface* messenger)
	{
		_server.Init(QuickLootAPI::API_MAJOR_VERSION, QuickLootAPI::API_MINOR_VERSION);

		using enum QuickLootAPI::RequestType;

		_server.RegisterHandler(kDisableLootMenu, "kDisableLootMenu", DisableLootMenu);
		_server.RegisterHandler(kEnableLootMenu, "kEnableLootMenu", EnableLootMenu);

		_server.RegisterHandler(kRegisterTakingItemHandler, "kRegisterTakingItemHandler", RegisterHandler<TakingItemEvent, _takingItemHandlers>);
		_server.RegisterHandler(kRegisterTakeItemHandler, "kRegisterTakeItemHandler", RegisterHandler<TakeItemEvent, _takeItemHandlers>);
		_server.RegisterHandler(kRegisterSelectItemHandler, "kRegisterSelectItemHandler", RegisterHandler<SelectItemEvent, _selectItemHandlers>);
		_server.RegisterHandler(kRegisterOpeningLootMenuHandler, "kRegisterOpeningLootMenuHandler", RegisterHandler<OpeningLootMenuEvent, _openingLootMenuHandlers>);
		_server.RegisterHandler(kRegisterOpenLootMenuHandler, "kRegisterOpenLootMenuHandler", RegisterHandler<OpenLootMenuEvent, _openLootMenuHandlers>);
		_server.RegisterHandler(kRegisterCloseLootMenuHandler, "kRegisterCloseLootMenuHandler", RegisterHandler<CloseLootMenuEvent, _closeLootMenuHandlers>);
		_server.RegisterHandler(kRegisterInvalidateLootMenuHandler, "kRegisterInvalidateLootMenuHandler", RegisterHandler<InvalidateLootMenuEvent, _invalidateLootMenuHandlers>);

		if (!messenger->RegisterListener(nullptr, HandleSKSEMessage)) {
			logger::error("Failed to register SKSE message listener");
		}
	}

	void APIServer::HandleSKSEMessage(SKSE::MessagingInterface::Message* message)
	{
		_server.Handle(message);
	}

	template <typename TEvent>
	void DispatchEvent(const std::vector<EventHandler<TEvent>>& handlers, TEvent& e)
	{
		for (auto const& handler : handlers) {
			handler(&e);
		}
	}

	template <typename TEvent>
	HandleResult DispatchCancelableEvent(const std::vector<EventHandler<TEvent>>& handlers, TEvent& e)
	{
		for (auto const& handler : handlers) {
			handler(&e);

			if (e.result != HandleResult::kContinue) {
				break;
			}
		}

		return e.result;
	}

	HandleResult APIServer::DispatchTakingItemEvent(RE::Actor* actor, const std::vector<Element>& elements, RE::TESObjectREFR* container)
	{
		TakingItemEvent e{
			.actor = actor,
			.container = container,
			.elements = elements.data(),
			.elementsCount = elements.size(),
			.result = HandleResult::kContinue,
		};

		return DispatchCancelableEvent(_takingItemHandlers, e);
	}

	HandleResult APIServer::DispatchTakingItemEvent(RE::Actor* actor, RE::TESForm* object, std::ptrdiff_t count, RE::TESObjectREFR* container)
	{
		Element elements[] = { Element(object, count, container) };
		TakingItemEvent e{
			.actor = actor,
			.container = container,
			.elements = elements,
			.elementsCount = 1,
			.result = HandleResult::kContinue,
		};

		return DispatchCancelableEvent(_takingItemHandlers, e);
	}

	void APIServer::DispatchTakeItemEvent(RE::Actor* actor, const std::vector<Element>& elements, RE::TESObjectREFR* container)
	{
		TakeItemEvent e{
			.actor = actor,
			.container = container,
			.elements = elements.data(),
			.elementsCount = elements.size(),
		};

		DispatchEvent(_takeItemHandlers, e);
	}

	void APIServer::DispatchTakeItemEvent(RE::Actor* actor, RE::TESForm* object, std::ptrdiff_t count, RE::TESObjectREFR* container)
	{
		Element elements[] = { Element(object, count, container) };
		TakeItemEvent e{
			.actor = actor,
			.container = container,
			.elements = elements,
			.elementsCount = 1,
		};

		DispatchEvent(_takeItemHandlers, e);
	}

	void APIServer::DispatchSelectItemEvent(RE::Actor* actor, const std::vector<Element>& elements, RE::TESObjectREFR* container)
	{
		SelectItemEvent e{
			.actor = actor,
			.container = container,
			.elements = elements.data(),
			.elementsCount = elements.size(),
		};

		DispatchEvent(_selectItemHandlers, e);
	}

	void APIServer::DispatchSelectItemEvent(RE::Actor* actor, RE::TESForm* object, std::ptrdiff_t count, const RE::ObjectRefHandle& container)
	{
		Element elements[] = { Element(object, count, container) };
		SelectItemEvent e{
			e.actor = actor,
			e.container = container.get().get(),
			e.elements = elements,
			e.elementsCount = 1,
		};

		DispatchEvent(_selectItemHandlers, e);
	}

	HandleResult APIServer::DispatchOpeningLootMenuEvent(const RE::TESObjectREFRPtr& container)
	{
		OpeningLootMenuEvent e{
			.container = container.get()
		};

		return DispatchCancelableEvent(_openingLootMenuHandlers, e);
	}

	void APIServer::DispatchOpenLootMenuEvent(const RE::ObjectRefHandle& container)
	{
		OpenLootMenuEvent e{
			.container = container.get().get()
		};

		DispatchEvent(_openLootMenuHandlers, e);
	}

	void APIServer::DispatchCloseLootMenuEvent(const RE::ObjectRefHandle& container)
	{
		CloseLootMenuEvent e{
			.container = container.get().get()
		};

		DispatchEvent(_closeLootMenuHandlers, e);
	}

	void APIServer::DispatchInvalidateLootMenuEvent(const std::vector<Element>& elements, const RE::ObjectRefHandle& container)
	{
		InvalidateLootMenuEvent e{
			.container = container.get().get(),
			.elements = elements.data(),
			.elementsCount = elements.size(),
		};

		DispatchEvent(_invalidateLootMenuHandlers, e);
	}
}
