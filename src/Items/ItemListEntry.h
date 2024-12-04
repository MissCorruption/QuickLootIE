#pragma once

#ifdef _MSC_VER
#	undef GetObject
#endif

#include "ItemInfoCache.h"
#include "ItemType.h"

namespace QuickLoot::Items
{
	class ItemListEntry
	{
	public:

		ItemListEntry(std::ptrdiff_t a_count, bool a_stealing, SKSE::stl::observer<RE::InventoryEntryData*> a_item);
		ItemListEntry(std::ptrdiff_t a_count, bool a_stealing, std::span<const RE::ObjectRefHandle> a_items);
		[[nodiscard]] constexpr std::ptrdiff_t Count() const noexcept { return _count; }
		[[nodiscard]] int                      Compare(const ItemListEntry& a_rhs) const;
		[[nodiscard]] const std::string&       GetDisplayName() const;
		[[nodiscard]] double                   GetEnchantmentCharge() const;
		[[nodiscard]] bool                     IsEnchanted() const;
		[[nodiscard]] bool                     IsKnownEnchanted() const;
		[[nodiscard]] bool                     IsSpecialEnchanted() const;
		[[nodiscard]] RE::TESForm*             GetObject() const;
		[[nodiscard]] RE::FormID               GetFormID() const;
		[[nodiscard]] ItemType                    GetItemType() const;
		[[nodiscard]] std::ptrdiff_t           GetValue() const;
		[[nodiscard]] double                   GetWeight() const;
		[[nodiscard]] RE::SOUL_LEVEL           GetSoulSize() const;
		[[nodiscard]] bool                     IsAmmo() const;
		[[nodiscard]] bool                     IsBook() const;
		[[nodiscard]] bool                     IsRead() const;
		[[nodiscard]] bool                     IsGold() const;
		[[nodiscard]] bool                     IsKey() const;
		[[nodiscard]] bool                     IsLockpick() const;
		[[nodiscard]] bool                     IsNote() const;
		[[nodiscard]] bool                     IsQuestItem() const;
		[[nodiscard]] bool                     IsStolen() const;

		[[nodiscard]] bool                     ItemIsNewArtifact() const;
		[[nodiscard]] bool                     ItemIsFoundArtifact() const;
		[[nodiscard]] bool                     ItemIsDisplayedArtifact() const;

		[[nodiscard]] bool                     ItemIsCompletionistNew() const;
		[[nodiscard]] bool                     ItemIsCompletionistFound() const;

		[[nodiscard]] RE::GFxValue             GFxValue(RE::GFxMovieView& a_view) const;

	private:
		EnchantmentType GetEnchantmentType() const;
		void SetupEnchantmentFlags() const;

		ItemType GetItemType(RE::TESForm *form) const;
		const char* GetItemIconLabel(ItemType type) const;

		using inventory_t = RE::InventoryEntryData*;
		using ground_t = std::span<const RE::ObjectRefHandle>;
		
		std::variant<inventory_t, ground_t> _src;
		std::ptrdiff_t _count;
		mutable ItemInfoCache _cache{};
		bool _stealing;
		ItemType _item_type;
	};

	[[nodiscard]] inline bool operator==(const ItemListEntry& a_lhs, const ItemListEntry& a_rhs) { return a_lhs.Compare(a_rhs) == 0; }
	[[nodiscard]] inline bool operator!=(const ItemListEntry& a_lhs, const ItemListEntry& a_rhs) { return a_lhs.Compare(a_rhs) != 0; }
	[[nodiscard]] inline bool operator< (const ItemListEntry& a_lhs, const ItemListEntry& a_rhs) { return a_lhs.Compare(a_rhs) <  0; }
	[[nodiscard]] inline bool operator> (const ItemListEntry& a_lhs, const ItemListEntry& a_rhs) { return a_lhs.Compare(a_rhs) >  0; }
	[[nodiscard]] inline bool operator<=(const ItemListEntry& a_lhs, const ItemListEntry& a_rhs) { return a_lhs.Compare(a_rhs) <= 0; }
	[[nodiscard]] inline bool operator>=(const ItemListEntry& a_lhs, const ItemListEntry& a_rhs) { return a_lhs.Compare(a_rhs) >= 0; }
}

