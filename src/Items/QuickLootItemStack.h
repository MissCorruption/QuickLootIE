#pragma once

#include "ItemStack.h"

namespace QuickLoot::Items
{
	struct QuickLootItemData
	{
		MandatoryField<RE::BSString> displayName;
		OptionalField<uint32_t> textColor;

		OptionalField<bool> enchanted;
		OptionalField<bool> knownEnchanted;
		OptionalField<bool> specialEnchanted;

		OptionalField<bool> dbmNew;
		OptionalField<bool> dbmFound;
		OptionalField<bool> dbmDisplayed;

		OptionalField<bool> compNew;
		OptionalField<bool> compFound;
	};

	class QuickLootItemStack : public ItemStack
	{
	public:
		QuickLootItemStack(RE::InventoryEntryData* entry, RE::ObjectRefHandle container, RE::ObjectRefHandle dropRef = {});

		[[nodiscard]] QuickLootItemData& GetQuickLootData();
		[[nodiscard]] RE::GFxValue& BuildDataObject(RE::GFxMovieView* view) override;

		void OnSelected(RE::Actor* actor) const;
		void Take(RE::Actor* actor, int count) const override;

	private:
		mutable QuickLootItemData _data;
		mutable bool _dataInitialized = false;

		void SetQuickLootData();
	};
}
