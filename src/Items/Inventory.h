#pragma once

#include "QuickLootAPI.h"

namespace QuickLoot::Items
{
	using InventoryEntry = API::ItemStack;

	class Inventory
	{
	public:
		Inventory() = delete;
		~Inventory() = delete;
		Inventory(Inventory&&) = delete;
		Inventory(const Inventory&) = delete;
		Inventory& operator=(Inventory&&) = delete;
		Inventory& operator=(const Inventory&) = delete;

		static RE::BSTArray<InventoryEntry> LoadContainerInventory(RE::TESObjectREFR* container,
			const std::function<bool(RE::TESBoundObject&)>& filter = RE::TESObjectREFR::DEFAULT_INVENTORY_FILTER);

		// returns a list of indices
		static std::vector<size_t> FindBestInClassItems(const RE::BSTArray<InventoryEntry>& inventory, bool includePlayerInventory = true);

	private:

		// Native functions

		static void RefreshEnchantedWeapons(RE::Actor* actor, RE::InventoryChanges* changes);
	};
}
