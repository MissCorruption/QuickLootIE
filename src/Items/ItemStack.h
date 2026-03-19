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

		[[nodiscard]] ItemData& GetData() const { LoadData(); return _data; }
		[[nodiscard]] virtual RE::GFxValue& BuildDataObject(RE::GFxMovieView* view) const;

		virtual void LoadData() const;

		void TakeStack(RE::Actor* actor) const;
		void TakeOne(RE::Actor* actor) const;
		void Take(RE::Actor* actor, int count) const;
		void Use(RE::Actor* actor) const;
		const char* GetUseLabel() const;

	protected:
		std::unique_ptr<RE::InventoryEntryData> _entry;
		RE::TESBoundObject* _object;
		RE::ObjectRefHandle _container;
		RE::ObjectRefHandle _dropRef;

		mutable ItemData _data{};
		mutable RE::GFxValue _dataObj{};
		mutable bool _dataInitialized = false;

		void LoadVanillaData() const;

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

		static bool IsPlayerAllowedToTakeItemWithValue(RE::PlayerCharacter* player, RE::TESForm* ownerNpcOrFaction, int value);
		static RE::ExtraDataList* GetInventoryEntryExtraListForRemoval(RE::InventoryEntryData* entry, int count, bool isViewingContainer);
	};
}
