#pragma once
#include "Inventory.h"

namespace QuickLoot::Items
{
	RE::BSTArray<InventoryEntry> Inventory::LoadContainerInventory(RE::TESObjectREFR* container,
		const std::function<bool(RE::TESBoundObject&)>& filter)
	{
		if (!container) {
			return {};
		}

		const auto changes = container->GetInventoryChanges();

		if (const auto actor = skyrim_cast<RE::Actor*>(container)) {
			RefreshEnchantedWeapons(actor, changes);
		}

		RE::BSTArray<InventoryEntry> inventory{};
		std::map<RE::TESBoundObject*, RE::BSTArray<InventoryEntry>::size_type> lookup{};

		// Changed items

		if (changes && changes->entryList) {
			for (const auto entry : *changes->entryList) {
				if (!entry->object || !filter(*entry->object)) {
					continue;
				}

				lookup.emplace(entry->object, inventory.size());
				inventory.emplace_back(InventoryEntry{ new RE::InventoryEntryData(*entry), {} });
			}
		}

		// Base container items

		if (const auto baseContainer = container->GetContainer()) {
			baseContainer->ForEachContainerObject([&](RE::ContainerObject& entry) {
				const auto object = entry.obj;

				if (!object || !filter(*object) || skyrim_cast<RE::TESLevItem*>(object)) {
					return RE::BSContainer::ForEachResult::kContinue;
				}

				const auto it = lookup.find(object);

				if (it == lookup.end()) {
					lookup.emplace(object, inventory.size());
					inventory.emplace_back(InventoryEntry{ new RE::InventoryEntryData(object, entry.count), {} });
				} else {
					inventory[it->second].entry->countDelta += entry.count;
				}

				return RE::BSContainer::ForEachResult::kContinue;
			});
		}

		// Dropped items always appear as separate item stacks because we need to attach the drop ref to them.

		if (const auto extraDrops = container->extraList.GetByType<RE::ExtraDroppedItemList>()) {
			for (const auto& dropRefHandle : extraDrops->droppedItemList) {
				const auto reference = dropRefHandle.get();

				if (!reference) {
					continue;
				}

				const auto object = reference->GetObjectReference();

				if (!object || !filter(*object)) {
					continue;
				}

				const auto count = reference->extraList.GetCount();
				const auto entry = new RE::InventoryEntryData(object, count);

				// The game uses ExtraDataType::kItemDropper to attach an object reference to the entry,
				// but we can just save the handle directly.

				inventory.emplace_back(InventoryEntry{ entry, dropRefHandle });
			}
		}

		for (int i = 0; i < std::ssize(inventory); i++) {
			if (inventory[i].entry->countDelta <= 0) {
				inventory.erase(&inventory[i]);
				i--;
			}
		}

		return inventory;
	}

	std::vector<size_t> Inventory::FindBestInClassItems(const RE::BSTArray<InventoryEntry>& inventory, bool includePlayerInventory)
	{
		struct BestItem
		{
			int index = -1;
			float value = 0;
		};

		BestItem bestArmorBySlot[32]{};
		BestItem bestOneHandWeapon{};
		BestItem bestTwoHandWeapon{};
		BestItem bestRangedWeapon{};
		BestItem bestAmmo{};

		const auto UpdateBest = [](BestItem& bestItem, float value, int index) {
			if (value > bestItem.value) {
				bestItem.value = value;
				bestItem.index = index;
			}
		};

		const auto CheckBest = [&](RE::InventoryEntryData* entry, int index) {
			const auto player = RE::PlayerCharacter::GetSingleton();
			const auto object = entry->object;

			switch (object->formType.get()) {
			case RE::FormType::Armor:
				{
					const auto armor = skyrim_cast<RE::TESObjectARMO*>(object);
					unsigned long slot = 0;
					if (_BitScanForward(&slot, armor->GetSlotMask().underlying())) {
						UpdateBest(bestArmorBySlot[slot], player->GetArmorValue(entry), index);
					}
					break;
				}

			case RE::FormType::Weapon:
				{
					const auto weapon = skyrim_cast<RE::TESObjectWEAP*>(object);
					switch (weapon->weaponData.animationType.get()) {
					case RE::WEAPON_TYPE::kOneHandAxe:
					case RE::WEAPON_TYPE::kOneHandMace:
					case RE::WEAPON_TYPE::kOneHandSword:
					case RE::WEAPON_TYPE::kOneHandDagger:
						UpdateBest(bestOneHandWeapon, player->GetDamage(entry), index);
						break;

					case RE::WEAPON_TYPE::kTwoHandAxe:
					case RE::WEAPON_TYPE::kTwoHandSword:
						UpdateBest(bestTwoHandWeapon, player->GetDamage(entry), index);

					case RE::WEAPON_TYPE::kBow:
					case RE::WEAPON_TYPE::kCrossbow:
						UpdateBest(bestRangedWeapon, player->GetDamage(entry), index);
						break;

					default:
						break;
					}
					break;
				}

			case RE::FormType::Ammo:
				{
					UpdateBest(bestAmmo, player->GetDamage(entry), index);
					break;
				}

			default:
				break;
			}
		};

		for (int i = 0; i < std::ssize(inventory); i++) {
			CheckBest(inventory[i].entry, i);
		}

		if (includePlayerInventory) {
			// We don't need to actually build the inventory here. Just iterating the raw change list should do.

			const auto player = RE::PlayerCharacter::GetSingleton();
			const auto changes = player->GetInventoryChanges(false);

			for (const auto& entry : *changes->entryList) {
				CheckBest(entry, -1);
			}
		}

		std::vector<size_t> indices{};

		for (auto& bestItem : bestArmorBySlot) {
			if (bestItem.index >= 0) {
				indices.push_back(static_cast<size_t>(bestItem.index));
			}
		}

		if (bestOneHandWeapon.index >= 0) {
			indices.push_back(static_cast<size_t>(bestOneHandWeapon.index));
		}

		if (bestTwoHandWeapon.index >= 0) {
			indices.push_back(static_cast<size_t>(bestTwoHandWeapon.index));
		}

		if (bestRangedWeapon.index >= 0) {
			indices.push_back(static_cast<size_t>(bestRangedWeapon.index));
		}

		if (bestAmmo.index >= 0) {
			indices.push_back(static_cast<size_t>(bestAmmo.index));
		}

		return indices;
	}

	void Inventory::RefreshEnchantedWeapons(RE::Actor* actor, RE::InventoryChanges* changes)
	{
		// TODO implement this for VR
		if (REL::Module::IsVR()) return;

		using func_t = decltype(&RefreshEnchantedWeapons);
		REL::Relocation<func_t> func{ RELOCATION_ID(50946, 51823) };
		return func(actor, changes);
	}
}
