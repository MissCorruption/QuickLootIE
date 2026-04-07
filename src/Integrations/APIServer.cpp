#include "APIServer.h"

#include "LootMenuManager.h"
#include "MenuVisibilityManager.h"
#include "Items/Inventory.h"

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

	void APIServer::InterfaceV20::RegisterModifyInventoryHandler(const char* plugin, ModifyInventoryHandler handler)
	{
		RegisterHandler(plugin, handler, _modifyInventoryHandlers);
	}

	void APIServer::InterfaceV20::RegisterPopulateInfoBarHandler(const char* plugin, PopulateInfoBarHandler handler)
	{
		RegisterHandler(plugin, handler, _populateInfoBarHandlers);
	}

	void APIServer::InterfaceV20::RegisterPopulateButtonBarHandler(const char* plugin, PopulateButtonBarHandler handler)
	{
		RegisterHandler(plugin, handler, _populateButtonBarHandlers);
	}

	void APIServer::InterfaceV20::ForceCurrentContainer(const char* plugin, RE::ObjectRefHandle container)
	{
		MenuVisibilityManager::SetForcedContainer(std::move(container));
		logger::trace("Plugin {} forced container {:08X}", plugin, container.get() ? container.get()->GetFormID() : 0);
	}

	void APIServer::InterfaceV20::ClearForcedContainer(const char* plugin)
	{
		MenuVisibilityManager::SetForcedContainer({});
		logger::trace("Plugin {} cleared forced container", plugin);
	}

	void APIServer::InterfaceV20::CloseLootMenu(const char* plugin)
	{
		LootMenuManager::RequestHide();
		logger::trace("Plugin {} requested hiding the loot menu", plugin);
	}

	void APIServer::InterfaceV20::RefreshLootMenu(const char* plugin)
	{
		logger::trace("Plugin {} requested a loot menu refresh", plugin);
		LootMenuManager::RequestRefresh(RefreshFlags::kAll);
	}

#pragma endregion

#pragma region Dispatch

	HandleResult APIServer::DispatchTakingItemEvent(RE::Actor* actor, RE::ObjectRefHandle container, RE::InventoryEntryData* entry, RE::ObjectRefHandle dropRef)
	{
		ItemStack stack{ entry, std::move(dropRef) };

		TakingItemEvent e{
			.actor = actor,
			.container = container,
			.stack = &stack,
			.result = HandleResult::kContinue,
		};

		return DispatchCancelableEvent(_takingItemHandlers, e);
	}

	void APIServer::DispatchTakeItemEvent(RE::Actor* actor, RE::ObjectRefHandle container, RE::InventoryEntryData* entry, RE::ObjectRefHandle dropRef)
	{
		ItemStack stack{ entry, std::move(dropRef) };

		TakeItemEvent e{
			.actor = actor,
			.container = container,
			.stack = &stack,
		};

		DispatchEvent(_takeItemHandlers, e);
	}

	void APIServer::DispatchSelectItemEvent(RE::Actor* actor, RE::ObjectRefHandle container, RE::InventoryEntryData* entry, RE::ObjectRefHandle dropRef)
	{
		ItemStack stack{ entry, std::move(dropRef) };

		SelectItemEvent e{
			.actor = actor,
			.container = container,
			.stack = &stack,
		};

		DispatchEvent(_selectItemHandlers, e);
	}

	HandleResult APIServer::DispatchOpeningLootMenuEvent(RE::ObjectRefHandle container)
	{
		OpeningLootMenuEvent e{
			.container = container
		};

		return DispatchCancelableEvent(_openingLootMenuHandlers, e);
	}

	void APIServer::DispatchOpenLootMenuEvent(RE::ObjectRefHandle container)
	{
		OpenLootMenuEvent e{
			.container = container
		};

		DispatchEvent(_openLootMenuHandlers, e);
	}

	void APIServer::DispatchCloseLootMenuEvent(RE::ObjectRefHandle container)
	{
		CloseLootMenuEvent e{
			.container = container
		};

		DispatchEvent(_closeLootMenuHandlers, e);
	}

	void APIServer::DispatchInvalidateLootMenuEvent(RE::ObjectRefHandle container, const RE::BSTArray<ItemStack>& inventory)
	{
		InvalidateLootMenuEvent e{
			.container = container,
			.inventory = inventory,
		};

		DispatchEvent(_invalidateLootMenuHandlers, e);
	}

	void APIServer::DispatchModifyInventoryEvent(RE::ObjectRefHandle container, RE::BSTArray<Items::InventoryEntry>& inventory)
	{
		std::shared_lock guard(_lock);

		ModifyInventoryEvent e{
			.container = container,
			.inventory = inventory,
		};

		DispatchEvent(_modifyInventoryHandlers, e);
	}

	std::vector<RE::BSString> APIServer::DispatchPopulateInfoBarEvent(RE::ObjectRefHandle container, RE::InventoryEntryData* entry, RE::ObjectRefHandle dropRef)
	{
		ItemStack stack{ entry, std::move(dropRef) };

		PopulateInfoBarEvent e{
			.container = container,
			.stack = entry ? &stack : nullptr,
			.result = {},
		};

		return DispatchResultEvent<RE::BSString>(_populateInfoBarHandlers, e);
	}

	std::vector<ButtonDefinition> APIServer::DispatchPopulateButtonBarEvent(RE::ObjectRefHandle container, RE::InventoryEntryData* entry, RE::ObjectRefHandle dropRef)
	{
		ItemStack stack{ entry, std::move(dropRef) };

		PopulateButtonBarEvent e{
			.container = container,
			.stack = entry ? &stack : nullptr,
			.result = {},
		};

		return DispatchResultEvent<ButtonDefinition>(_populateButtonBarHandlers, e);
	}

#pragma endregion
}
