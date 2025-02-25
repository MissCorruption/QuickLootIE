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
		~ItemStack();

		[[nodiscard]] RE::InventoryEntryData* GetEntry() const { return _entry; }
		[[nodiscard]] RE::ObjectRefHandle GetContainer() const { return _container; }
		[[nodiscard]] RE::ObjectRefHandle GetDropRef() const { return _dropRef; }

		[[nodiscard]] ItemData& GetData();
		[[nodiscard]] RE::GFxValue& BuildDataObject(RE::GFxMovieView* view);

		void TakeStack(RE::Actor* actor) const;
		void TakeOne(RE::Actor* actor) const;
		void Take(RE::Actor* actor, int count) const;

		static std::vector<std::unique_ptr<ItemStack>> LoadContainerInventory(RE::TESObjectREFR* container,
			const std::function<bool(RE::TESBoundObject&)>& filter = RE::TESObjectREFR::DEFAULT_INVENTORY_FILTER);

	private:
		RE::InventoryEntryData* _entry;
		RE::TESBoundObject* _object;
		RE::ObjectRefHandle _container;
		RE::ObjectRefHandle _dropRef;

		ItemData _data{};
		RE::GFxValue _dataObj{};
		bool _dataInitialized = false;

		void SetVanillaData();

		void SkseExtendItemData();
		void SkseExtendCommonItemData();
		void SkseExtendItemInfoData();
		void SkseExtendStandardItemData();
		void SkseExtendInventoryData();
		void SkseExtendMagicItemData();

		void SkyUiProcessEntry();
		void SkyUiProcessArmorClass();
		void SkyUiProcessArmorPartMask();
		void SkyUiProcessArmorOther();
		void SkyUiProcessArmorKnownForms();
		void SkyUiProcessMaterialKeywords();
		void SkyUiProcessBookType();
		void SkyUiProcessMiscType();
		void SkyUiProcessMiscKnownForms();
		void SkyUiProcessWeaponType();
		void SkyUiProcessWeaponKnownForms();
		void SkyUiProcessAmmoType();
		void SkyUiProcessAmmoKnownForms();
		void SkyUiProcessKeyType();
		void SkyUiProcessPotionType();
		void SkyUiProcessSoulGemType();
		void SkyUiProcessSoulGemStatus();
		void SkyUiProcessSoulGemKnownForms();

		void SkyUiSelectIcon();
		void SkyUiSelectScrollColor();
		void SkyUiSelectArmorIcon();
		void SkyUiSelectLightArmorIcon();
		void SkyUiSelectHeavyArmorIcon();
		void SkyUiSelectClothingIcon();
		void SkyUiSelectJewelryIcon();
		void SkyUiSelectBookIcon();
		void SkyUiSelectMiscIcon();
		void SkyUiSelectWeaponIcon();
		void SkyUiSelectAmmoIcon();
		void SkyUiSelectPotionIcon();
		void SkyUiSelectSoulGemIcon();
		void SkyUiSelectSoulGemStatusIcon(bool grand);

		// Helpers

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
	};
}
