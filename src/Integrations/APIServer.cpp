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
		APIServer::RegisterHandler(plugin, handler, APIServer::_takingItemHandlers);
	}

	void APIServer::InterfaceV20::RegisterTakeItemHandler(const char* plugin, TakeItemHandler handler)
	{
		APIServer::RegisterHandler(plugin, handler, APIServer::_takeItemHandlers);
	}

	void APIServer::InterfaceV20::RegisterSelectItemHandler(const char* plugin, SelectItemHandler handler)
	{
		APIServer::RegisterHandler(plugin, handler, APIServer::_selectItemHandlers);
	}

	void APIServer::InterfaceV20::RegisterOpeningLootMenuHandler(const char* plugin, OpeningLootMenuHandler handler)
	{
		APIServer::RegisterHandler(plugin, handler, APIServer::_openingLootMenuHandlers);
	}

	void APIServer::InterfaceV20::RegisterOpenLootMenuHandler(const char* plugin, OpenLootMenuHandler handler)
	{
		APIServer::RegisterHandler(plugin, handler, APIServer::_openLootMenuHandlers);
	}

	void APIServer::InterfaceV20::RegisterCloseLootMenuHandler(const char* plugin, CloseLootMenuHandler handler)
	{
		APIServer::RegisterHandler(plugin, handler, APIServer::_closeLootMenuHandlers);
	}

	void APIServer::InterfaceV20::RegisterInvalidateLootMenuHandler(const char* plugin, InvalidateLootMenuHandler handler)
	{
		APIServer::RegisterHandler(plugin, handler, APIServer::_invalidateLootMenuHandlers);
	}

	void APIServer::InterfaceV20::RegisterModifyInventoryHandler(const char* plugin, ModifyInventoryHandler handler)
	{
		APIServer::RegisterHandler(plugin, handler, APIServer::_modifyInventoryHandlers);
	}

	void APIServer::InterfaceV20::RegisterPopulateInfoBarHandler(const char* plugin, PopulateInfoBarHandler handler)
	{
		APIServer::RegisterHandler(plugin, handler, APIServer::_populateInfoBarHandlers);
	}

	void APIServer::InterfaceV20::RegisterPopulateButtonBarHandler(const char* plugin, PopulateButtonBarHandler handler)
	{
		APIServer::RegisterHandler(plugin, handler, APIServer::_populateButtonBarHandlers);
	}

	void APIServer::InterfaceV20::ForceCurrentContainer(const char* plugin, RE::ObjectRefHandle container)
	{
		logger::trace("Plugin {} forced container {:08X}", plugin, container.get() ? container.get()->GetFormID() : 0);
		MenuVisibilityManager::SetForcedContainer(std::move(container));
	}

	void APIServer::InterfaceV20::ClearForcedContainer(const char* plugin)
	{
		logger::trace("Plugin {} cleared forced container", plugin);
		MenuVisibilityManager::SetForcedContainer({});
	}

	void APIServer::InterfaceV20::CloseLootMenu(const char* plugin)
	{
		logger::trace("Plugin {} requested hiding the loot menu", plugin);
		LootMenuManager::RequestHide();
	}

	void APIServer::InterfaceV20::RefreshLootMenu(const char* plugin)
	{
		logger::trace("Plugin {} requested a loot menu refresh", plugin);
		LootMenuManager::RequestRefresh(RefreshFlags::kAll);
	}

	void APIServer::InterfaceV21::RegisterModifyButtonBarHandler(const char* plugin, ModifyButtonBarHandler handler)
	{
		APIServer::RegisterHandler(plugin, handler, APIServer::_modifyButtonBarHandlers);
	}

	void APIServer::InterfaceV21::RegisterModifyItemDataHandler(const char* plugin, ModifyItemDataHandler handler)
	{
		APIServer::RegisterHandler(plugin, handler, APIServer::_modifyItemDataHandlers);
	}

	void APIServer::InterfaceV21::RegisterInputActionHandler(const char* plugin, InputActionHandler handler)
	{
		APIServer::RegisterHandler(plugin, handler, APIServer::_inputActionHandlers);
	}

	void APIServer::InterfaceV21::PerformInputAction(const char* plugin, QuickLootAction action)
	{
		logger::trace("Plugin {} requested input action {}", plugin, static_cast<int>(action));
		LootMenuManager::OnInputAction(action);
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

	void APIServer::DispatchModifyButtonBarEvent(RE::ObjectRefHandle container, RE::InventoryEntryData* entry, RE::ObjectRefHandle dropRef, RE::BSTArray<ButtonDefinition2>& buttons)
	{
		ItemStack stack{ entry, std::move(dropRef) };

		ModifyButtonBarEvent e{
			.container = container,
			.stack = entry ? &stack : nullptr,
			.buttons = buttons,
		};

		DispatchEvent(_modifyButtonBarHandlers, e);
	}

	void APIServer::DispatchModifyItemDataEvent(RE::ObjectRefHandle container, RE::InventoryEntryData* entry, RE::ObjectRefHandle dropRef, RE::GFxValue& data)
	{
		ItemStack stack{ entry, std::move(dropRef) };

		ModifyItemDataEvent e{
			.container = container,
			.stack = entry ? &stack : nullptr,
			.data = data,
		};

		DispatchEvent(_modifyItemDataHandlers, e);
	}

	HandleResult APIServer::DispatchInputActionEvent(RE::ObjectRefHandle container, QuickLootAction action)
	{
		InputActionEvent e{
			.container = container,
			.action = action
		};

		return DispatchCancelableEvent(_inputActionHandlers, e);
	}

#pragma endregion
}
