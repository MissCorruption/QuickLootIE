#pragma once

#ifdef _MSC_VER
#	undef GetObject
#endif

#include "LOTD/LOTD.h"

namespace Items
{
	enum class kType
	{
		None,
		DefaultWeapon,
		WeaponSword,
		WeaponGreatSword,
		WeaponDaedra,
		WeaponDagger,
		WeaponWarAxe,
		WeaponBattleAxe,
		WeaponMace,
		WeaponHammer,
		WeaponStaff,
		WeaponBow,
		WeaponArrow,
		WeaponPickAxe,
		WeaponWoodAxe,
		WeaponCrossbow,
		WeaponBolt,

		DefaultArmor,

		LightArmorBody,
		LightArmorHead,
		LightArmorHands,
		LightArmorForearms,
		LightArmorFeet,
		LightArmorCalves,
		LightArmorShield,
		LightArmorMask,

		ArmorBody,
		ArmorHead,
		ArmorHands,
		ArmorForearms,
		ArmorFeet,
		ArmorCalves,
		ArmorShield,
		ArmorMask,
		ArmorBracer,
		ArmorDaedra,

		ClothingBody,
		ClothingRobe,
		ClothingHead,
		ClothingPants,
		ClothingHands,
		ClothingForearms,
		ClothingFeet,
		ClothingCalves,
		ClothingShoes,
		ClothingShield,
		ClothingMask,

		ArmorAmulet,
		ArmorRing,
		Circlet,

		DefaultScroll,

		DefaultBook,
		BookRead,
		BookTome,
		TomeRead,
		BookJournal,
		BookNote,
		BookMap,

		DefaultFood,
		FoodWine,
		FoodBeer,

		DefaultIngredient,

		DefaultKey,
		KeyHouse,

		DefaultPotion,
		PotionHealth,
		PotionStam,
		PotionMagic,
		PotionPoison,
		PotionFrost,
		PotionFire,
		PotionShock,

		DefaultMisc,
		MiscArtifact,
		MiscClutter,
		MiscLockPick,
		MiscSoulGem,

		SoulGemEmpty,
		SoulGemPartial,
		SoulGemFull,
		SoulGemGrandEmpty,
		SoulGemGrandPartial,
		SoulGemGrandFull,
		SoulGemAzura,

		MiscGem,
		MiscOre,
		MiscIngot,
		MiscHide,
		MiscStrips,
		MiscLeather,
		MiscWood,
		MiscRemains,
		MiscTrollSkull,
		MiscTorch,
		MiscGoldSack,
		MiscGold,
		MiscDragonClaw
	};

	enum class EnchantmentType
	{
		None,
		Known,
		CannotDisenchant,
		Unknown
	};


	class GFxItem
	{
	public:

		GFxItem(std::ptrdiff_t a_count, bool a_stealing, SKSE::stl::observer<RE::InventoryEntryData*> a_item);
		GFxItem(std::ptrdiff_t a_count, bool a_stealing, std::span<const RE::ObjectRefHandle> a_items);
		[[nodiscard]] constexpr std::ptrdiff_t Count() const noexcept { return _count; }
		[[nodiscard]] int                      Compare(const GFxItem& a_rhs) const;
		[[nodiscard]] const std::string&       GetDisplayName() const;
		[[nodiscard]] double                   GetEnchantmentCharge() const;
		[[nodiscard]] bool                     IsEnchanted() const;
		[[nodiscard]] bool                     IsKnownEnchanted() const;
		[[nodiscard]] bool                     IsSpecialEnchanted() const;
		[[nodiscard]] RE::TESForm*             GetObject() const;
		[[nodiscard]] RE::FormID               GetFormID() const;
		[[nodiscard]] kType                    GetItemType() const;
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
		[[nodiscard]] bool                     ItemIsNew() const;
		[[nodiscard]] bool                     ItemIsFound() const;

		[[nodiscard]] bool                     ItemIsNeeded() const;

		[[nodiscard]] bool                     ItemIsCollected() const;
		[[nodiscard]] bool                     ItemIsDisplayed() const;
		[[nodiscard]] RE::GFxValue             GFxValue(RE::GFxMovieView& a_view) const;

	private:
		EnchantmentType GetEnchantmentType() const;
		void SetupEnchantmentFlags() const;

		kType GetItemType(RE::TESForm *form) const;
		const char* GetItemIconLabel(kType type) const;

		class Cache;

		#include "GFxItemCache.hpp"

		using inventory_t = RE::InventoryEntryData*;
		using ground_t = std::span<const RE::ObjectRefHandle>;

		std::variant<inventory_t, ground_t> _src;
		std::ptrdiff_t _count;
		mutable Cache _cache{};
		bool _stealing;
		kType _item_type;
	};

	[[nodiscard]] inline bool operator==(const GFxItem& a_lhs, const GFxItem& a_rhs) { return a_lhs.Compare(a_rhs) == 0; }
	[[nodiscard]] inline bool operator!=(const GFxItem& a_lhs, const GFxItem& a_rhs) { return a_lhs.Compare(a_rhs) != 0; }
	[[nodiscard]] inline bool operator< (const GFxItem& a_lhs, const GFxItem& a_rhs) { return a_lhs.Compare(a_rhs) <  0; }
	[[nodiscard]] inline bool operator> (const GFxItem& a_lhs, const GFxItem& a_rhs) { return a_lhs.Compare(a_rhs) >  0; }
	[[nodiscard]] inline bool operator<=(const GFxItem& a_lhs, const GFxItem& a_rhs) { return a_lhs.Compare(a_rhs) <= 0; }
	[[nodiscard]] inline bool operator>=(const GFxItem& a_lhs, const GFxItem& a_rhs) { return a_lhs.Compare(a_rhs) >= 0; }
}

