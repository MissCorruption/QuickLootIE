#include "LootMenu.h"
#include "RE/F/FormTypes.h"
#include "Util/Profiler.h"

#include <algorithm>
#include <functional>
#include <map>
#include <string>

namespace QuickLoot
{
	namespace SortHelpers
	{
		template <typename T>
		static int Sign(T value) {
			return (T(0) < value) - (value < T(0));
		}

		template <typename T>
		static int OrderAscending(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b, T (*func)(const Items::QuickLootItemStack&))
		{
			return Sign(func(a) - func(b));
		}

		template <typename T>
		static int OrderAscending(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b, T (*func)(const Items::ItemData&))
		{
			return Sign(func(a.GetData()) - func(b.GetData()));
		}

		template <typename T>
		static int OrderDescending(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b, T (*func)(const Items::QuickLootItemStack&))
		{
			return Sign(func(b) - func(a));
		}

		template <typename T>
		static int OrderDescending(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b, T (*func)(const Items::ItemData&))
		{
			return Sign(func(b.GetData()) - func(a.GetData()));
		}

		template <RE::FormType type>
		static bool IsFormType(const Items::ItemData& data)
		{
			return data.formType == type;
		}

		template <Items::ItemType type>
		static bool IsItemType(const Items::ItemData& data)
		{
			return data.type == type;
		}

		template <Items::MiscType type>
		static bool IsMiscItem(const Items::ItemData& data)
		{
			return data.type == Items::ItemType::kMisc && data.misc.subType.valid && data.misc.subType == type;
		}
	}

	namespace SortRules
	{
		using namespace SortHelpers;

		static int Gold(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b)
		{
			return OrderDescending<bool>(a, b, IsMiscItem<Items::MiscType::kGold>);
		}

		static int Gems(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b)
		{
			return OrderDescending<bool>(a, b, IsMiscItem<Items::MiscType::kGem>);
		}

		static int SoulGems(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b)
		{
			return OrderDescending<bool>(a, b, IsItemType<Items::ItemType::kSoulGem>);
		}

		static int Lockpicks(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b)
		{
			return OrderDescending<bool>(a, b, IsMiscItem<Items::MiscType::kLockpick>);
		}

		static int OresIngots(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b)
		{
			return OrderDescending<bool>(a, b, IsMiscItem<Items::MiscType::kIngot>);
		}

		static int Potions(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b)
		{
			return OrderDescending<bool>(a, b, [](const Items::ItemData& data) {
				return data.formType == RE::FormType::AlchemyItem &&
				       data.potion.subType != Items::PotionType::kFood &&
				       data.potion.subType != Items::PotionType::kDrink;
			});
		}

		static int FoodDrinks(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b)
		{
			return OrderDescending<bool>(a, b, [](const Items::ItemData& data) {
				return data.formType == RE::FormType::AlchemyItem &&
				       (data.potion.subType == Items::PotionType::kFood ||
						   data.potion.subType == Items::PotionType::kDrink);
			});
		}

		static int Books(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b)
		{
			constexpr auto IsBook = [](const Items::ItemData& data) {
				return data.formType == RE::FormType::Book &&
				       (!data.book.subType.valid ||
						   data.book.subType != Items::BookSubType::kNote &&
							   data.book.subType != Items::BookSubType::kRecipe);
			};

			const bool bookA = IsBook(a.GetData());
			const bool bookB = IsBook(b.GetData());

			if (bookA && bookB) {
				return a.GetData().book.isRead - b.GetData().book.isRead;
			}

			return bookA - bookB;
		}

		static int Notes(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b)
		{
			constexpr auto IsNote = [](const Items::ItemData& data) {
				return data.formType == RE::FormType::Book &&
				       data.book.subType.valid &&
				       (data.book.subType == Items::BookSubType::kNote ||
						   data.book.subType == Items::BookSubType::kRecipe);
			};

			const bool noteA = IsNote(a.GetData());
			const bool noteB = IsNote(b.GetData());

			if (noteA && noteB) {
				return a.GetData().book.isRead - b.GetData().book.isRead;
			}

			return noteA - noteB;
		}

		static int Scrolls(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b)
		{
			return OrderDescending(a, b, IsFormType<RE::FormType::Scroll>);
		}

		static int Weapons(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b)
		{
			return OrderDescending(a, b, IsFormType<RE::FormType::Weapon>);
		}

		static int ArrowsBolts(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b)
		{
			return OrderDescending(a, b, IsFormType<RE::FormType::Ammo>);
		}

		static int Armor(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b)
		{
			return OrderDescending<bool>(a, b, [](const Items::ItemData& data) {
				return IsFormType<RE::FormType::Armor>(data) &&
				       data.armor.weightClass != Items::ArmorWeightClass::kClothing &&
				       data.armor.weightClass != Items::ArmorWeightClass::kJewelry;
			});
		}

		static int Clothes(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b)
		{
			return OrderDescending<bool>(a, b, [](const Items::ItemData& data) {
				return IsFormType<RE::FormType::Armor>(data) &&
				       data.armor.weightClass == Items::ArmorWeightClass::kClothing;
			});
		}

		static int Jewelry(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b)
		{
			return OrderDescending<bool>(a, b, [](const Items::ItemData& data) {
				return IsFormType<RE::FormType::Armor>(data) &&
				       data.armor.weightClass == Items::ArmorWeightClass::kJewelry;
			});
		}

		static int Weightless(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b)
		{
			return OrderDescending<bool>(a, b, [](const Items::ItemData& data) {
				return data.weight.value <= 0;
			});
		}

		static int ByWeight(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b)
		{
			return OrderAscending<float>(a, b, [](const Items::ItemData& data) {
				return data.weight.value;
			});
		}

		static int ByValue(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b)
		{
			return OrderDescending<int32_t>(a, b, [](const Items::ItemData& data) {
				return data.value.value;
			});
		}

		static int ByValuePerWeight(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b)
		{
			return OrderDescending<float>(a, b, [](const Items::ItemData& data) {
				if (data.weight <= 0)
					return std::numeric_limits<float>::infinity();
				if (data.value <= 0)
					return 0.0f;
				return data.value / data.weight;
			});
		}

		static int ByName(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b)
		{
			const auto& nameA = a.GetQuickLootData().displayName.value.c_str();
			const auto& nameB = b.GetQuickLootData().displayName.value.c_str();
			return strcmp(nameA, nameB);
		}

		static int ArtifactNeeded(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b)
		{
			return OrderDescending<bool>(a, b, [](const Items::QuickLootItemStack& stack) {
				return stack.GetQuickLootData().artifactNew.value;
			});
		}

		static int CompletionistNeeded(Items::QuickLootItemStack& a, Items::QuickLootItemStack& b)
		{
			return OrderDescending<bool>(a, b, [](const Items::QuickLootItemStack& stack) {
				return stack.GetQuickLootData().compNeeded.value;
			});
		}
	}

	using namespace SortRules;

	static std::map<std::string, LootMenu::SortRule> ruleTable = {
		{ "$qlie_SortRule_Gold", Gold },
		{ "$qlie_SortRule_Gems", Gems },
		{ "$qlie_SortRule_SoulGems", SoulGems },
		{ "$qlie_SortRule_Lockpicks", Lockpicks },
		{ "$qlie_SortRule_OresIngots", OresIngots },
		{ "$qlie_SortRule_Potions", Potions },
		{ "$qlie_SortRule_FoodDrinks", FoodDrinks },
		{ "$qlie_SortRule_Books", Books },
		{ "$qlie_SortRule_Notes", Notes },
		{ "$qlie_SortRule_Scrolls", Scrolls },
		{ "$qlie_SortRule_Weapons", Weapons },
		{ "$qlie_SortRule_ArrowsBolts", ArrowsBolts },
		{ "$qlie_SortRule_Armors", Armor },
		{ "$qlie_SortRule_Clothes", Clothes },
		{ "$qlie_SortRule_Jewelry", Jewelry },
		{ "$qlie_SortRule_Weightless", Weightless },
		{ "$qlie_SortRule_ByWeight", ByWeight },
		{ "$qlie_SortRule_ByValue", ByValue },
		{ "$qlie_SortRule_ByV/W", ByValuePerWeight },
		{ "$qlie_SortRule_ByName", ByName },
		{ "$qlie_SortRule_ArtifactNeeded", ArtifactNeeded },
		{ "$qlie_SortRule_CompletionistNeeded", CompletionistNeeded },
	};

	const std::map<std::string, LootMenu::SortRule>& LootMenu::GetAvailableSortRules()
	{
		return ruleTable;
	}

	void LootMenu::SortInventory()
	{
		PROFILE_SCOPE;

		std::vector<SortRule> selectedRules{};

		for (auto& ruleName : Config::UserSettings::GetActiveSortRules()) {
			const auto it = ruleTable.find(ruleName);
			if (it != ruleTable.end()) {
				selectedRules.push_back(it->second);
			}
		}

		std::ranges::stable_sort(_inventory, [&](const auto& a, const auto& b) {
			for (auto ruleFunction : selectedRules) {
				const auto cmp = ruleFunction(*a, *b);
				if (cmp < 0)
					return true;
				if (cmp > 0)
					return false;
			}

			return ByName(*a, *b) < 0;
		});
	}
}
