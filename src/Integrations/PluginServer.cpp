#include "PluginServer.h"

namespace QuickLoot::Integrations
{
	void PluginServer::Init(const SKSE::MessagingInterface* messagingInterface)
	{
		_server.Init(QuickLootAPI::API_MAJOR_VERSION, QuickLootAPI::API_MINOR_VERSION);

		_server.RegisterHandler<TakeHandler::Request, bool>(QuickLootAPI::RequestType::kRegisterTakeHandler, "kRegisterTakeHandler", [](const char*, const TakeHandler::Request* request, bool* response) {
			return *response = PluginServer::RegisterOnTakeHandler(request->handler);
		});
		_server.RegisterHandler<SelectHandler::Request, bool>(QuickLootAPI::RequestType::kRegisterSelectHandler, "kRegisterSelectHandler", [](const char*, const SelectHandler::Request* request, bool* response) {
			return *response = PluginServer::RegisterOnSelectHandler(request->handler);
		});
		_server.RegisterHandler<InvalidateLootMenuHandler::Request, bool>(QuickLootAPI::RequestType::kRegisterInvalidateLootMenuHandler, "kRegisterInvalidateLootMenuHandler", [](const char*, const InvalidateLootMenuHandler::Request* request, bool* response) {
			return *response = PluginServer::RegisterInvalidateLootMenuHandler(request->handler);
		});
		_server.RegisterHandler<OpenLootMenuHandler::Request, bool>(QuickLootAPI::RequestType::kRegisterOpenLootMenuHandler, "kRegisterOpenLootMenuHandler", [](const char*, const OpenLootMenuHandler::Request* request, bool* response) {
			return *response = PluginServer::RegisterOpenLootMenuHandler(request->handler);
		});
		_server.RegisterHandler<OpeningLootMenuHandler::Request, bool>(QuickLootAPI::RequestType::kRegisterOpeningLootMenuHandler, "kRegisterOpeningLootMenuHandler", [](const char*, const OpeningLootMenuHandler::Request* request, bool* response) {
			return *response = PluginServer::RegisterOpeningLootMenuHandler(request->handler);
		});
		_server.RegisterHandler<CloseLootMenuHandler::Request, bool>(QuickLootAPI::RequestType::kRegisterCloseLootMenuHandler, "kRegisterCloseLootMenuHandler", [](const char*, const CloseLootMenuHandler::Request* request, bool* response) {
			return *response = PluginServer::RegisterCloseLootMenuHandler(request->handler);
		});

		if (!messagingInterface->RegisterListener(nullptr, [](SKSE::MessagingInterface::Message* a_msg) { PluginServer::GetRequestServer().Handle(a_msg); })) {
			logger::error("Error on register QuickLootIE Message Handler"sv);
		}
	}

	inline bool PluginServer::RegisterOnTakeHandler(TakeHandler::OnTakeHandler handler)
	{
		_onTakeHandlers.push_back(handler);
		return true;
	}
	inline bool PluginServer::RegisterOnSelectHandler(SelectHandler::OnSelectHandler handler)
	{
		_onSelectHandlers.push_back(handler);
		return true;
	}
	inline bool PluginServer::RegisterInvalidateLootMenuHandler(InvalidateLootMenuHandler::OnInvalidateLootMenuHandler handler)
	{
		_onInvalidateLootMenuHandlers.push_back(handler);
		return true;
	}
	inline bool PluginServer::RegisterOpenLootMenuHandler(OpenLootMenuHandler::OnOpenLootMenuHandler handler)
	{
		_onOpenLootMenuHandlers.push_back(handler);
		return true;
	}
	inline bool PluginServer::RegisterOpeningLootMenuHandler(OpeningLootMenuHandler::OnOpeningLootMenuHandler handler)
	{
		_onOpeningLootMenuHandlers.push_back(handler);
		return true;
	}
	inline bool PluginServer::RegisterCloseLootMenuHandler(CloseLootMenuHandler::OnCloseLootMenuHandler handler)
	{
		_onCloseLootMenuHandlers.push_back(handler);
		return true;
	}

	inline void PluginServer::HandleOnTake(TakeHandler::TakeEvent* evt)
	{
		for (auto const& handler : _onTakeHandlers) {
			handler(evt);
		}
	}

	void PluginServer::HandleOnTake(RE::Actor* actor, std::vector<Element>* elements, RE::TESObjectREFR* container)
	{
		TakeHandler::TakeEvent evt{};
		evt.actor = actor;
		evt.container = container;
		evt.elements = elements->data();
		evt.elementsCount = elements->size();

		HandleOnTake(&evt);
	}

	void PluginServer::HandleOnTake(RE::Actor* actor, RE::TESForm* object, std::ptrdiff_t count, RE::TESObjectREFR* container)
	{
		Element elements[1] = { Element(object, count, container) };
		TakeHandler::TakeEvent evt{};
		evt.actor = actor;
		evt.container = container;
		evt.elements = elements;
		evt.elementsCount = 1;

		HandleOnTake(&evt);
	}

	inline void PluginServer::HandleOnSelect(SelectHandler::SelectEvent* evt)
	{
		for (auto const& handler : _onSelectHandlers) {
			handler(evt);
		}
	}
	void PluginServer::HandleOnSelect(RE::Actor* actor, std::vector<Element>* elements, RE::TESObjectREFR* container)
	{
		SelectHandler::SelectEvent evt{};
		evt.actor = actor;
		evt.container = container;
		evt.elements = elements->data();
		evt.elementsCount = elements->size();

		HandleOnSelect(&evt);
	}
	void PluginServer::HandleOnSelect(RE::Actor* actor, RE::TESForm* object, std::ptrdiff_t count, RE::ObjectRefHandle container)
	{
		Element elements[1] = { Element(object, count, container) };
		SelectHandler::SelectEvent evt{};
		evt.actor = actor;
		evt.container = container.get() ? container.get().get() : nullptr;
		evt.elements = elements;
		evt.elementsCount = 1;

		HandleOnSelect(&evt);
	}

	inline void PluginServer::HandleOnInvalidateLootMenu(InvalidateLootMenuHandler::InvalidateLootMenuEvent* evt)
	{
		for (auto const& handler : _onInvalidateLootMenuHandlers) {
			handler(evt);
		}
	}
	void PluginServer::HandleOnInvalidateLootMenu(std::vector<Element>* elements, RE::ObjectRefHandle container)
	{
		InvalidateLootMenuHandler::InvalidateLootMenuEvent evt{};
		evt.container = container.get() ? container.get().get() : nullptr;
		evt.elements = elements->data();
		evt.elementsCount = elements->size();

		HandleOnInvalidateLootMenu(&evt);
	}

	inline void PluginServer::HandleOnOpenLootMenu(OpenLootMenuHandler::OpenLootMenuEvent* evt)
	{
		for (auto const& handler : _onOpenLootMenuHandlers) {
			handler(evt);
		}
	}
	void PluginServer::HandleOnOpenLootMenu(RE::ObjectRefHandle containerHandle)
	{
		OpenLootMenuHandler::OpenLootMenuEvent evt{};
		evt.container = containerHandle.get() ? containerHandle.get().get() : nullptr;

		HandleOnOpenLootMenu(&evt);
	}

	inline void PluginServer::HandleOnOpeningLootMenu(OpeningLootMenuHandler::OpeningLootMenuEvent* evt)
	{
		for (auto const& handler : _onOpeningLootMenuHandlers) {
			handler(evt);
			if (evt->result == OpeningLootMenuHandler::kContinue) {
				continue;
			}
			return;
		}
	}
	OpeningLootMenuHandler::HandleResult PluginServer::HandleOnOpeningLootMenu(RE::TESObjectREFRPtr containerHandle)
	{
		OpeningLootMenuHandler::OpeningLootMenuEvent evt{};
		evt.container = containerHandle.get();
		HandleOnOpeningLootMenu(&evt);
		return evt.result;
	}

	inline void PluginServer::HandleOnCloseLootMenu(CloseLootMenuHandler::CloseLootMenuEvent* evt)
	{
		for (auto const& handler : _onCloseLootMenuHandlers) {
			handler(evt);
		}
	}
	void PluginServer::HandleOnCloseLootMenu()
	{
		CloseLootMenuHandler::CloseLootMenuEvent evt{};

		HandleOnCloseLootMenu(&evt);
	}
}
