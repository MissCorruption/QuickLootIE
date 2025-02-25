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

	class QuickLootItemStack
	{
	public:
		explicit QuickLootItemStack(std::unique_ptr<ItemStack> stack);

		[[nodiscard]] RE::InventoryEntryData* GetEntry() const { return _stack->GetEntry(); }
		[[nodiscard]] RE::ObjectRefHandle GetContainer() const { return _stack->GetContainer(); }
		[[nodiscard]] RE::ObjectRefHandle GetDropRef() const { return _stack->GetDropRef(); }

		[[nodiscard]] ItemData& GetData() const;
		[[nodiscard]] QuickLootItemData& GetQuickLootData() const;
		[[nodiscard]] RE::GFxValue& BuildDataObject(RE::GFxMovieView* view);

		void OnSelected(RE::Actor* actor) const;
		void TakeStack(RE::Actor* actor) const;
		void TakeOne(RE::Actor* actor) const;
		void Take(RE::Actor* actor, int count) const;

		static std::vector<QuickLootItemStack> LoadContainerInventory(
			RE::TESObjectREFR* container,
			const std::function<bool(RE::TESBoundObject&)>& filter = RE::TESObjectREFR::DEFAULT_INVENTORY_FILTER);

	private:
		std::unique_ptr<ItemStack> _stack;
		mutable QuickLootItemData _data;
		mutable bool _dataInitialized = false;

		void SetQuickLootData() const;
	};
}
