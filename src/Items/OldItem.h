#pragma once

#include "Integrations/PluginServer.h"
#include "Items/ItemListEntry.h"

namespace QuickLoot::Items
{
	class OldItem
	{
	public:
		OldItem() = delete;
		OldItem(const OldItem&) = delete;
		OldItem(OldItem&&) = default;

		OldItem(std::ptrdiff_t a_count, bool a_stealing, stl::observer<RE::InventoryEntryData*> a_item) :
			_item(a_count, a_stealing, a_item)
		{}

		OldItem(std::ptrdiff_t a_count, bool a_stealing, std::span<const RE::ObjectRefHandle> a_items) :
			_item(a_count, a_stealing, a_items)
		{}

		virtual ~OldItem() = default;

		OldItem& operator=(const OldItem&) = delete;
		OldItem& operator=(OldItem&&) = default;

		[[nodiscard]] int Compare(const OldItem& a_rhs) const { return _item.Compare(a_rhs._item); }

		[[nodiscard]] RE::GFxValue GFxValue(RE::GFxMovieView& a_view) const { return _item.GFxValue(a_view); }

		void Take(RE::Actor& a_dst, std::ptrdiff_t a_count) { DoTake(a_dst, a_count); }
		void Take(RE::Actor& a_dst) { DoTake(a_dst, 1); }
		void TakeAll(RE::Actor& a_dst) { DoTake(a_dst, Count()); }
		void OnSelected(RE::Actor& a_dst) { DoSelect(a_dst); }

		virtual void FillElementsVector(std::vector<QuickLoot::Integrations::Element>* elements) = 0;

		[[nodiscard]] double EnchantmentCharge() const { return _item.GetEnchantmentCharge(); }

		[[nodiscard]] const std::string& Name() const { return _item.GetDisplayName(); }
		[[nodiscard]] std::ptrdiff_t Value() const { return _item.GetValue(); }
		[[nodiscard]] double Weight() const { return _item.GetWeight(); }

	protected:
		virtual void DoTake(RE::Actor& a_dst, std::ptrdiff_t a_count) = 0;
		virtual void DoSelect(RE::Actor& a_dst) = 0;

		[[nodiscard]] std::ptrdiff_t Count() const { return std::max<std::ptrdiff_t>(_item.Count(), 0); }
		[[nodiscard]] bool Stolen() const { return _item.IsStolen(); }

	private:
		ItemListEntry _item;
	};

	[[nodiscard]] inline bool operator==(const OldItem& a_lhs, const OldItem& a_rhs) { return a_lhs.Compare(a_rhs) == 0; }
	[[nodiscard]] inline bool operator!=(const OldItem& a_lhs, const OldItem& a_rhs) { return a_lhs.Compare(a_rhs) != 0; }
	[[nodiscard]] inline bool operator<(const OldItem& a_lhs, const OldItem& a_rhs) { return a_lhs.Compare(a_rhs) < 0; }
	[[nodiscard]] inline bool operator>(const OldItem& a_lhs, const OldItem& a_rhs) { return a_lhs.Compare(a_rhs) > 0; }
	[[nodiscard]] inline bool operator<=(const OldItem& a_lhs, const OldItem& a_rhs) { return a_lhs.Compare(a_rhs) <= 0; }
	[[nodiscard]] inline bool operator>=(const OldItem& a_lhs, const OldItem& a_rhs) { return a_lhs.Compare(a_rhs) >= 0; }
}
