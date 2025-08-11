#include "ItemStack.h"
#include "ItemDefines.h"

namespace QuickLoot::Items
{
	void ItemStack::SkseExtendItemData() const
	{
		PROFILE_SCOPE;

		// https://github.com/ianpatt/skse64/blob/9843a236aa12b12fa4c6530c29113adfb941da72/skse64/Hooks_Scaleform.cpp#L1164

		SkseExtendCommonItemData();
		SkseExtendItemInfoData();
		SkseExtendStandardItemData();
		SkseExtendInventoryData();
		SkseExtendMagicItemData();

		// SKSE also invokes inventory plugins registered via its RegisterScaleformInventory function,
		// but we don't have access to those. It seems to be a pretty rarely used feature though.
	}

	void ItemStack::SkseExtendCommonItemData() const
	{
		PROFILE_SCOPE;

		// https://github.com/ianpatt/skse64/blob/9843a236aa12b12fa4c6530c29113adfb941da72/skse64/ScaleformExtendedData.cpp#L113

		_data.formType = _object->formType;
		_data.formId = _object->formID;
	}

	void ItemStack::SkseExtendItemInfoData() const
	{
		PROFILE_SCOPE;

		// https://github.com/ianpatt/skse64/blob/9843a236aa12b12fa4c6530c29113adfb941da72/skse64/ScaleformExtendedData.cpp#L782

		_data.value = _entry->GetValue();
		_data.weight = _object->GetWeight();
		//_data.isStolen = _entry->IsOwnedBy(RE::PlayerCharacter::GetSingleton(), true); // SkyUI overwrites this
	}

	void ItemStack::SkseExtendStandardItemData() const
	{
		PROFILE_SCOPE;

		// https://github.com/ianpatt/skse64/blob/9843a236aa12b12fa4c6530c29113adfb941da72/skse64/ScaleformExtendedData.cpp#L122

		switch (_object->formType.get()) {
		case RE::FormType::Armor:
			if (const auto armor = skyrim_cast<RE::TESObjectARMO*>(_object)) {
				_data.armor.partMask = armor->bipedModelData.bipedObjectSlots;
				_data.armor.weightClass = static_cast<ArmorWeightClass>(armor->bipedModelData.armorType.get());
			}
			break;

		case RE::FormType::Ammo:
			if (const auto ammo = skyrim_cast<RE::TESAmmo*>(_object)) {
				_data.ammo.flags = ammo->GetRuntimeData().data.flags;
			}
			break;

		case RE::FormType::Weapon:
			if (const auto weapon = skyrim_cast<RE::TESObjectWEAP*>(_object)) {
				_data.weapon.weaponType = weapon->weaponData.animationType;
				_data.weapon.speed = weapon->weaponData.speed;
				_data.weapon.reach = weapon->weaponData.reach;
				_data.weapon.stagger = weapon->weaponData.staggerValue;
				_data.weapon.critDamage = weapon->criticalData.damage;
				_data.weapon.minRange = weapon->weaponData.minRange;
				_data.weapon.maxRange = weapon->weaponData.maxRange;
				_data.weapon.baseDamage = weapon->GetAttackDamage();

				if (const auto equipSlot = weapon->GetEquipSlot()) {
					_data.weapon.equipSlot = equipSlot->formID;
				}
			}
			break;

		case RE::FormType::SoulGem:
			if (const auto soulGem = skyrim_cast<RE::TESSoulGem*>(_object)) {
				_data.soulGem.gemSize = static_cast<SoulLevel>(soulGem->soulCapacity.get());
				_data.soulGem.soulSize = static_cast<SoulLevel>(_entry->GetSoulLevel());
			}
			break;

		case RE::FormType::AlchemyItem:
			if (const auto alchemyItem = skyrim_cast<RE::AlchemyItem*>(_object)) {
				_data.potion.flags = alchemyItem->data.flags;
			}
			break;

		case RE::FormType::Book:
			if (const auto book = skyrim_cast<RE::TESObjectBOOK*>(_object)) {
				_data.book.flags = book->data.flags;
				_data.book.bookType = book->data.type;

				if (book->data.flags.all(BookFlags::kTeachesSpell)) {
					_data.book.teachesSpell = book->data.teaches.spell ? book->data.teaches.spell->formID : -1;
				} else if (book->data.flags.all(BookFlags::kAdvancesActorValue)) {
					_data.book.teachesSkill = book->data.teaches.actorValueToAdvance;
				}

				// We deviate from the SKSE implementation here to incorporate the fix by
				// https://www.nexusmods.com/skyrimspecialedition/mods/32561. It's usually
				// invoked as an SKSE Scaleform plugin, but we can't access it that way.

				static auto notePattern = std::regex(R"(^.*(?:Note|FishMap)[^\\\/]+$)", std::regex_constants::icase);
				if (book->inventoryModel && std::regex_match(book->inventoryModel->GetModel(), notePattern)) {
					_data.book.bookType = BookType::kNoteScroll;
				}
			}
			break;

		default:
			break;
		}
	}

	void ItemStack::SkseExtendInventoryData() const
	{
		PROFILE_SCOPE;

		// https://github.com/ianpatt/skse64/blob/9843a236aa12b12fa4c6530c29113adfb941da72/skse64/ScaleformExtendedData.cpp#L724

		if (const auto keywordForm = skyrim_cast<RE::BGSKeywordForm*>(_object)) {
			_data.keywords = keywordForm;
		}

		const auto player = RE::PlayerCharacter::GetSingleton();

		switch (_object->formType.get()) {
		case RE::FormType::Armor:
			_data.armor.armor = RoundValue(player->GetArmorValue(_entry.get()));
			break;

		case RE::FormType::Weapon:
			_data.weapon.damage = RoundValue(player->GetDamage(_entry.get()));
			break;

		case RE::FormType::Ammo:
			_data.ammo.damage = RoundValue(player->GetDamage(_entry.get()));
			break;

		default:
			break;
		}
	}

	void ItemStack::SkseExtendMagicItemData() const
	{
		PROFILE_SCOPE;

		// https://github.com/ianpatt/skse64/blob/9843a236aa12b12fa4c6530c29113adfb941da72/skse64/ScaleformExtendedData.cpp#L227

		if (const auto magicItem = skyrim_cast<RE::MagicItem*>(_object)) {
			_data.magic.spellName = magicItem->fullName.c_str();

			const auto effect = magicItem->GetCostliestEffectItem(static_cast<RE::MagicSystem::Delivery>(5), false);
			if (effect && effect->baseEffect) {
				_data.magic.magnitude = effect->effectItem.magnitude;
				_data.magic.duration = effect->effectItem.duration;
				_data.magic.area = effect->effectItem.area;

				const auto baseEffect = effect->baseEffect;
				_data.magic.effectName = baseEffect->fullName.c_str();
				_data.magic.subType = baseEffect->data.associatedSkill;
				_data.magic.effectFlags = baseEffect->data.flags;
				_data.magic.school = baseEffect->data.associatedSkill;
				_data.magic.skillLevel = baseEffect->data.minimumSkill;
				_data.magic.archetype = baseEffect->data.archetype;
				_data.magic.deliveryType = baseEffect->data.delivery;
				_data.magic.castTime = baseEffect->data.spellmakingChargeTime;
				_data.magic.delayTime = baseEffect->data.aiDelayTimer;
				_data.magic.actorValue = baseEffect->data.primaryAV;
				_data.magic.castType = baseEffect->data.castingType;
				_data.magic.resistance = baseEffect->data.resistVariable;  // SkyUI renames this from magicType to resistance
			}
		}

		if (const auto spellItem = skyrim_cast<RE::SpellItem*>(_object)) {
			_data.spell.spellType = spellItem->data.spellType;
			_data.spell.trueCost = spellItem->data.costOverride;

			if (const auto equipSlot = spellItem->GetEquipSlot()) {
				_data.spell.equipSlot = equipSlot->formID;
			}
		}

		if (const auto alchemyItem = skyrim_cast<RE::AlchemyItem*>(_object)) {
			if (const auto sound = alchemyItem->data.consumptionSound) {
				_data.potion.useSound = sound;
			}
		}

		if (const auto enchantmentItem = skyrim_cast<RE::EnchantmentItem*>(_object)) {
			_data.enchantment.flags = enchantmentItem->formFlags;
			_data.enchantment.baseEnchant = enchantmentItem->data.baseEnchantment;
			_data.enchantment.restrictions = enchantmentItem->data.wornRestrictions;
		}

		if (const auto shout = skyrim_cast<RE::TESShout*>(_object)) {
			_data.shout.fullName = shout->fullName.c_str();

			for (int i = 0; i < 3; ++i) {
				const auto& shoutWord = shout->variations[i];
				_data.shout.words[i].word = shoutWord.word->fullName.c_str();
				_data.shout.words[i].fullName = shoutWord.word->translation.c_str();
				_data.shout.words[i].recoveryTime = shoutWord.recoveryTime;
			}
		}
	}
}
