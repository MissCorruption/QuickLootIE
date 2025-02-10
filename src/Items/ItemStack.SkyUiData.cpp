#include "ItemStack.h"
#include "ItemDefines.h"

namespace QuickLoot::Items
{
	void ItemStack::SkyUiProcessEntry()
	{
		PROFILE_SCOPE;

		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryDataSetter.as#L24

		const auto itemInfo = LoadItemCardInfo();
		{
			PROFILE_SCOPE_NAMED("Info Data");

			_data.SetMember("baseId", _object->formID & 0xFFFFFF);
			_data.SetMember("type", GetMember(itemInfo, "type"));

			_data.SetMember("isEquipped", GetMember<int>(_data, "equipState", 0) > 0);
			_data.SetMember("isStolen", GetMember(itemInfo, "stolen").GetBool());

			const auto value = GetMember(itemInfo, "value").GetNumber();
			const auto weight = GetMember(itemInfo, "weight").GetNumber();

			_data.SetMember("infoValue", TruncatePrecision(value));
			_data.SetMember("infoWeight", TruncatePrecision(weight));
			_data.SetMember("infoValueWeight", value > 0 && weight > 0 ? RoundValue(value / weight) : null);
		}

		switch (_object->formType.get()) {
		case RE::FormType::Scroll:
			_data.SetMember("subTypeDisplay", "$Scroll");

			_data.SetMember("duration", TruncatePrecision(GetMember(_data, "duration").GetNumber()));
			_data.SetMember("magnitude", TruncatePrecision(GetMember(_data, "magnitude").GetNumber()));
			break;

		case RE::FormType::Armor:
			_data.SetMember("isEnchanted", strlen(GetMember(itemInfo, "effects").GetString()) > 0);
			_data.SetMember("infoArmor", TruncatePrecision(GetMember(itemInfo, "armor").GetNumber()));

			SkyUiProcessArmorClass();
			SkyUiProcessArmorPartMask();
			SkyUiProcessMaterialKeywords();
			SkyUiProcessArmorOther();
			SkyUiProcessArmorOther();
			break;

		case RE::FormType::Book:
			SkyUiProcessBookType();
			break;

		case RE::FormType::Ingredient:
			_data.SetMember("subTypeDisplay", "$Ingredient");
			break;

		case RE::FormType::Light:
			_data.SetMember("subTypeDisplay", "$Torch");
			break;

		case RE::FormType::Misc:
			SkyUiProcessMiscType();
			SkyUiProcessMiscKnownForms();
			break;

		case RE::FormType::Weapon:
			_data.SetMember("isEnchanted", strlen(GetMember(itemInfo, "effects").GetString()) > 0);
			_data.SetMember("isPoisoned", GetMember(itemInfo, "poisoned").GetBool());
			_data.SetMember("infoDamage", TruncatePrecision(GetMember(itemInfo, "damage").GetNumber()));

			SkyUiProcessWeaponType();
			SkyUiProcessMaterialKeywords();
			SkyUiProcessWeaponKnownForms();
			break;

		case RE::FormType::Ammo:
			_data.SetMember("isEnchanted", strlen(GetMember(itemInfo, "effects").GetString()) > 0);
			_data.SetMember("infoDamage", TruncatePrecision(GetMember(itemInfo, "damage").GetNumber()));

			SkyUiProcessAmmoType();
			SkyUiProcessMaterialKeywords();
			SkyUiProcessAmmoKnownForms();
			break;

		case RE::FormType::KeyMaster:
			SkyUiProcessKeyType();
			break;

		case RE::FormType::AlchemyItem:
			_data.SetMember("duration", TruncatePrecision(GetMember(_data, "duration").GetNumber()));
			_data.SetMember("magnitude", TruncatePrecision(GetMember(_data, "magnitude").GetNumber()));

			SkyUiProcessPotionType();
			break;

		case RE::FormType::SoulGem:
			SkyUiProcessSoulGemType();
			SkyUiProcessSoulGemStatus();
			SkyUiProcessSoulGemKnownForms();
			break;

		default:
			break;
		}
	}

	void ItemStack::SkyUiProcessArmorClass()
	{
		PROFILE_SCOPE;

		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryDataSetter.as#L115

		if (const auto armor = skyrim_cast<RE::TESObjectARMO*>(_object)) {
			switch (static_cast<ArmorWeightClass>(armor->bipedModelData.armorType.underlying())) {
			case ArmorWeightClass::kLight:
				_data.SetMember("weightClassDisplay", "$Light");
				break;

			case ArmorWeightClass::kHeavy:
				_data.SetMember("weightClassDisplay", "$Heavy");
				break;

			case ArmorWeightClass::kNone:
				if (_object->HasKeywordByEditorID("VendorItemClothing")) {
					_data.SetMember("weightClass", ArmorWeightClass::kClothing);
					_data.SetMember("weightClassDisplay", "$Clothing");
					break;
				}

				if (_object->HasKeywordByEditorID("VendorItemJewelry")) {
					_data.SetMember("weightClass", ArmorWeightClass::kJewelry);
					_data.SetMember("weightClassDisplay", "$Jewelry");
					break;
				}

				_data.SetMember("weightClass", RE::GFxValue::ValueType::kNull);
				_data.SetMember("weightClassDisplay", "$Other");
				break;

			default:
				break;
			}
		}
	}

	void ItemStack::SkyUiProcessArmorPartMask()
	{
		PROFILE_SCOPE;

		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryDataSetter.as#L415

		if (const auto armor = skyrim_cast<RE::TESObjectARMO*>(_object)) {
			const auto partMask = armor->bipedModelData.bipedObjectSlots;
			auto mainPartMask = ArmorSlot::kNone;

			for (auto slot : ArmorSlotPrecedence) {
				if (partMask.all(slot)) {
					mainPartMask = slot;
					break;
				}
			}

			_data.SetMember("mainPartMask", mainPartMask);

			if (mainPartMask == ArmorSlot::kNone) {
				return;
			}

			for (const auto& partEntry : ArmorSubTypeTable) {
				if (partEntry.slot == mainPartMask) {
					_data.SetMember("subType", partEntry.subType);
					_data.SetMember("subTypeDisplay", partEntry.subTypeDisplay);
					return;
				}
			}
		}
	}

	void ItemStack::SkyUiProcessArmorOther()
	{
		PROFILE_SCOPE;

		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryDataSetter.as#L506

		if (GetMember(_data, "weightClass").IsNumber()) {
			return;
		}

		// Set fallback weight classes if no keywords were set.
		switch (GetMember<ArmorSlot>(_data, "mainPartMask")) {
		case ArmorSlot::kHead:
		case ArmorSlot::kHair:
		case ArmorSlot::kLongHair:
		case ArmorSlot::kBody:
		case ArmorSlot::kHands:
		case ArmorSlot::kForearms:
		case ArmorSlot::kFeet:
		case ArmorSlot::kCalves:
		case ArmorSlot::kShield:
		case ArmorSlot::kTail:
			_data.SetMember("weightClass", ArmorWeightClass::kClothing);
			_data.SetMember("weightClassDisplay", "$Clothing");
			break;

		case ArmorSlot::kAmulet:
		case ArmorSlot::kRing:
		case ArmorSlot::kCirclet:
		case ArmorSlot::kEars:
			_data.SetMember("weightClass", ArmorWeightClass::kJewelry);
			_data.SetMember("weightClassDisplay", "$Jewelry");
			break;

		default:
			break;
		}
	}

	void ItemStack::SkyUiProcessArmorKnownForms()
	{
		PROFILE_SCOPE;

		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryDataSetter.as#L536

		if (_object == KnownForms::ClothesWeddingWreath) {
			_data.SetMember("weightClass", ArmorWeightClass::kJewelry);
			_data.SetMember("weightClassDisplay", "$Jewelry");
		}

		if (_object == KnownForms::DLC1ClothesVampireLordArmor) {
			_data.SetMember("subType", ArmorSubType::kBody);
			_data.SetMember("subTypeDisplay", "$Body");
		}
	}

	void ItemStack::SkyUiProcessMaterialKeywords()
	{
		PROFILE_SCOPE;

		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryDataSetter.as#L145

		for (const auto& entry : MaterialTable) {
			if (_object->HasAnyKeywordByEditorID(entry.keywords)) {
				_data.SetMember("material", entry.material);
				_data.SetMember("materialDisplay", entry.materialDisplay);
				return;
			}
		}

		_data.SetMember("material", RE::GFxValue::ValueType::kNull);
		_data.SetMember("materialDisplay", "$Other");
	}

	void ItemStack::SkyUiProcessBookType()
	{
		PROFILE_SCOPE;

		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryDataSetter.as#L550

		if (const auto book = skyrim_cast<RE::TESObjectBOOK*>(_object)) {
			_data.SetMember("isRead", book->data.flags.all(BookFlags::kHasBeenRead));
		}

		if (_object->HasKeywordByEditorID("VendorItemRecipe")) {
			_data.SetMember("subType", BookSubType::kRecipe);
			_data.SetMember("subTypeDisplay", "$Recipe");
			return;
		}

		if (_object->HasKeywordByEditorID("VendorItemSpellTome")) {
			_data.SetMember("subType", BookSubType::kSpellTome);
			_data.SetMember("subTypeDisplay", "$Spell Tome");
			return;
		}

		if (GetMember<BookType>(_data, "bookType", BookType::kBookTome) == BookType::kNoteScroll) {
			_data.SetMember("subType", BookSubType::kNote);
			_data.SetMember("subTypeDisplay", "$Note");
			return;
		}

		_data.SetMember("subType", undefined);
		_data.SetMember("subTypeDisplay", "$Book");
	}

	void ItemStack::SkyUiProcessMiscType()
	{
		PROFILE_SCOPE;

		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryDataSetter.as#L774

		for (const auto& entry : MiscSubTypeTable) {
			if (_object->HasAnyKeywordByEditorID(entry.keywords)) {
				_data.SetMember("subType", entry.subType);
				_data.SetMember("subTypeDisplay", entry.subTypeDisplay);
				return;
			}
		}

		_data.SetMember("subType", undefined);
		_data.SetMember("subTypeDisplay", "$Misc");
	}

	void ItemStack::SkyUiProcessMiscKnownForms()
	{
		PROFILE_SCOPE;

		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryDataSetter.as#L836

		if (_object == KnownForms::Gold001) {
			_data.SetMember("subType", MiscSubType::kGold);
			_data.SetMember("subTypeDisplay", "$Gold");
			return;
		}

		if (_object == KnownForms::Lockpick) {
			_data.SetMember("subType", MiscSubType::kLockpick);
			_data.SetMember("subTypeDisplay", "$Lockpick");
			return;
		}

		if (_object == KnownForms::Leather01) {
			_data.SetMember("subType", MiscSubType::kLeather);
			_data.SetMember("subTypeDisplay", "$Leather");
			return;
		}

		if (_object == KnownForms::LeatherStrips) {
			_data.SetMember("subType", MiscSubType::kLeatherStrips);
			_data.SetMember("subTypeDisplay", "$Strips");
			return;
		}

		if (_object == KnownForms::GemAmethystFlawless) {
			_data.SetMember("subType", MiscSubType::kGem);
			_data.SetMember("subTypeDisplay", "$Gem");
			return;
		}

		if (_object == KnownForms::dunDeadMensRubyDragonClaw ||
			_object == KnownForms::dunFolgunthurIvoryDragonClaw ||
			_object == KnownForms::dunForelhostGlassClaw ||
			_object == KnownForms::dunKorvanjundEbonyDragonClaw ||
			_object == KnownForms::dunReachwaterRockEmeraldDragonClaw ||
			_object == KnownForms::dunSkuldafnDiamondClaw ||
			_object == KnownForms::dunValthumeIronClaw ||
			_object == KnownForms::dunYngolBarrowCoralClaw ||
			_object == KnownForms::E3GoldenClaw ||
			_object == KnownForms::FFI01Claw ||
			_object == KnownForms::MS13GoldenClaw) {
			_data.SetMember("subType", MiscSubType::kGem);
			_data.SetMember("subTypeDisplay", "$Gem");
			return;
		}
	}

	void ItemStack::SkyUiProcessWeaponType()
	{
		PROFILE_SCOPE;

		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryDataSetter.as#L325

		if (const auto weapon = skyrim_cast<RE::TESObjectWEAP*>(_object)) {
			auto animationType = weapon->weaponData.animationType;
			if (animationType >= WeaponAnimationType::kTotal) {
				animationType -= WeaponAnimationType::kTotal;
			}

			switch (animationType.get()) {
			case WeaponAnimationType::kHandToHandMelee:
				_data.SetMember("subType", WeaponType::kMelee);
				_data.SetMember("subTypeDisplay", "$Melee");
				break;

			case WeaponAnimationType::kOneHandSword:
				_data.SetMember("subType", WeaponType::kSword);
				_data.SetMember("subTypeDisplay", "$Sword");
				break;

			case WeaponAnimationType::kOneHandDagger:
				_data.SetMember("subType", WeaponType::kDagger);
				_data.SetMember("subTypeDisplay", "$Dagger");
				break;

			case WeaponAnimationType::kOneHandAxe:
				_data.SetMember("subType", WeaponType::kWarAxe);
				_data.SetMember("subTypeDisplay", "$War Axe");
				break;

			case WeaponAnimationType::kOneHandMace:
				_data.SetMember("subType", WeaponType::kMace);
				_data.SetMember("subTypeDisplay", "$Mace");
				break;

			case WeaponAnimationType::kTwoHandSword:
				_data.SetMember("subType", WeaponType::kGreatsword);
				_data.SetMember("subTypeDisplay", "$Greatsword");
				break;

			case WeaponAnimationType::kTwoHandAxe:
				if (_object->HasKeywordByEditorID("WeapTypeWarhammer")) {
					_data.SetMember("subType", WeaponType::kWarhammer);
					_data.SetMember("subTypeDisplay", "$Warhammer");
				} else {
					_data.SetMember("subType", WeaponType::kBattleaxe);
					_data.SetMember("subTypeDisplay", "$Battleaxe");
				}
				break;

			case WeaponAnimationType::kBow:
				_data.SetMember("subType", WeaponType::kBow);
				_data.SetMember("subTypeDisplay", "$Bow");
				break;

			case WeaponAnimationType::kStaff:
				_data.SetMember("subType", WeaponType::kStaff);
				_data.SetMember("subTypeDisplay", "$Staff");
				break;

			case WeaponAnimationType::kCrossbow:
				_data.SetMember("subType", WeaponType::kCrossbow);
				_data.SetMember("subTypeDisplay", "$Crossbow");
				break;

			default:
				_data.SetMember("subType", null);
				_data.SetMember("subTypeDisplay", "$Weapon");
				break;
			}
		}
	}

	void ItemStack::SkyUiProcessWeaponKnownForms()
	{
		PROFILE_SCOPE;

		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryDataSetter.as#L398

		if (_object == KnownForms::weapPickaxe ||
			_object == KnownForms::SSDRocksplinterPickaxe ||
			_object == KnownForms::dunVolunruudPickaxe) {
			_data.SetMember("subType", WeaponType::kPickaxe);
			_data.SetMember("subTypeDisplay", "$Pickaxe");
			return;
		}

		if (_object == KnownForms::Axe01 ||
			_object == KnownForms::dunHaltedStreamPoachersAxe) {
			_data.SetMember("subType", WeaponType::kWoodAxe);
			_data.SetMember("subTypeDisplay", "$Wood Axe");
			return;
		}
	}

	void ItemStack::SkyUiProcessAmmoType()
	{
		PROFILE_SCOPE;

		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryDataSetter.as#L574

		if (const auto ammo = skyrim_cast<RE::TESAmmo*>(_object)) {
			if (ammo->GetRuntimeData().data.flags.all(AmmoFlags::kNonBolt)) {
				_data.SetMember("subType", AmmoType::kArrow);
				_data.SetMember("subTypeDisplay", "$Arrow");
			} else {
				_data.SetMember("subType", AmmoType::kBolt);
				_data.SetMember("subTypeDisplay", "$Bolt");
			}
		}
	}

	void ItemStack::SkyUiProcessAmmoKnownForms()
	{
		PROFILE_SCOPE;

		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryDataSetter.as#L585

		if (_object == KnownForms::DaedricArrow) {
			_data.SetMember("material", MaterialType::kDaedric);
			_data.SetMember("materialDisplay", "$Daedric");
			return;
		}

		if (_object == KnownForms::EbonyArrow) {
			_data.SetMember("material", MaterialType::kEbony);
			_data.SetMember("materialDisplay", "$Ebony");
			return;
		}

		if (_object == KnownForms::GlassArrow) {
			_data.SetMember("material", MaterialType::kGlass);
			_data.SetMember("materialDisplay", "$Glass");
			return;
		}

		if (_object == KnownForms::ElvenArrow ||
			_object == KnownForms::DLC1ElvenArrowBlessed ||
			_object == KnownForms::DLC1ElvenArrowBlood) {
			_data.SetMember("material", MaterialType::kElven);
			_data.SetMember("materialDisplay", "$Elven");
			return;
		}

		if (_object == KnownForms::DwarvenArrow ||
			_object == KnownForms::DwarvenSphereArrow ||
			_object == KnownForms::DwarvenSphereBolt01 ||
			_object == KnownForms::DwarvenSphereBolt02 ||
			_object == KnownForms::DLC2DwarvenBallistaBolt) {
			_data.SetMember("material", MaterialType::kDwarven);
			_data.SetMember("materialDisplay", "$Dwarven");
			return;
		}

		if (_object == KnownForms::OrcishArrow) {
			_data.SetMember("material", MaterialType::kOrcish);
			_data.SetMember("materialDisplay", "$Orcish");
			return;
		}

		if (_object == KnownForms::NordHeroArrow) {
			_data.SetMember("material", MaterialType::kNordic);
			_data.SetMember("materialDisplay", "$Nordic");
			return;
		}

		if (_object == KnownForms::DraugrArrow) {
			_data.SetMember("material", MaterialType::kDraugr);
			_data.SetMember("materialDisplay", "$Draugr");
			return;
		}

		if (_object == KnownForms::FalmerArrow) {
			_data.SetMember("material", MaterialType::kFalmer);
			_data.SetMember("materialDisplay", "$Falmer");
			return;
		}

		if (_object == KnownForms::SteelArrow ||
			_object == KnownForms::MQ101SteelArrow) {
			_data.SetMember("material", MaterialType::kSteel);
			_data.SetMember("materialDisplay", "$Steel");
			return;
		}

		if (_object == KnownForms::IronArrow ||
			_object == KnownForms::CWArrow ||
			_object == KnownForms::CWArrowShort ||
			_object == KnownForms::TrapDart ||
			_object == KnownForms::dunArcherPracticeArrow ||
			_object == KnownForms::dunGeirmundSigdisArrowsIllusion ||
			_object == KnownForms::FollowerIronArrow ||
			_object == KnownForms::TestDLC1Bolt) {
			_data.SetMember("material", MaterialType::kIron);
			_data.SetMember("materialDisplay", "$Iron");
			return;
		}

		if (_object == KnownForms::ForswornArrow) {
			_data.SetMember("material", MaterialType::kHide);
			_data.SetMember("materialDisplay", "$Forsworn");
			return;
		}

		if (_object == KnownForms::DLC2RieklingSpearThrown) {
			_data.SetMember("material", MaterialType::kWood);
			_data.SetMember("materialDisplay", "$Wood");
			_data.SetMember("subTypeDisplay", "$Spear");
			return;
		}
	}

	void ItemStack::SkyUiProcessKeyType()
	{
		PROFILE_SCOPE;

		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryDataSetter.as#L659

		_data.SetMember("subTypeDisplay", "$Key");

		if (GetMember<int>(_data, "infoValue", 0) <= 0) {
			_data.SetMember("infoValue", null);
		}

		if (GetMember<int>(_data, "infoWeight", 0) <= 0) {
			_data.SetMember("infoWeight", null);
		}
	}

	void ItemStack::SkyUiProcessPotionType()
	{
		PROFILE_SCOPE;

		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryDataSetter.as#L670

		if (const auto alchemyItem = skyrim_cast<RE::AlchemyItem*>(_object)) {
			if (alchemyItem->data.flags.all(AlchemyFlags::kFoodItem)) {
				if (alchemyItem->data.consumptionSound == KnownForms::ITMPotionUse) {
					_data.SetMember("subType", PotionType::kDrink);
					_data.SetMember("subTypeDisplay", "$Drink");
				} else {
					_data.SetMember("subType", PotionType::kFood);
					_data.SetMember("subTypeDisplay", "$Food");
				}
				return;
			}

			if (alchemyItem->data.flags.all(AlchemyFlags::kPoison)) {
				_data.SetMember("subType", PotionType::kPoison);
				_data.SetMember("subTypeDisplay", "$Poison");
				return;
			}

			const auto actorValue = GetMember<RE::ActorValue>(_data, "actorValue");

			for (const auto& entry : PotionTypeTable) {
				if (entry.actorValue == actorValue) {
					_data.SetMember("subType", entry.subType);
					_data.SetMember("subTypeDisplay", entry.subTypeDisplay);
					return;
				}
			}

			_data.SetMember("subType", PotionType::kPotion);
			_data.SetMember("subTypeDisplay", "$Potion");
		}
	}

	void ItemStack::SkyUiProcessSoulGemType()
	{
		PROFILE_SCOPE;

		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryDataSetter.as#L744

		_data.SetMember("subTypeDisplay", "$Soul Gem");

		if (const auto soulGem = skyrim_cast<RE::TESSoulGem*>(_object)) {
			if (soulGem->soulCapacity) {
				_data.SetMember("subType", soulGem->soulCapacity.get());
			} else {
				_data.SetMember("subType", undefined);
			}
		}
	}

	void ItemStack::SkyUiProcessSoulGemStatus()
	{
		PROFILE_SCOPE;

		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryDataSetter.as#L754

		if (const auto soulGem = skyrim_cast<RE::TESSoulGem*>(_object)) {
			const auto gemLevel = soulGem->soulCapacity;
			const auto soulLevel = soulGem->currentSoul;

			if (!gemLevel || !soulLevel) {
				_data.SetMember("status", SoulGemStatus::kEmpty);
				return;
			}

			if (soulLevel < gemLevel) {
				_data.SetMember("status", SoulGemStatus::kPartial);
				return;
			}

			_data.SetMember("status", SoulGemStatus::kFull);
		}
	}

	void ItemStack::SkyUiProcessSoulGemKnownForms()
	{
		PROFILE_SCOPE;

		// https://github.com/schlangster/skyui/blob/835428728e2305865e220fdfc99d791434955eb1/src/ItemMenus/InventoryDataSetter.as#L764

		if (_object == KnownForms::DA01SoulGemAzurasStar ||
			_object == KnownForms::DA01SoulGemBlackStar) {
			_data.SetMember("subType", SoulLevel::kAzura);
		}
	}
}
