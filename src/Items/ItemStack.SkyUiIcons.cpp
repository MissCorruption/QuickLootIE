#include "ItemStack.h"
#include "ItemDefines.h"

namespace QuickLoot::Items
{
	void ItemStack::SkyUiSelectIcon()
	{
		PROFILE_SCOPE;

		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L40

		switch (_object->formType.get()) {
		case RE::FormType::Scroll:
			_data.SetMember("iconLabel", "default_scroll");
			SkyUiSelectScrollColor();
			break;

		case RE::FormType::Armor:
			SkyUiSelectArmorIcon();
			break;

		case RE::FormType::Book:
			SkyUiSelectBookIcon();
			break;

		case RE::FormType::Ingredient:
			_data.SetMember("iconLabel", "default_ingredient");
			break;

		case RE::FormType::Light:
			_data.SetMember("iconLabel", "misc_torch");
			break;

		case RE::FormType::Misc:
			SkyUiSelectMiscIcon();
			break;

		case RE::FormType::Weapon:
			SkyUiSelectWeaponIcon();
			break;

		case RE::FormType::Ammo:
			SkyUiSelectAmmoIcon();
			break;

		case RE::FormType::KeyMaster:
			_data.SetMember("iconLabel", "default_key");
			break;

		case RE::FormType::AlchemyItem:
			SkyUiSelectPotionIcon();
			break;

		case RE::FormType::SoulGem:
			SkyUiSelectSoulGemIcon();
			break;

		default:
			break;
		}
	}

	void ItemStack::SkyUiSelectScrollColor()
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L93

		switch (GetMember<RE::ActorValue>(_data, "resistance")) {
		case RE::ActorValue::kResistFire:
			_data.SetMember("iconColor", 0xC73636);
			break;

		case RE::ActorValue::kResistShock:
			_data.SetMember("iconColor", 0xFFFF00);
			break;

		case RE::ActorValue::kResistFrost:
			_data.SetMember("iconColor", 0x1FFBFF);
			break;

		default:
			break;
		}
	}

	void ItemStack::SkyUiSelectArmorIcon()
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L113

		_data.SetMember("iconLabel", "default_armor");
		_data.SetMember("iconColor", 0xEDDA87);

		switch (GetMember<ArmorWeightClass>(_data, "weightClass")) {
		case ArmorWeightClass::kLight:
			SkyUiSelectLightArmorIcon();
			break;

		case ArmorWeightClass::kHeavy:
			SkyUiSelectHeavyArmorIcon();
			break;

		case ArmorWeightClass::kClothing:
			SkyUiSelectClothingIcon();
			break;

		case ArmorWeightClass::kJewelry:
			SkyUiSelectJewelryIcon();
			break;

		default:
			break;
		}
	}

	void ItemStack::SkyUiSelectLightArmorIcon()
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L138

		_data.SetMember("iconColor", 0x756000);

		switch (GetMember<ArmorSubType>(_data, "subType")) {
		case ArmorSubType::kHead:
		case ArmorSubType::kHair:
		case ArmorSubType::kLongHair:
			_data.SetMember("iconLabel", "lightarmor_head");
			break;

		case ArmorSubType::kBody:
		case ArmorSubType::kTail:
			_data.SetMember("iconLabel", "lightarmor_body");
			break;

		case ArmorSubType::kHands:
			_data.SetMember("iconLabel", "lightarmor_hands");
			break;

		case ArmorSubType::kForearms:
			_data.SetMember("iconLabel", "lightarmor_forearms");
			break;

		case ArmorSubType::kFeet:
			_data.SetMember("iconLabel", "lightarmor_feet");
			break;

		case ArmorSubType::kCalves:
			_data.SetMember("iconLabel", "lightarmor_calves");
			break;

		case ArmorSubType::kShield:
			_data.SetMember("iconLabel", "lightarmor_shield");
			break;

		default:
			SkyUiSelectJewelryIcon();
			break;
		}
	}

	void ItemStack::SkyUiSelectHeavyArmorIcon()
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L185

		_data.SetMember("iconColor", 0x6B7585);

		switch (GetMember<ArmorSubType>(_data, "subType")) {
		case ArmorSubType::kHead:
		case ArmorSubType::kHair:
		case ArmorSubType::kLongHair:
			_data.SetMember("iconLabel", "armor_head");
			break;

		case ArmorSubType::kBody:
		case ArmorSubType::kTail:
			_data.SetMember("iconLabel", "armor_body");
			break;

		case ArmorSubType::kHands:
			_data.SetMember("iconLabel", "armor_hands");
			break;

		case ArmorSubType::kForearms:
			_data.SetMember("iconLabel", "armor_forearms");
			break;

		case ArmorSubType::kFeet:
			_data.SetMember("iconLabel", "armor_feet");
			break;

		case ArmorSubType::kCalves:
			_data.SetMember("iconLabel", "armor_calves");
			break;

		case ArmorSubType::kShield:
			_data.SetMember("iconLabel", "armor_shield");
			break;

		default:
			SkyUiSelectJewelryIcon();
			break;
		}
	}

	void ItemStack::SkyUiSelectClothingIcon()
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L251

		switch (GetMember<ArmorSubType>(_data, "subType")) {
		case ArmorSubType::kHead:
		case ArmorSubType::kHair:
		case ArmorSubType::kLongHair:
			_data.SetMember("iconLabel", "clothing_head");
			break;

		case ArmorSubType::kBody:
		case ArmorSubType::kTail:
			_data.SetMember("iconLabel", "clothing_body");
			break;

		case ArmorSubType::kHands:
			_data.SetMember("iconLabel", "clothing_hands");
			break;

		case ArmorSubType::kForearms:
			_data.SetMember("iconLabel", "clothing_forearms");
			break;

		case ArmorSubType::kFeet:
			_data.SetMember("iconLabel", "clothing_feet");
			break;

		case ArmorSubType::kCalves:
			_data.SetMember("iconLabel", "clothing_calves");
			break;

		case ArmorSubType::kShield:
			_data.SetMember("iconLabel", "clothing_shield");
			break;

		default:
			break;
		}
	}

	void ItemStack::SkyUiSelectJewelryIcon()
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L231

		switch (GetMember<ArmorSubType>(_data, "subType")) {
		case ArmorSubType::kLongHair:
			_data.SetMember("iconLabel", "armor_amulet");
			break;

		case ArmorSubType::kTail:
			_data.SetMember("iconLabel", "armor_ring");
			break;

		case ArmorSubType::kHands:
			_data.SetMember("iconLabel", "armor_circlet");
			break;

		default:
			break;
		}
	}

	void ItemStack::SkyUiSelectBookIcon()
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L292

		switch (GetMember<BookSubType>(_data, "subType")) {
		case BookSubType::kSpellTome:
			_data.SetMember("iconLabel", "book_tome");
			break;

		case BookSubType::kNote:
		case BookSubType::kRecipe:
			_data.SetMember("iconLabel", "book_note");
			break;

		default:
			_data.SetMember("iconLabel", "default_book");
			break;
		}
	}

	void ItemStack::SkyUiSelectMiscIcon()
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L514

		switch (GetMember<MiscSubType>(_data, "subType")) {
		case MiscSubType::kGem:
			_data.SetMember("iconLabel", "misc_gem");
			_data.SetMember("iconColor", 0xFFB0D1);
			break;

		case MiscSubType::kDragonClaw:
			_data.SetMember("iconLabel", "misc_dragonclaw");
			break;

		case MiscSubType::kArtifact:
			_data.SetMember("iconLabel", "misc_artifact");
			break;

		case MiscSubType::kLeather:
			_data.SetMember("iconLabel", "misc_leather");
			_data.SetMember("iconColor", 0xBA8D23);
			break;

		case MiscSubType::kLeatherStrips:
			_data.SetMember("iconLabel", "misc_strips");
			_data.SetMember("iconColor", 0xBA8D23);
			break;

		case MiscSubType::kHide:
			_data.SetMember("iconLabel", "misc_hide");
			_data.SetMember("iconColor", 0xDBB36E);
			break;

		case MiscSubType::kRemains:
			_data.SetMember("iconLabel", "misc_remains");
			break;

		case MiscSubType::kIngot:
			_data.SetMember("iconLabel", "misc_ingot");
			_data.SetMember("iconColor", 0x828282);
			break;

		case MiscSubType::kChildrensClothes:
			_data.SetMember("iconLabel", "clothing_body");
			_data.SetMember("iconColor", 0xEDDA87);
			break;

		case MiscSubType::kFirewood:
			_data.SetMember("iconLabel", "misc_wood");
			_data.SetMember("iconColor", 0xA89E8C);
			break;

		case MiscSubType::kClutter:
			_data.SetMember("iconLabel", "misc_clutter");
			break;

		case MiscSubType::kLockpick:
			_data.SetMember("iconLabel", "misc_lockpick");
			break;

		case MiscSubType::kGold:
			_data.SetMember("iconLabel", "misc_gold");
			_data.SetMember("iconColor", 0xCCCC33);
			break;

		default:
			_data.SetMember("iconLabel", "default_misc");
			break;
		}
	}

	void ItemStack::SkyUiSelectWeaponIcon()
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L309

		_data.SetMember("iconColor", 0xA4A5BF);

		switch (GetMember<WeaponType>(_data, "subType")) {
		case WeaponType::kSword:
			_data.SetMember("iconLabel", "weapon_sword");
			break;

		case WeaponType::kDagger:
			_data.SetMember("iconLabel", "weapon_dagger");
			break;

		case WeaponType::kWarAxe:
			_data.SetMember("iconLabel", "weapon_waraxe");
			break;

		case WeaponType::kMace:
			_data.SetMember("iconLabel", "weapon_mace");
			break;

		case WeaponType::kGreatsword:
			_data.SetMember("iconLabel", "weapon_greatsword");
			break;

		case WeaponType::kBattleaxe:
			_data.SetMember("iconLabel", "weapon_battleaxe");
			break;

		case WeaponType::kWarhammer:
			_data.SetMember("iconLabel", "weapon_hammer");
			break;

		case WeaponType::kBow:
			_data.SetMember("iconLabel", "weapon_bow");
			break;

		case WeaponType::kCrossbow:
			_data.SetMember("iconLabel", "weapon_crossbow");
			break;

		case WeaponType::kStaff:
			_data.SetMember("iconLabel", "weapon_staff");
			break;

		case WeaponType::kPickaxe:
			_data.SetMember("iconLabel", "weapon_pickaxe");
			break;

		case WeaponType::kWoodAxe:
			_data.SetMember("iconLabel", "weapon_woodaxe");
			break;

		default:
			_data.SetMember("iconLabel", "default_weapon");
			break;
		}
	}

	void ItemStack::SkyUiSelectAmmoIcon()
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L372

		_data.SetMember("iconColor", 0xA89E8C);

		if (GetMember<AmmoType>(_data, "subType") == AmmoType::kBolt) {
			_data.SetMember("iconLabel", "weapon_bolt");
		} else {
			_data.SetMember("iconLabel", "weapon_arrow");
		}
	}

	void ItemStack::SkyUiSelectPotionIcon()
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L387

		switch (GetMember<PotionType>(_data, "subType")) {
		case PotionType::kHealth:
		case PotionType::kHealRate:
		case PotionType::kHealRateMult:
			_data.SetMember("iconLabel", "potion_health");
			_data.SetMember("iconColor", 0xDB2E73);
			break;

		case PotionType::kMagicka:
		case PotionType::kMagickaRate:
		case PotionType::kMagickaRateMult:
			_data.SetMember("iconLabel", "potion_magic");
			_data.SetMember("iconColor", 0x2E9FDB);
			break;

		case PotionType::kStamina:
		case PotionType::kStaminaRate:
		case PotionType::kStaminaRateMult:
			_data.SetMember("iconLabel", "potion_stam");
			_data.SetMember("iconColor", 0x51DB2E);
			break;

		case PotionType::kFireResist:
			_data.SetMember("iconLabel", "potion_fire");
			_data.SetMember("iconColor", 0xC73636);
			break;

		case PotionType::kElectricResist:
			_data.SetMember("iconLabel", "potion_shock");
			_data.SetMember("iconColor", 0xEAAB00);
			break;

		case PotionType::kFrostResist:
			_data.SetMember("iconLabel", "potion_frost");
			_data.SetMember("iconColor", 0x1FFBFF);
			break;

		case PotionType::kDrink:
			_data.SetMember("iconLabel", "food_wine");
			break;

		case PotionType::kFood:
			_data.SetMember("iconLabel", "default_food");
			break;

		case PotionType::kPoison:
			_data.SetMember("iconLabel", "potion_poison");
			_data.SetMember("iconColor", 0xAD00B3);
			break;

		default:
			_data.SetMember("iconLabel", "default_potion");
			break;
		}
	}

	void ItemStack::SkyUiSelectSoulGemIcon()
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L444

		switch (GetMember<SoulLevel>(_data, "subType")) {
		case SoulLevel::kPetty:
			SkyUiSelectSoulGemStatusIcon(false);
			_data.SetMember("iconColor", 0xD7D4FF);
			break;

		case SoulLevel::kLesser:
			SkyUiSelectSoulGemStatusIcon(false);
			_data.SetMember("iconColor", 0xC0BAFF);
			break;

		case SoulLevel::kCommon:
			SkyUiSelectSoulGemStatusIcon(false);
			_data.SetMember("iconColor", 0xABA3FF);
			break;

		case SoulLevel::kGreater:
			SkyUiSelectSoulGemStatusIcon(true);
			_data.SetMember("iconColor", 0x948BFC);
			break;

		case SoulLevel::kGrand:
			SkyUiSelectSoulGemStatusIcon(true);
			_data.SetMember("iconColor", 0x7569FF);
			break;

		case SoulLevel::kAzura:
			_data.SetMember("iconLabel", "soulgem_azura");
			_data.SetMember("iconColor", 0x7569FF);
			break;

		default:
			_data.SetMember("iconLabel", "misc_soulgem");
			_data.SetMember("iconColor", 0xE3E0FF);
			break;
		}
	}

	void ItemStack::SkyUiSelectSoulGemStatusIcon(bool grand)
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L482
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L498

		switch (GetMember<SoulGemStatus>(_data, "status")) {
		case SoulGemStatus::kEmpty:
			_data.SetMember("iconLabel", grand ? "soulgem_grandempty" : "soulgem_empty");
			break;

		case SoulGemStatus::kPartial:
			_data.SetMember("iconLabel", grand ? "soulgem_grandpartial" : "soulgem_partial");
			break;

		case SoulGemStatus::kFull:
			_data.SetMember("iconLabel", grand ? "soulgem_grandfull" : "soulgem_full");
			break;

		default:
			_data.SetMember("iconLabel", "misc_soulgem");
			break;
		}
	}
}
