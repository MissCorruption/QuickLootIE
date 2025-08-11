#include "ItemStack.h"
#include "ItemDefines.h"

namespace QuickLoot::Items
{
	void ItemStack::SkyUiSelectIcon() const
	{
		PROFILE_SCOPE;

		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L40

		switch (_object->formType.get()) {
		case RE::FormType::Scroll:
			_data.iconLabel = "default_scroll";
			SkyUiSelectScrollColor();
			break;

		case RE::FormType::Armor:
			SkyUiSelectArmorIcon();
			break;

		case RE::FormType::Book:
			SkyUiSelectBookIcon();
			break;

		case RE::FormType::Ingredient:
			_data.iconLabel = "default_ingredient";
			break;

		case RE::FormType::Light:
			_data.iconLabel = "misc_torch";
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
			_data.iconLabel = "default_key";
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

	void ItemStack::SkyUiSelectScrollColor() const
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L93

		switch (_data.scroll.resistance) {
		case RE::ActorValue::kResistFire:
			_data.iconColor = 0xC73636;
			break;

		case RE::ActorValue::kResistShock:
			_data.iconColor = 0xFFFF00;
			break;

		case RE::ActorValue::kResistFrost:
			_data.iconColor = 0x1FFBFF;
			break;

		default:
			break;
		}
	}

	void ItemStack::SkyUiSelectArmorIcon() const
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L113

		_data.iconLabel = "default_armor";
		_data.iconColor = 0xEDDA87;

		switch (_data.armor.weightClass) {
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

	void ItemStack::SkyUiSelectLightArmorIcon() const
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L138

		_data.iconColor = 0x756000;

		switch (_data.armor.subType) {
		case ArmorSubType::kHead:
		case ArmorSubType::kHair:
		case ArmorSubType::kLongHair:
			_data.iconLabel = "lightarmor_head";
			break;

		case ArmorSubType::kBody:
		case ArmorSubType::kTail:
			_data.iconLabel = "lightarmor_body";
			break;

		case ArmorSubType::kHands:
			_data.iconLabel = "lightarmor_hands";
			break;

		case ArmorSubType::kForearms:
			_data.iconLabel = "lightarmor_forearms";
			break;

		case ArmorSubType::kFeet:
			_data.iconLabel = "lightarmor_feet";
			break;

		case ArmorSubType::kCalves:
			_data.iconLabel = "lightarmor_calves";
			break;

		case ArmorSubType::kShield:
			_data.iconLabel = "lightarmor_shield";
			break;

		default:
			SkyUiSelectJewelryIcon();
			break;
		}
	}

	void ItemStack::SkyUiSelectHeavyArmorIcon() const
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L185

		_data.iconColor = 0x6B7585;

		switch (_data.armor.subType) {
		case ArmorSubType::kHead:
		case ArmorSubType::kHair:
		case ArmorSubType::kLongHair:
			_data.iconLabel = "armor_head";
			break;

		case ArmorSubType::kBody:
		case ArmorSubType::kTail:
			_data.iconLabel = "armor_body";
			break;

		case ArmorSubType::kHands:
			_data.iconLabel = "armor_hands";
			break;

		case ArmorSubType::kForearms:
			_data.iconLabel = "armor_forearms";
			break;

		case ArmorSubType::kFeet:
			_data.iconLabel = "armor_feet";
			break;

		case ArmorSubType::kCalves:
			_data.iconLabel = "armor_calves";
			break;

		case ArmorSubType::kShield:
			_data.iconLabel = "armor_shield";
			break;

		default:
			SkyUiSelectJewelryIcon();
			break;
		}
	}

	void ItemStack::SkyUiSelectClothingIcon() const
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L251

		switch (_data.armor.subType) {
		case ArmorSubType::kHead:
		case ArmorSubType::kHair:
		case ArmorSubType::kLongHair:
			_data.iconLabel = "clothing_head";
			break;

		case ArmorSubType::kBody:
		case ArmorSubType::kTail:
			_data.iconLabel = "clothing_body";
			break;

		case ArmorSubType::kHands:
			_data.iconLabel = "clothing_hands";
			break;

		case ArmorSubType::kForearms:
			_data.iconLabel = "clothing_forearms";
			break;

		case ArmorSubType::kFeet:
			_data.iconLabel = "clothing_feet";
			break;

		case ArmorSubType::kCalves:
			_data.iconLabel = "clothing_calves";
			break;

		case ArmorSubType::kShield:
			_data.iconLabel = "clothing_shield";
			break;

		default:
			break;
		}
	}

	void ItemStack::SkyUiSelectJewelryIcon() const
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L231

		switch (_data.armor.subType) {
		case ArmorSubType::kLongHair:
			_data.iconLabel = "armor_amulet";
			break;

		case ArmorSubType::kTail:
			_data.iconLabel = "armor_ring";
			break;

		case ArmorSubType::kHands:
			_data.iconLabel = "armor_circlet";
			break;

		default:
			break;
		}
	}

	void ItemStack::SkyUiSelectBookIcon() const
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L292

		switch (_data.book.subType) {
		case BookSubType::kSpellTome:
			_data.iconLabel = "book_tome";
			break;

		case BookSubType::kNote:
		case BookSubType::kRecipe:
			_data.iconLabel = "book_note";
			break;

		default:
			_data.iconLabel = "default_book";
			break;
		}
	}

	void ItemStack::SkyUiSelectMiscIcon() const
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L514

		switch (_data.misc.subType) {
		case MiscType::kGem:
			_data.iconLabel = "misc_gem";
			_data.iconColor = 0xFFB0D1;
			break;

		case MiscType::kDragonClaw:
			_data.iconLabel = "misc_dragonclaw";
			break;

		case MiscType::kArtifact:
			_data.iconLabel = "misc_artifact";
			break;

		case MiscType::kLeather:
			_data.iconLabel = "misc_leather";
			_data.iconColor = 0xBA8D23;
			break;

		case MiscType::kLeatherStrips:
			_data.iconLabel = "misc_strips";
			_data.iconColor = 0xBA8D23;
			break;

		case MiscType::kHide:
			_data.iconLabel = "misc_hide";
			_data.iconColor = 0xDBB36E;
			break;

		case MiscType::kRemains:
			_data.iconLabel = "misc_remains";
			break;

		case MiscType::kIngot:
			_data.iconLabel = "misc_ingot";
			_data.iconColor = 0x828282;
			break;

		case MiscType::kChildrensClothes:
			_data.iconLabel = "clothing_body";
			_data.iconColor = 0xEDDA87;
			break;

		case MiscType::kFirewood:
			_data.iconLabel = "misc_wood";
			_data.iconColor = 0xA89E8C;
			break;

		case MiscType::kClutter:
			_data.iconLabel = "misc_clutter";
			break;

		case MiscType::kLockpick:
			_data.iconLabel = "misc_lockpick";
			break;

		case MiscType::kGold:
			_data.iconLabel = "misc_gold";
			_data.iconColor = 0xCCCC33;
			break;

		default:
			_data.iconLabel = "default_misc";
			break;
		}
	}

	void ItemStack::SkyUiSelectWeaponIcon() const
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L309

		_data.iconColor = 0xA4A5BF;

		switch (_data.weapon.subType) {
		case WeaponType::kSword:
			_data.iconLabel = "weapon_sword";
			break;

		case WeaponType::kDagger:
			_data.iconLabel = "weapon_dagger";
			break;

		case WeaponType::kWarAxe:
			_data.iconLabel = "weapon_waraxe";
			break;

		case WeaponType::kMace:
			_data.iconLabel = "weapon_mace";
			break;

		case WeaponType::kGreatsword:
			_data.iconLabel = "weapon_greatsword";
			break;

		case WeaponType::kBattleaxe:
			_data.iconLabel = "weapon_battleaxe";
			break;

		case WeaponType::kWarhammer:
			_data.iconLabel = "weapon_hammer";
			break;

		case WeaponType::kBow:
			_data.iconLabel = "weapon_bow";
			break;

		case WeaponType::kCrossbow:
			_data.iconLabel = "weapon_crossbow";
			break;

		case WeaponType::kStaff:
			_data.iconLabel = "weapon_staff";
			break;

		case WeaponType::kPickaxe:
			_data.iconLabel = "weapon_pickaxe";
			break;

		case WeaponType::kWoodAxe:
			_data.iconLabel = "weapon_woodaxe";
			break;

		default:
			_data.iconLabel = "default_weapon";
			break;
		}
	}

	void ItemStack::SkyUiSelectAmmoIcon() const
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L372

		_data.iconColor = 0xA89E8C;

		if (_data.ammo.subType == AmmoType::kBolt) {
			_data.iconLabel = "weapon_bolt";
		} else {
			_data.iconLabel = "weapon_arrow";
		}
	}

	void ItemStack::SkyUiSelectPotionIcon() const
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L387

		switch (_data.potion.subType) {
		case PotionType::kHealth:
		case PotionType::kHealRate:
		case PotionType::kHealRateMult:
			_data.iconLabel = "potion_health";
			_data.iconColor = 0xDB2E73;
			break;

		case PotionType::kMagicka:
		case PotionType::kMagickaRate:
		case PotionType::kMagickaRateMult:
			_data.iconLabel = "potion_magic";
			_data.iconColor = 0x2E9FDB;
			break;

		case PotionType::kStamina:
		case PotionType::kStaminaRate:
		case PotionType::kStaminaRateMult:
			_data.iconLabel = "potion_stam";
			_data.iconColor = 0x51DB2E;
			break;

		case PotionType::kFireResist:
			_data.iconLabel = "potion_fire";
			_data.iconColor = 0xC73636;
			break;

		case PotionType::kElectricResist:
			_data.iconLabel = "potion_shock";
			_data.iconColor = 0xEAAB00;
			break;

		case PotionType::kFrostResist:
			_data.iconLabel = "potion_frost";
			_data.iconColor = 0x1FFBFF;
			break;

		case PotionType::kDrink:
			_data.iconLabel = "food_wine";
			break;

		case PotionType::kFood:
			_data.iconLabel = "default_food";
			break;

		case PotionType::kPoison:
			_data.iconLabel = "potion_poison";
			_data.iconColor = 0xAD00B3;
			break;

		default:
			_data.iconLabel = "default_potion";
			break;
		}
	}

	void ItemStack::SkyUiSelectSoulGemIcon() const
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L444

		switch (_data.soulGem.subType) {
		case SoulLevel::kPetty:
			SkyUiSelectSoulGemStatusIcon(false);
			_data.iconColor = 0xD7D4FF;
			break;

		case SoulLevel::kLesser:
			SkyUiSelectSoulGemStatusIcon(false);
			_data.iconColor = 0xC0BAFF;
			break;

		case SoulLevel::kCommon:
			SkyUiSelectSoulGemStatusIcon(false);
			_data.iconColor = 0xABA3FF;
			break;

		case SoulLevel::kGreater:
			SkyUiSelectSoulGemStatusIcon(true);
			_data.iconColor = 0x948BFC;
			break;

		case SoulLevel::kGrand:
			SkyUiSelectSoulGemStatusIcon(true);
			_data.iconColor = 0x7569FF;
			break;

		case SoulLevel::kAzura:
			_data.iconLabel = "soulgem_azura";
			_data.iconColor = 0x7569FF;
			break;

		default:
			_data.iconLabel = "misc_soulgem";
			_data.iconColor = 0xE3E0FF;
			break;
		}
	}

	void ItemStack::SkyUiSelectSoulGemStatusIcon(bool grand) const
	{
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L482
		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryIconSetter.as#L498

		switch (_data.soulGem.status) {
		case SoulGemStatus::kEmpty:
			_data.iconLabel = grand ? "soulgem_grandempty" : "soulgem_empty";
			break;

		case SoulGemStatus::kPartial:
			_data.iconLabel = grand ? "soulgem_grandpartial" : "soulgem_partial";
			break;

		case SoulGemStatus::kFull:
			_data.iconLabel = grand ? "soulgem_grandfull" : "soulgem_full";
			break;

		default:
			_data.iconLabel = "misc_soulgem";
			break;
		}
	}
}
