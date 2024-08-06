#pragma once 

#include <QuickLootIntegrations.h>

#include "PluginRequests/RequestServer.h"

namespace QuickLoot::Integrations
{
	class PluginServer
	{
	public:
		static void Init(const SKSE::MessagingInterface* messagingInterface);

		static void HandleOnTake(RE::Actor* actor, std::vector<Element>* elements, RE::TESObjectREFR* container = nullptr);
		static void HandleOnTake(RE::Actor* actor, RE::TESForm* object, std::ptrdiff_t count, RE::TESObjectREFR* container = nullptr);
		
		static void HandleOnSelect(RE::Actor* actor, std::vector<Element>* elements, RE::TESObjectREFR* container = nullptr);
		static void HandleOnSelect(RE::Actor* actor, RE::TESForm* object, std::ptrdiff_t count, RE::ObjectRefHandle container);

		static void HandleOnInvalidateLootMenu(std::vector<Element>* elements, RE::ObjectRefHandle container);
		static void HandleOnOpenLootMenu(RE::ObjectRefHandle containerHandle);
		static void HandleOnCloseLootMenu();
		static OpeningLootMenuHandler::HandleResult HandleOnOpeningLootMenu(RE::TESObjectREFRPtr containerHandle);

		static inline PluginRequests::RequestServer GetRequestServer() { return _server; };

	protected:
		static inline PluginRequests::RequestServer _server{};
		static inline std::list<TakeHandler::OnTakeHandler> _onTakeHandlers{};
		static inline std::list<SelectHandler::OnSelectHandler> _onSelectHandlers{};
		static inline std::list<InvalidateLootMenuHandler::OnInvalidateLootMenuHandler> _onInvalidateLootMenuHandlers{};
		static inline std::list<OpenLootMenuHandler::OnOpenLootMenuHandler> _onOpenLootMenuHandlers{};
		static inline std::list<OpeningLootMenuHandler::OnOpeningLootMenuHandler> _onOpeningLootMenuHandlers{};
		static inline std::list<CloseLootMenuHandler::OnCloseLootMenuHandler> _onCloseLootMenuHandlers{};

		static bool RegisterOnTakeHandler(TakeHandler::OnTakeHandler handler);
		static bool RegisterOnSelectHandler(SelectHandler::OnSelectHandler handler);
		static bool RegisterInvalidateLootMenuHandler(InvalidateLootMenuHandler::OnInvalidateLootMenuHandler handler);
		static bool RegisterOpenLootMenuHandler(OpenLootMenuHandler::OnOpenLootMenuHandler handler);
		static bool RegisterOpeningLootMenuHandler(OpeningLootMenuHandler::OnOpeningLootMenuHandler handler);
		static bool RegisterCloseLootMenuHandler(CloseLootMenuHandler::OnCloseLootMenuHandler handler);

		static void HandleOnTake(TakeHandler::TakeEvent* evt);
		static void HandleOnSelect(SelectHandler::SelectEvent* evt);
		static void HandleOnInvalidateLootMenu(InvalidateLootMenuHandler::InvalidateLootMenuEvent* evt);
		static void HandleOnOpenLootMenu(OpenLootMenuHandler::OpenLootMenuEvent* evt);
		static void HandleOnOpeningLootMenu(OpeningLootMenuHandler::OpeningLootMenuEvent* evt);
		static void HandleOnCloseLootMenu(CloseLootMenuHandler::CloseLootMenuEvent* evt);
	};
}
