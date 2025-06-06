#include "APIServer.h"

#include "MenuVisibilityManager.h"

namespace QuickLoot::API
{
	extern "C" __declspec(dllexport) void* GetQuickLootInterfaceV20()
	{
		return APIServer::GetInterfaceV20();
	}

#pragma region Interface

	void APIServer::InterfaceV20::DisableLootMenu(const char* plugin)
	{
		MenuVisibilityManager::DisableLootMenu(plugin);
	}

	void APIServer::InterfaceV20::EnableLootMenu(const char* plugin)
	{
		MenuVisibilityManager::EnableLootMenu(plugin);
	}

	void APIServer::InterfaceV20::RegisterTakingItemHandler(const char* plugin, TakingItemHandler handler)
	{
		RegisterHandler(plugin, handler, _takingItemHandlers);
	}

	void APIServer::InterfaceV20::RegisterTakeItemHandler(const char* plugin, TakeItemHandler handler)
	{
		RegisterHandler(plugin, handler, _takeItemHandlers);
	}

	void APIServer::InterfaceV20::RegisterSelectItemHandler(const char* plugin, SelectItemHandler handler)
	{
		RegisterHandler(plugin, handler, _selectItemHandlers);
	}

	void APIServer::InterfaceV20::RegisterOpeningLootMenuHandler(const char* plugin, OpeningLootMenuHandler handler)
	{
		RegisterHandler(plugin, handler, _openingLootMenuHandlers);
	}

	void APIServer::InterfaceV20::RegisterOpenLootMenuHandler(const char* plugin, OpenLootMenuHandler handler)
	{
		RegisterHandler(plugin, handler, _openLootMenuHandlers);
	}

	void APIServer::InterfaceV20::RegisterCloseLootMenuHandler(const char* plugin, CloseLootMenuHandler handler)
	{
		RegisterHandler(plugin, handler, _closeLootMenuHandlers);
	}

	void APIServer::InterfaceV20::RegisterInvalidateLootMenuHandler(const char* plugin, InvalidateLootMenuHandler handler)
	{
		RegisterHandler(plugin, handler, _invalidateLootMenuHandlers);
	}

#pragma endregion

#pragma region Dispatch

	HandleResult APIServer::DispatchTakingItemEvent(RE::Actor* actor, RE::TESObjectREFR* container, RE::InventoryEntryData* entry, RE::TESObjectREFR* dropRef)
	{
		ItemStack stack{ entry, dropRef };

		TakingItemEvent e{
			.actor = actor,
			.container = container,
			.stack = &stack,
			.result = HandleResult::kContinue,
		};

		return DispatchCancelableEvent(_takingItemHandlers, e);
	}

	void APIServer::DispatchTakeItemEvent(RE::Actor* actor, RE::TESObjectREFR* container, RE::InventoryEntryData* entry, RE::TESObjectREFR* dropRef)
	{
		ItemStack stack{ entry, dropRef };

		TakeItemEvent e{
			.actor = actor,
			.container = container,
			.stack = &stack,
		};

		DispatchEvent(_takeItemHandlers, e);
	}

	void APIServer::DispatchSelectItemEvent(RE::Actor* actor, RE::TESObjectREFR* container, RE::InventoryEntryData* entry, RE::TESObjectREFR* dropRef)
	{
		ItemStack stack{ entry, dropRef };

		SelectItemEvent e{
			.actor = actor,
			.container = container,
			.stack = &stack,
		};

		DispatchEvent(_selectItemHandlers, e);
	}

	HandleResult APIServer::DispatchOpeningLootMenuEvent(RE::TESObjectREFR* container)
	{
		OpeningLootMenuEvent e{
			.container = container
		};

		return DispatchCancelableEvent(_openingLootMenuHandlers, e);
	}

	void APIServer::DispatchOpenLootMenuEvent(RE::TESObjectREFR* container)
	{
		OpenLootMenuEvent e{
			.container = container
		};

		DispatchEvent(_openLootMenuHandlers, e);
	}

	void APIServer::DispatchCloseLootMenuEvent(RE::TESObjectREFR* container)
	{
		CloseLootMenuEvent e{
			.container = container
		};

		DispatchEvent(_closeLootMenuHandlers, e);
	}

	void APIServer::DispatchInvalidateLootMenuEvent(RE::TESObjectREFR* container, const std::vector<ItemStack>& inventory)
	{
		InvalidateLootMenuEvent e{
			.container = container,
			.stacks = inventory.data(),
			.stackCount = inventory.size(),
		};

		DispatchEvent(_invalidateLootMenuHandlers, e);
	}

#pragma endregion
}
