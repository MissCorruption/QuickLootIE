#pragma once

#include "RE/G/GFxValue.h"

#include "ItemData.h"
#include "ItemDefines.h"

namespace QuickLoot::Items
{
	class ItemStack
	{
	public:
		ItemStack(ItemStack&&) = delete;
		ItemStack(const ItemStack&) = delete;
		ItemStack& operator=(ItemStack&&) = delete;
		ItemStack& operator=(const ItemStack&) = delete;

		ItemStack(RE::InventoryEntryData* entry, RE::ObjectRefHandle container, RE::ObjectRefHandle dropRef = {});
		virtual ~ItemStack() = default;

		[[nodiscard]] RE::InventoryEntryData* GetEntry() const { return _entry.get(); }
		[[nodiscard]] RE::ObjectRefHandle GetContainer() const { return _container; }
		[[nodiscard]] RE::ObjectRefHandle GetDropRef() const { return _dropRef; }

		[[nodiscard]] virtual ItemData& GetData() const;
		[[nodiscard]] virtual RE::GFxValue& BuildDataObject(RE::GFxMovieView* view) const;

		virtual void TakeStack(RE::Actor* actor) const;
		virtual void TakeOne(RE::Actor* actor) const;
		virtual void Take(RE::Actor* actor, int count) const;
		virtual void Use(RE::Actor* actor) const;
		virtual const char* GetUseLabel() const;

	protected:
		std::unique_ptr<RE::InventoryEntryData> _entry;
		RE::TESBoundObject* _object;
		RE::ObjectRefHandle _container;
		RE::ObjectRefHandle _dropRef;

		mutable ItemData _data{};
		mutable RE::GFxValue _dataObj{};
		mutable bool _dataInitialized = false;

		void SetVanillaData() const;

		void SkseExtendItemData() const;
		void SkseExtendCommonItemData() const;
		void SkseExtendItemInfoData() const;
		void SkseExtendStandardItemData() const;
		void SkseExtendInventoryData() const;
		void SkseExtendMagicItemData() const;

		void SkyUiProcessEntry() const;
		void SkyUiProcessArmorClass() const;
		void SkyUiProcessArmorPartMask() const;
		void SkyUiProcessArmorOther() const;
		void SkyUiProcessArmorKnownForms() const;
		void SkyUiProcessMaterialKeywords() const;
		void SkyUiProcessBookType() const;
		void SkyUiProcessMiscType() const;
		void SkyUiProcessMiscKnownForms() const;
		void SkyUiProcessWeaponType() const;
		void SkyUiProcessWeaponKnownForms() const;
		void SkyUiProcessAmmoType() const;
		void SkyUiProcessAmmoKnownForms() const;
		void SkyUiProcessKeyType() const;
		void SkyUiProcessPotionType() const;
		void SkyUiProcessSoulGemType() const;
		void SkyUiProcessSoulGemStatus() const;
		void SkyUiProcessSoulGemKnownForms() const;

		void SkyUiSelectIcon() const;
		void SkyUiSelectScrollColor() const;
		void SkyUiSelectArmorIcon() const;
		void SkyUiSelectLightArmorIcon() const;
		void SkyUiSelectHeavyArmorIcon() const;
		void SkyUiSelectClothingIcon() const;
		void SkyUiSelectJewelryIcon() const;
		void SkyUiSelectBookIcon() const;
		void SkyUiSelectMiscIcon() const;
		void SkyUiSelectWeaponIcon() const;
		void SkyUiSelectAmmoIcon() const;
		void SkyUiSelectPotionIcon() const;
		void SkyUiSelectSoulGemIcon() const;
		void SkyUiSelectSoulGemStatusIcon(bool grand) const;

		void I4ExtendItemData() const;

		// Helpers

		bool IsStealing() const;
		ItemType GetItemType() const;
		int GetPickpocketChance() const;

		static float RoundValue(float value);
		static float TruncatePrecision(float value);

		EnchantmentType GetEnchantmentType() const;

		// Native calls

		static void ShowItemInfo(RE::ItemCard* itemCard, RE::InventoryEntryData* entry, bool isContainerItem);
		static bool IsPlayerAllowedToTakeItemWithValue(RE::PlayerCharacter* player, RE::TESForm* ownerNpcOrFaction, int value);
		static void RefreshEnchantedWeapons(RE::Actor* actor, RE::InventoryChanges* changes);
		static RE::InventoryEntryData* GetInventoryEntryAt(RE::InventoryChanges* changes, int index);
		static RE::ExtraDataList* GetInventoryEntryExtraListForRemoval(RE::InventoryEntryData* entry, int count, bool isViewingContainer);

	public:
		template <typename TItemStack = ItemStack>
		static std::vector<std::unique_ptr<TItemStack>> LoadContainerInventory(RE::TESObjectREFR* container,
			const std::function<bool(RE::TESBoundObject&)>& filter = RE::TESObjectREFR::DEFAULT_INVENTORY_FILTER)
		{
			static_assert(std::is_constructible_v<TItemStack, RE::InventoryEntryData*, RE::ObjectRefHandle, RE::ObjectRefHandle>, "Missing TItemStack constructor");

			if (!container) {
				return {};
			}

			const auto containerHandle = container->GetHandle();
			const auto changes = container->GetInventoryChanges();

			if (const auto actor = skyrim_cast<RE::Actor*>(container)) {
				RefreshEnchantedWeapons(actor, changes);
			}

			std::vector<std::unique_ptr<TItemStack>> inventory{};
			std::map<RE::TESBoundObject*, size_t> lookup{};

			// Changed items

			if (changes && changes->entryList) {
				for (const auto entry : *changes->entryList) {
					if (!entry->object || !filter(*entry->object)) {
						continue;
					}

					lookup.emplace(entry->object, inventory.size());
					inventory.emplace_back(std::make_unique<TItemStack>(new RE::InventoryEntryData(*entry), containerHandle));
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
						inventory.emplace_back(std::make_unique<TItemStack>(new RE::InventoryEntryData(object, entry.count), containerHandle));
					} else {
						inventory[it->second]->_entry->countDelta += entry.count;
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

					inventory.emplace_back(std::make_unique<TItemStack>(entry, containerHandle, dropRefHandle));
				}
			}

			std::erase_if(inventory, [](const auto& stack) {
				return stack->_entry->countDelta <= 0;
			});

			return inventory;
		}

		template <typename TItemStack = ItemStack>
		static void MarkBestInClassItems(const std::vector<std::unique_ptr<TItemStack>>& inventory, bool includePlayerInventory = true)
		{
			struct BestItem
			{
				TItemStack* stack;
				float value;
			};

			BestItem bestArmorBySlot[32]{};
			BestItem bestOneHandWeapon{};
			BestItem bestTwoHandWeapon{};
			BestItem bestRangedWeapon{};
			BestItem bestAmmo{};

			const auto UpdateBest = [](BestItem& bestItem, float value, TItemStack* stack) {
				if (value > bestItem.value) {
					bestItem.value = value;
					bestItem.stack = stack;
				}
			};

			const auto CheckBest = [&](RE::InventoryEntryData* entry, TItemStack* stack) {
				const auto player = RE::PlayerCharacter::GetSingleton();
				const auto object = entry->object;

				switch (object->formType.get()) {
				case RE::FormType::Armor:
					{
						const auto armor = skyrim_cast<RE::TESObjectARMO*>(object);
						unsigned long slot = 0;
						if (_BitScanForward(&slot, static_cast<unsigned long>(armor->GetSlotMask()))) {
							UpdateBest(bestArmorBySlot[slot], player->GetArmorValue(entry), stack);
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
							UpdateBest(bestOneHandWeapon, player->GetDamage(entry), stack);
							break;

						case RE::WEAPON_TYPE::kTwoHandAxe:
						case RE::WEAPON_TYPE::kTwoHandSword:
							UpdateBest(bestTwoHandWeapon, player->GetDamage(entry), stack);

						case RE::WEAPON_TYPE::kBow:
						case RE::WEAPON_TYPE::kCrossbow:
							UpdateBest(bestRangedWeapon, player->GetDamage(entry), stack);
							break;

						default:
							break;
						}
						break;
					}

				case RE::FormType::Ammo:
					{
						UpdateBest(bestAmmo, player->GetDamage(entry), stack);
						break;
					}

				default:
					break;
				}
			};

			for (const auto& stack : inventory) {
				CheckBest(stack->GetEntry(), stack.get());
			}

			if (includePlayerInventory) {
				// We don't need to actually build the inventory here. Just iterating the raw change list should do.

				const auto player = RE::PlayerCharacter::GetSingleton();
				const auto changes = player->GetInventoryChanges(false);

				for (const auto& entry : *changes->entryList) {
					CheckBest(entry, nullptr);
				}
			}

			for (auto& bestItem : bestArmorBySlot) {
				if (bestItem.stack) {
					bestItem.stack->GetData().bestInClass = true;
				}
			}

			if (bestOneHandWeapon.stack) {
				bestOneHandWeapon.stack->GetData().bestInClass = true;
			}

			if (bestTwoHandWeapon.stack) {
				bestTwoHandWeapon.stack->GetData().bestInClass = true;
			}

			if (bestRangedWeapon.stack) {
				bestRangedWeapon.stack->GetData().bestInClass = true;
			}

			if (bestAmmo.stack) {
				bestAmmo.stack->GetData().bestInClass = true;
			}
		}
	};
}
