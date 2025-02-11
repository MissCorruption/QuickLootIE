#pragma once

#include "RE/G/GFxValue.h"

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

		ItemStack(RE::GFxMovieView* view, RE::ObjectRefHandle container, RE::InventoryEntryData* entry, RE::ObjectRefHandle dropRef = {});
		~ItemStack();

		RE::GFxValue& GetData();

		void OnSelected(RE::Actor* actor) const;
		void TakeStack(RE::Actor* actor) const;
		void TakeOne(RE::Actor* actor) const;
		void Take(RE::Actor* actor, int count) const;

		static std::vector<std::unique_ptr<ItemStack>> LoadContainerInventory(
			RE::GFxMovieView* view,
			RE::TESObjectREFR* container,
			const std::function<bool(RE::TESBoundObject&)>& filter = RE::TESObjectREFR::DEFAULT_INVENTORY_FILTER);

	private:
		RE::GFxValue _data;
		RE::GFxMovieView* _view;
		RE::TESBoundObject* _object;
		RE::InventoryEntryData* _entry;
		RE::ObjectRefHandle _container;
		RE::ObjectRefHandle _dropRef;

		void SetVanillaData();
		void SetQuickLootData();

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
		RE::GFxValue GetBasicFormInfo(RE::TESForm* form) const;
		RE::GFxValue GetKeywords() const;

		static RE::GFxValue GetMember(const RE::GFxValue& obj, const char* name)
		{
			RE::GFxValue member;

			obj.GetMember(name, &member);

			return member;
		}

		template <typename T>
		static T GetMember(const RE::GFxValue& obj, const char* name)
		{
			return GetMember(obj, name, T::kNone);
		}

		template <typename T>
		static T GetMember(const RE::GFxValue& obj, const char* name, T fallback)
		{
			RE::GFxValue member;

			obj.GetMember(name, &member);

			if (member.IsNumber()) {
				return static_cast<T>(member.GetNumber());
			}

			return fallback;
		}

		static double RoundValue(double value);
		static RE::GFxValue TruncatePrecision(double value, bool allowNegative = false);

		EnchantmentType GetEnchantmentType() const;

		// Native calls

		static void GetItemCardData(RE::ItemCard* itemCard, RE::InventoryEntryData* entry, bool isContainerItem);
		static bool IsPlayerAllowedToTakeItemWithValue(RE::PlayerCharacter* player, RE::TESForm* ownerNpcOrFaction, int value);
		static void RefreshEnchantedWeapons(RE::Actor* actor, RE::InventoryChanges* changes);
		static RE::InventoryEntryData* GetInventoryEntryAt(RE::InventoryChanges* changes, int index);
		static RE::ExtraDataList* GetInventoryEntryExtraListForRemoval(RE::InventoryEntryData* entry, int count, bool isViewingContainer);
	};
}
