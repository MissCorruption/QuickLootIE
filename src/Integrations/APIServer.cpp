#include "APIServer.h"

#include "LootMenuManager.h"
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

	void APIServer::InterfaceV20::RegisterModifyInventoryHandler(const char* plugin, ModifyInventoryHandler handler)
	{
		RegisterHandler(plugin, handler, _modifyInventoryHandlers);
	}

	void APIServer::InterfaceV20::RegisterPopulateInfoBarHandler(const char* plugin, PopulateInfoBarHandler handler)
	{
		RegisterHandler(plugin, handler, _populateInfoBarHandlers);
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

	void APIServer::DispatchInvalidateLootMenuEvent(RE::TESObjectREFR* container, const std::vector<std::unique_ptr<Items::QuickLootItemStack>>& inventory)
	{
		std::vector<ItemStack> apiInventory;
		apiInventory.reserve(inventory.size());
		for (auto& item : inventory) {
			apiInventory.emplace_back(item->GetEntry(), item->GetDropRef().get().get());
		}

		InvalidateLootMenuEvent e{
			.container = container,
			.stacks = apiInventory.data(),
			.stackCount = apiInventory.size(),
		};

		DispatchEvent(_invalidateLootMenuHandlers, e);
	}

	void APIServer::DispatchModifyInventoryEvent(RE::TESObjectREFR* container, std::vector<std::unique_ptr<Items::QuickLootItemStack>>& inventory)
	{
		std::shared_lock guard(_lock);

		ModifyInventoryEvent e{
			.container = container,
			.stacks = nullptr,
			.stackCount = 0,
			.result = {},
		};

		std::vector<ItemStack> apiInventory;

		bool rebuildApiInventory = true;

		for (auto const& handler : _modifyInventoryHandlers) {
			if (rebuildApiInventory) {
				apiInventory.clear();
				apiInventory.reserve(inventory.size());

				for (auto& item : inventory) {
					apiInventory.emplace_back(item->GetEntry(), item->GetDropRef().get().get());
				}

				e.stacks = apiInventory.data();
				e.stackCount = apiInventory.size();
				rebuildApiInventory = false;
			}

			handler(&e);

			for (const auto& modification : e.result) {
				const auto entry = modification.stack.entry;
				const auto dropRef = modification.stack.dropRef ? modification.stack.dropRef->GetHandle() : RE::ObjectRefHandle{};

				rebuildApiInventory = true;

				switch (modification.type) {
				case InventoryModificationType::kAddStack:
					inventory.push_back(std::make_unique<Items::QuickLootItemStack>(entry, container->GetHandle(), dropRef));
					break;

				case InventoryModificationType::kRemoveStack:
					const auto it = std::ranges::find_if(inventory, [&](const auto& stack) { return stack->GetEntry() == modification.stack.entry; });
					if (it != inventory.end()) {
						inventory.erase(it);
					}
					break;
				}
			}
		}
	}

	std::vector<RE::BSString> APIServer::DispatchPopulateInfoBarEvent(RE::TESObjectREFR* container, RE::InventoryEntryData* entry, RE::TESObjectREFR* dropRef)
	{
		ItemStack stack{ entry, dropRef };

		PopulateInfoBarEvent e{
			.container = container,
			.stack = &stack,
			.result = {},
		};

		return DispatchResultEvent<RE::BSString>(_populateInfoBarHandlers, e);
	}

#pragma endregion
}
