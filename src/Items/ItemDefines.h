#pragma once

namespace QuickLoot::Items
{
#pragma region General

	const inline RE::GFxValue undefined{ RE::GFxValue::ValueType::kUndefined };
	const inline RE::GFxValue null{ RE::GFxValue::ValueType::kNull };

	enum class ItemType
	{
		kNone = 0,

		kArmor = 1,
		kWeapon = 2,
		kMisc = 3,
		kBook = 4,
		kFood = 5,
		kMagicItem = 6,
		kIngredient = 8,
		kKey = 9,
		kSoulGem = 12,
	};

#pragma endregion

#pragma region Known Forms

	struct KnownForm
	{
		RE::FormID localFormId;
		const char* modName = "Skyrim.esm";

		template <typename T>
		T* LookupForm() const
		{
			return skyrim_cast<T*>(RE::TESDataHandler::GetSingleton()->LookupForm(localFormId, modName));
		}
	};

	// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/Common/skyui/defines/Form.as#L168
	namespace KnownForms
	{
		// Armor
		constexpr KnownForm ClothesWeddingWreath{ 0x08895A };
		constexpr KnownForm DLC1ClothesVampireLordArmor{ 0x011A84, "Dawnguard.esm" };

		// Weapons
		constexpr KnownForm weapPickaxe{ 0x0E3C16 };
		constexpr KnownForm SSDRocksplinterPickaxe{ 0x06A707 };
		constexpr KnownForm dunVolunruudPickaxe{ 0x1019D4 };

		constexpr KnownForm Axe01{ 0x02F2F4 };
		constexpr KnownForm dunHaltedStreamPoachersAxe{ 0x0AE086 };

		// Ammo
		constexpr KnownForm boundArrow{ 0x10B0A7 };
		constexpr KnownForm CWArrow{ 0x020DDF };
		constexpr KnownForm CWArrowShort{ 0x020F02 };
		constexpr KnownForm DaedricArrow{ 0x0139C0 };
		constexpr KnownForm DraugrArrow{ 0x034182 };
		constexpr KnownForm dunArcherPracticeArrow{ 0x0CAB52 };
		constexpr KnownForm dunGeirmundSigdisArrowsIllusion{ 0x0E738A };
		constexpr KnownForm DwarvenArrow{ 0x0139BC };
		constexpr KnownForm DwarvenSphereArrow{ 0x07B932 };
		constexpr KnownForm DwarvenSphereBolt01{ 0x07B935 };
		constexpr KnownForm DwarvenSphereBolt02{ 0x10EC8C };
		constexpr KnownForm EbonyArrow{ 0x0139BF };
		constexpr KnownForm ElvenArrow{ 0x0139BD };
		constexpr KnownForm FalmerArrow{ 0x038341 };
		constexpr KnownForm FollowerIronArrow{ 0x10E2DE };
		constexpr KnownForm ForswornArrow{ 0x0CEE9E };
		constexpr KnownForm GlassArrow{ 0x0139BE };
		constexpr KnownForm IronArrow{ 0x01397D };
		constexpr KnownForm MQ101SteelArrow{ 0x105EE7 };
		constexpr KnownForm NordHeroArrow{ 0x0EAFDF };
		constexpr KnownForm OrcishArrow{ 0x0139BB };
		constexpr KnownForm SteelArrow{ 0x01397F };
		constexpr KnownForm TrapDart{ 0x0236DD };
		constexpr KnownForm DLC1ElvenArrowBlessed{ 0x0098A1, "Dawnguard.esm" };
		constexpr KnownForm DLC1ElvenArrowBlood{ 0x0098A0, "Dawnguard.esm" };
		constexpr KnownForm TestDLC1Bolt{ 0x00590C, "Dawnguard.esm" };
		constexpr KnownForm DLC2DwarvenBallistaBolt{ 0x0339A1, "Dragonborn.esm" };
		constexpr KnownForm DLC2RieklingSpearThrown{ 0x017720, "Dragonborn.esm" };

		// Alchemy
		constexpr KnownForm ITMPotionUse{ 0x0B6435 };

		// Soul Gems
		constexpr KnownForm DA01AzurasStarBroken{ 0x028AD7 };
		constexpr KnownForm DA01SoulGemAzurasStar{ 0x063B27 };
		constexpr KnownForm DA01SoulGemBlackStar{ 0x063B29 };

		// Misc
		constexpr KnownForm Lockpick{ 0x00000A };
		constexpr KnownForm Gold001{ 0x00000F };

		constexpr KnownForm Leather01{ 0x0DB5D2 };
		constexpr KnownForm LeatherStrips{ 0x0800E4 };

		constexpr KnownForm GemAmethystFlawless{ 0x06851E };

		constexpr KnownForm dunDeadMensRubyDragonClaw{ 0x04B56C };
		constexpr KnownForm dunFolgunthurIvoryDragonClaw{ 0x0AB7BB };
		constexpr KnownForm dunForelhostGlassClaw{ 0x07C260 };
		constexpr KnownForm dunKorvanjundEbonyDragonClaw{ 0x05AF48 };
		constexpr KnownForm dunReachwaterRockEmeraldDragonClaw{ 0x0ED417 };
		constexpr KnownForm dunSkuldafnDiamondClaw{ 0x0AB375 };
		constexpr KnownForm dunValthumeIronClaw{ 0x08CDFA };
		constexpr KnownForm dunYngolBarrowCoralClaw{ 0x0B634C };
		constexpr KnownForm E3GoldenClaw{ 0x0999E7 };
		constexpr KnownForm FFI01Claw{ 0x0663D7 };
		constexpr KnownForm MS13GoldenClaw{ 0x039647 };

		// Other
		constexpr KnownForm MagicDisallowEnchanting{ 0x0C27BD };
	}

	inline bool operator==(const RE::TESForm* form, const KnownForm& knownForm)
	{
		return form && form->formID == RE::TESDataHandler::GetSingleton()->LookupFormID(knownForm.localFormId, knownForm.modName);
	}

#pragma endregion

#pragma region Materials

	// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/Common/skyui/defines/Material.as#L3
	enum class MaterialType : uint8_t
	{
		kAetherium = 0,
		kArtifact = 1,
		kBonemold = 2,
		kBrotherhood = 3,
		kChitin = 4,
		kClothing = 5,
		kDaedric = 6,
		kDawnguard = 7,
		kDragonbone = 8,
		kDragonplate = 9,
		kDragonscale = 10,
		kDraugr = 11,
		kDraugrHoned = 12,
		kDeathbrand = 13,
		kDwarven = 14,
		kEbony = 15,
		kElven = 16,
		kElvenGilded = 17,
		kFalmer = 18,
		kFalmerHardened = 19,
		kFalmerHoned = 20,
		kFur = 21,
		kGlass = 22,
		kHide = 23,
		kHunter = 24,
		kImperial = 25,
		kImperialStudded = 26,
		kIron = 27,
		kIronBanded = 28,
		kLeather = 29,
		kMagic = 30,
		kMoragTong = 31,
		kNightingale = 32,
		kNordic = 33,
		kOrcish = 34,
		kScaled = 35,
		kSilver = 36,
		kStalhrim = 37,
		kSteel = 38,
		kSteelPlate = 39,
		kStormcloak = 40,
		kStudded = 41,
		kVampire = 42,
		kWood = 43,
	};

	struct MaterialTableEntry
	{
		MaterialType material;
		const char* materialDisplay;
		std::vector<std::string> keywords;
	};

	// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryDataSetter.as#L153
	const inline auto MaterialTable = std::vector<MaterialTableEntry>{
		{ MaterialType::kDaedric, "$Daedric", { "ArmorMaterialDaedric", "WeapMaterialDaedric" } },
		{ MaterialType::kDragonplate, "$Dragonplate", { "ArmorMaterialDragonplate" } },
		{ MaterialType::kDragonscale, "$Dragonscale", { "ArmorMaterialDragonscale" } },
		{ MaterialType::kDwarven, "$Dwarven", { "ArmorMaterialDwarven", "WeapMaterialDwarven" } },
		{ MaterialType::kEbony, "$Ebony", { "ArmorMaterialEbony", "WeapMaterialEbony" } },
		{ MaterialType::kElven, "$Elven", { "ArmorMaterialElven", "WeapMaterialElven" } },
		{ MaterialType::kElvenGilded, "$Elven Gilded", { "ArmorMaterialElvenGilded" } },
		{ MaterialType::kGlass, "$Glass", { "ArmorMaterialGlass", "WeapMaterialGlass" } },
		{ MaterialType::kHide, "$Hide", { "ArmorMaterialHide" } },
		{ MaterialType::kImperial, "$Imperial", { "ArmorMaterialImperialHeavy", "ArmorMaterialImperialLight", "WeapMaterialImperial" } },
		{ MaterialType::kImperialStudded, "$Studded", { "ArmorMaterialImperialStudded" } },
		{ MaterialType::kIron, "$Iron", { "ArmorMaterialIron", "WeapMaterialIron" } },
		{ MaterialType::kIronBanded, "$Iron Banded", { "ArmorMaterialIronBanded" } },
		{ MaterialType::kVampire, "$Vampire", { "DLC1ArmorMaterialVampire" } },
		{ MaterialType::kLeather, "$Leather", { "ArmorMaterialLeather" } },
		{ MaterialType::kOrcish, "$Orcish", { "ArmorMaterialOrcish", "WeapMaterialOrcish" } },
		{ MaterialType::kScaled, "$Scaled", { "ArmorMaterialScaled" } },
		{ MaterialType::kSteel, "$Steel", { "ArmorMaterialSteel", "WeapMaterialSteel" } },
		{ MaterialType::kSteelPlate, "$Steel Plate", { "ArmorMaterialSteelPlate" } },
		{ MaterialType::kStormcloak, "$Stormcloak", { "ArmorMaterialStormcloak" } },
		{ MaterialType::kStudded, "$Studded", { "ArmorMaterialStudded" } },
		{ MaterialType::kDawnguard, "$Dawnguard", { "DLC1ArmorMaterialDawnguard" } },
		{ MaterialType::kFalmerHardened, "$Falmer Hardened", { "DLC1ArmorMaterialFalmerHardened", "DLC1ArmorMaterialFalmerHeavy" } },
		{ MaterialType::kHunter, "$Hunter", { "DLC1ArmorMaterialHunter" } },
		{ MaterialType::kAetherium, "$Aetherium", { "DLC1LD_CraftingMaterialAetherium" } },
		{ MaterialType::kDragonbone, "$Dragonbone", { "DLC1WeapMaterialDragonbone" } },
		{ MaterialType::kBonemold, "$Bonemold", { "DLC2ArmorMaterialBonemoldHeavy", "DLC2ArmorMaterialBonemoldLight" } },
		{ MaterialType::kChitin, "$Chitin", { "DLC2ArmorMaterialChitinHeavy", "DLC2ArmorMaterialChitinLight" } },
		{ MaterialType::kMoragTong, "$Morag Tong", { "DLC2ArmorMaterialMoragTong" } },
		{ MaterialType::kNordic, "$Nordic", { "DLC2ArmorMaterialNordicHeavy", "DLC2ArmorMaterialNordicLight", "DLC2WeaponMaterialNordic" } },
		{ MaterialType::kDeathbrand, "Deathbrand", { "DLC2dunHaknirArmor" } },
		{ MaterialType::kStalhrim, "$Stalhrim", { "DLC2ArmorMaterialStalhrimHeavy", "DLC2ArmorMaterialStalhrimLight", "DLC2WeaponMaterialStalhrim" } },
		{ MaterialType::kDraugr, "$Draugr", { "WeapMaterialDraugr" } },
		{ MaterialType::kDraugrHoned, "$Draugr Honed", { "WeapMaterialDraugrHoned" } },
		{ MaterialType::kFalmer, "$Falmer", { "WeapMaterialFalmer" } },
		{ MaterialType::kFalmerHoned, "$Falmer Honed", { "WeapMaterialFalmerHoned" } },
		{ MaterialType::kSilver, "$Silver", { "WeapMaterialSilver" } },
		{ MaterialType::kWood, "$Wood", { "WeapMaterialWood" } },
	};

#pragma endregion

#pragma region Armor

	// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/Common/skyui/defines/Armor.as#L3
	enum class ArmorWeightClass : uint8_t
	{
		kLight = 0,
		kHeavy = 1,
		kNone = 2,
		kClothing = 3,
		kJewelry = 4,
	};

	// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/Common/skyui/defines/Armor.as#L77
	enum class ArmorSubType
	{
		kNone = -1,
		kHead = 0,
		kHair = 1,
		kLongHair = 2,
		kBody = 3,
		kForearms = 4,
		kHands = 5,
		kShield = 6,
		kCalves = 7,
		kFeet = 8,
		kCirclet = 9,
		kAmulet = 10,
		kEars = 11,
		kRing = 12,
		kTail = 13,
	};

	// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/Common/skyui/defines/Armor.as#L10
	using ArmorSlot = RE::BIPED_MODEL::BipedObjectSlot;

	// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/Common/skyui/defines/Armor.as#L43
	const inline auto ArmorSlotPrecedence = std::vector{
		ArmorSlot::kBody,
		ArmorSlot::kHair,
		ArmorSlot::kHands,
		ArmorSlot::kForearms,
		ArmorSlot::kFeet,
		ArmorSlot::kCalves,
		ArmorSlot::kShield,
		ArmorSlot::kAmulet,
		ArmorSlot::kRing,
		ArmorSlot::kLongHair,
		ArmorSlot::kEars,
		ArmorSlot::kHead,
		ArmorSlot::kCirclet,
		ArmorSlot::kTail,
		ArmorSlot::kModMouth,
		ArmorSlot::kModNeck,
		ArmorSlot::kModChestPrimary,
		ArmorSlot::kModBack,
		ArmorSlot::kModMisc1,
		ArmorSlot::kModPelvisPrimary,
		ArmorSlot::kDecapitateHead,
		ArmorSlot::kDecapitate,
		ArmorSlot::kModPelvisSecondary,
		ArmorSlot::kModLegRight,
		ArmorSlot::kModLegLeft,
		ArmorSlot::kModFaceJewelry,
		ArmorSlot::kModChestSecondary,
		ArmorSlot::kModShoulder,
		ArmorSlot::kModArmLeft,
		ArmorSlot::kModArmRight,
		ArmorSlot::kModMisc2,
		ArmorSlot::kFX01,
	};

	struct ArmorSubTypeTableEntry
	{
		ArmorSlot slot;
		ArmorSubType subType;
		const char* subTypeDisplay;
	};

	// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryDataSetter.as#L431
	const inline auto ArmorSubTypeTable = std::vector<ArmorSubTypeTableEntry>{
		{ ArmorSlot::kHead, ArmorSubType::kHead, "$Head" },
		{ ArmorSlot::kHair, ArmorSubType::kHair, "$Head" },
		{ ArmorSlot::kLongHair, ArmorSubType::kLongHair, "$Head" },
		{ ArmorSlot::kBody, ArmorSubType::kBody, "$Body" },
		{ ArmorSlot::kHands, ArmorSubType::kHands, "$Hands" },
		{ ArmorSlot::kForearms, ArmorSubType::kForearms, "$Forearms" },
		{ ArmorSlot::kAmulet, ArmorSubType::kAmulet, "$Amulet" },
		{ ArmorSlot::kRing, ArmorSubType::kRing, "$Ring" },
		{ ArmorSlot::kFeet, ArmorSubType::kFeet, "$Feet" },
		{ ArmorSlot::kCalves, ArmorSubType::kCalves, "$Calves" },
		{ ArmorSlot::kShield, ArmorSubType::kShield, "$Shield" },
		{ ArmorSlot::kCirclet, ArmorSubType::kCirclet, "$Circlet" },
		{ ArmorSlot::kEars, ArmorSubType::kEars, "$Ears" },
		{ ArmorSlot::kTail, ArmorSubType::kTail, "$Tail" },
	};

#pragma endregion

#pragma region Weapons

	// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/Common/skyui/defines/Weapon.as#L3
	using WeaponAnimationType = RE::WEAPON_TYPE;

	// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/Common/skyui/defines/Weapon.as#L24
	enum class WeaponType
	{
		kNone = -1,

		kMelee = 0,
		kSword = 1,
		kDagger = 2,
		kWarAxe = 3,
		kMace = 4,
		kGreatsword = 5,
		kBattleaxe = 6,
		kWarhammer = 7,
		kBow = 8,
		kCrossbow = 9,
		kStaff = 10,
		kPickaxe = 11,
		kWoodAxe = 12,
	};

	// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/Common/skyui/defines/Weapon.as#L42
	using AmmoFlags = RE::AMMO_DATA::Flag;

	// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/Common/skyui/defines/Weapon.as#L39
	enum class AmmoType
	{
		kNone = -1,

		kArrow = 0,
		kBolt = 1,
	};

#pragma endregion

#pragma region Books

	// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/Common/skyui/defines/Item.as#L55
	using BookFlags = RE::OBJ_BOOK::Flag;

	// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/Common/skyui/defines/Item.as#L60
	using BookType = RE::OBJ_BOOK::Type;

	// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/Common/skyui/defines/Item.as#L48
	enum class BookSubType
	{
		kNone = -1,
		kSpellTome = 0,
		kNote = 1,
		kRecipe = 2,
	};

#pragma endregion

#pragma region Alchemy

	// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/Common/skyui/defines/Item.as#L63
	using AlchemyFlags = RE::AlchemyItem::AlchemyFlag;

	// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/Common/skyui/defines/Item.as#L5
	enum class PotionType
	{
		kNone = -1,

		kHealth = 0,
		kHealRate = 1,
		kHealRateMult = 2,
		kMagicka = 3,
		kMagickaRate = 4,
		kMagickaRateMult = 5,
		kStamina = 6,
		kStaminaRate = 7,
		kStaminaRateMult = 8,
		kFireResist = 9,
		kElectricResist = 10,
		kFrostResist = 11,
		kPotion = 12,
		kDrink = 13,
		kFood = 14,
		kPoison = 15,
	};

	struct PotionTypeTableEntry
	{
		RE::ActorValue actorValue;
		PotionType subType;
		const char* subTypeDisplay;
	};

	const inline auto PotionTypeTable = std::vector<PotionTypeTableEntry>{
		{ RE::ActorValue::kHealth, PotionType::kHealth, "$Health" },
		{ RE::ActorValue::kHealRate, PotionType::kHealRate, "$Health" },
		{ RE::ActorValue::kHealRateMult, PotionType::kHealRateMult, "$Health" },
		{ RE::ActorValue::kMagicka, PotionType::kMagicka, "$Magicka" },
		{ RE::ActorValue::kMagickaRate, PotionType::kMagickaRate, "$Magicka" },
		{ RE::ActorValue::kMagickaRateMult, PotionType::kMagickaRateMult, "$Magicka" },
		{ RE::ActorValue::kStamina, PotionType::kStamina, "$Stamina" },
		{ RE::ActorValue::kStaminaRate, PotionType::kStaminaRate, "$Stamina" },
		{ RE::ActorValue::kStaminaRateMult, PotionType::kStaminaRateMult, "$Stamina" },
		{ RE::ActorValue::kResistFire, PotionType::kFireResist, "$Potion" },
		{ RE::ActorValue::kResistShock, PotionType::kElectricResist, "$Potion" },
		{ RE::ActorValue::kResistFrost, PotionType::kFrostResist, "$Potion" },
	};

#pragma endregion

#pragma region Soul Gems

	// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/Common/skyui/defines/Item.as#L72
	enum class SoulLevel
	{
		kNone = 0,
		kPetty = 1,
		kLesser = 2,
		kCommon = 3,
		kGreater = 4,
		kGrand = 5,
		kAzura = 6,
	};

	// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/Common/skyui/defines/Item.as#L72
	enum class SoulGemStatus
	{
		kNone = -1,

		kEmpty = 0,
		kPartial = 1,
		kFull = 2,
	};

#pragma endregion

#pragma region Magic

	using EffectFlags = RE::EffectSetting::EffectSettingData::Flag;

	// https://github.com/ahzaab/moreHUDSE/blob/0b6995a8628cec786f822d2e177eae46dcee0569/include/AHZTarget.h#L3
	enum class EnchantmentType
	{
		kNone,
		kKnown,
		kCannotDisenchant,
		kUnknown
	};

#pragma endregion

#pragma region Misc

	// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/Common/skyui/defines/Item.as#L23
	enum class MiscType
	{
		kNone = -1,

		kGem = 0,
		kDragonClaw = 1,
		kArtifact = 2,
		kLeather = 3,
		kLeatherStrips = 4,
		kHide = 5,
		kRemains = 6,
		kIngot = 7,
		kTool = 8,
		kChildrensClothes = 9,
		kToy = 10,
		kFirewood = 11,

		kFastener = 12,
		kWeaponRack = 13,
		kShelf = 14,
		kFurniture = 15,
		kExterior = 16,
		kContainer = 17,

		kHousePart = 18,
		kClutter = 19,
		kLockpick = 20,
		kGold = 21,
	};

	struct MiscSubTypeTableEntry
	{
		MiscType subType;
		const char* subTypeDisplay;
		std::vector<std::string> keywords;
	};

	// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryDataSetter.as#L774
	const inline auto MiscSubTypeTable = std::vector<MiscSubTypeTableEntry>{
		{ MiscType::kChildrensClothes, "$Clothing", { "BYOHAdoptionClothesKeyword" } },
		{ MiscType::kToy, "$Toy", { "BYOHAdoptionToyKeyword" } },
		{ MiscType::kHousePart, "$House Part", { "BYOHHouseCraftingCategoryWeaponRacks", "BYOHHouseCraftingCategoryShelf", "BYOHHouseCraftingCategoryFurniture", "BYOHHouseCraftingCategoryExterior", "BYOHHouseCraftingCategoryContainers", "BYOHHouseCraftingCategoryBuilding", "BYOHHouseCraftingCategorySmithing" } },
		{ MiscType::kArtifact, "$Artifact", { "VendorItemDaedricArtifact" } },
		{ MiscType::kGem, "$Gem", { "VendorItemGem" } },
		{ MiscType::kHide, "$Hide", { "VendorItemAnimalHide" } },
		{ MiscType::kTool, "$Tool", { "VendorItemTool" } },
		{ MiscType::kRemains, "$Remains", { "VendorItemAnimalPart" } },
		{ MiscType::kIngot, "$Ingot", { "VendorItemOreIngot" } },
		{ MiscType::kClutter, "$Clutter", { "VendorItemClutter" } },
		{ MiscType::kFirewood, "$Firewood", { "VendorItemFirewood" } },
	};

#pragma endregion
}
