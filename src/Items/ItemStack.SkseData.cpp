#include "ItemStack.h"
#include "ItemDefines.h"

namespace QuickLoot::Items
{
	void ItemStack::SkseExtendItemData()
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

	void ItemStack::SkseExtendCommonItemData()
	{
		PROFILE_SCOPE;

		// https://github.com/ianpatt/skse64/blob/9843a236aa12b12fa4c6530c29113adfb941da72/skse64/ScaleformExtendedData.cpp#L113

		_data.SetMember("formType", _object->formType.underlying());
		_data.SetMember("formId", _object->formID);
	}

	void ItemStack::SkseExtendItemInfoData()
	{
		PROFILE_SCOPE;

		// https://github.com/ianpatt/skse64/blob/9843a236aa12b12fa4c6530c29113adfb941da72/skse64/ScaleformExtendedData.cpp#L782

		_data.SetMember("value", _entry->GetValue());
		_data.SetMember("weight", _object->GetWeight());
		_data.SetMember("isStolen", !_entry->IsOwnedBy(RE::PlayerCharacter::GetSingleton(), true));
	}

	void ItemStack::SkseExtendStandardItemData()
	{
		PROFILE_SCOPE;

		// https://github.com/ianpatt/skse64/blob/9843a236aa12b12fa4c6530c29113adfb941da72/skse64/ScaleformExtendedData.cpp#L122

		switch (_object->formType.get()) {
		case RE::FormType::Armor:
			if (const auto armor = skyrim_cast<RE::TESObjectARMO*>(_object)) {
				_data.SetMember("partMask", armor->bipedModelData.bipedObjectSlots.underlying());
				_data.SetMember("weightClass", armor->bipedModelData.armorType.underlying());
			}
			break;

		case RE::FormType::Ammo:
			if (const auto ammo = skyrim_cast<RE::TESAmmo*>(_object)) {
				_data.SetMember("flags", ammo->GetRuntimeData().data.flags.underlying());
			}
			break;

		case RE::FormType::Weapon:
			if (const auto weapon = skyrim_cast<RE::TESObjectWEAP*>(_object)) {
				_data.SetMember("subType", weapon->weaponData.animationType.underlying());
				_data.SetMember("weaponType", weapon->weaponData.animationType.underlying());
				_data.SetMember("speed", weapon->weaponData.speed);
				_data.SetMember("reach", weapon->weaponData.reach);
				_data.SetMember("stagger", weapon->weaponData.staggerValue);
				_data.SetMember("critDamage", weapon->criticalData.damage);
				_data.SetMember("minRange", weapon->weaponData.minRange);
				_data.SetMember("maxRange", weapon->weaponData.maxRange);
				_data.SetMember("baseDamage", weapon->GetAttackDamage());

				if (const auto equipSlot = weapon->GetEquipSlot()) {
					_data.SetMember("equipSlot", equipSlot->formID);
				}
			}
			break;

		case RE::FormType::SoulGem:
			if (const auto soulGem = skyrim_cast<RE::TESSoulGem*>(_object)) {
				_data.SetMember("gemSize", soulGem->soulCapacity.underlying());
				_data.SetMember("soulSize", _entry->GetSoulLevel());
			}
			break;

		case RE::FormType::AlchemyItem:
			if (const auto alchemyItem = skyrim_cast<RE::AlchemyItem*>(_object)) {
				_data.SetMember("flags", alchemyItem->data.flags.underlying());
			}
			break;

		case RE::FormType::Book:
			if (const auto book = skyrim_cast<RE::TESObjectBOOK*>(_object)) {
				_data.SetMember("flags", book->data.flags.underlying());
				_data.SetMember("bookType", book->data.type.underlying());

				if (book->data.flags.all(BookFlags::kTeachesSpell)) {
					_data.SetMember("teachesSpell", book->data.teaches.spell ? book->data.teaches.spell->formID : -1);
				} else if (book->data.flags.all(BookFlags::kAdvancesActorValue)) {
					_data.SetMember("teachesSkill", book->data.teaches.actorValueToAdvance);
				}

				// We deviate from the SKSE implementation here to incorporate the fix by
				// https://www.nexusmods.com/skyrimspecialedition/mods/32561. It's usually
				// invoked as an SKSE Scaleform plugin, but we can't access it that way.

				static auto notePattern = std::regex(R"(^.*(?:Note|FishMap)\d*[^\\\/]+$)", std::regex_constants::icase);
				if (book->inventoryModel && std::regex_match(book->inventoryModel->GetModel(), notePattern)) {
					_data.SetMember("bookType", BookType::kNoteScroll);
				}
			}
			break;

		default:
			break;
		}
	}

	void ItemStack::SkseExtendInventoryData()
	{
		PROFILE_SCOPE;

		// https://github.com/ianpatt/skse64/blob/9843a236aa12b12fa4c6530c29113adfb941da72/skse64/ScaleformExtendedData.cpp#L724

		_data.SetMember("keywords", GetKeywords());

		switch (_object->formType.get()) {
		case RE::FormType::Armor:
			_data.SetMember("armor", RoundValue(RE::PlayerCharacter::GetSingleton()->GetArmorValue(_entry)));
			break;

		case RE::FormType::Weapon:
		case RE::FormType::Ammo:
			_data.SetMember("damage", RoundValue(RE::PlayerCharacter::GetSingleton()->GetDamage(_entry)));
			break;

		default:
			break;
		}
	}

	void ItemStack::SkseExtendMagicItemData()
	{
		PROFILE_SCOPE;

		// https://github.com/ianpatt/skse64/blob/9843a236aa12b12fa4c6530c29113adfb941da72/skse64/ScaleformExtendedData.cpp#L227

		if (const auto magicItem = skyrim_cast<RE::MagicItem*>(_object)) {
			_data.SetMember("spellName", magicItem->fullName.c_str());

			const auto effect = magicItem->GetCostliestEffectItem(static_cast<RE::MagicSystem::Delivery>(5), false);
			if (effect && effect->baseEffect) {
				_data.SetMember("magnitude", effect->effectItem.magnitude);
				_data.SetMember("duration", effect->effectItem.duration);
				_data.SetMember("area", effect->effectItem.area);

				const auto baseEffect = effect->baseEffect;
				_data.SetMember("effectName", baseEffect->fullName.c_str());
				_data.SetMember("subType", baseEffect->data.associatedSkill);
				_data.SetMember("effectFlags", baseEffect->data.flags.underlying());
				_data.SetMember("school", baseEffect->data.associatedSkill);
				_data.SetMember("skillLevel", baseEffect->data.minimumSkill);
				_data.SetMember("archetype", baseEffect->data.archetype);
				_data.SetMember("deliveryType", baseEffect->data.delivery);
				_data.SetMember("castTime", baseEffect->data.spellmakingChargeTime);
				_data.SetMember("delayTime", baseEffect->data.aiDelayTimer);
				_data.SetMember("actorValue", baseEffect->data.primaryAV);
				_data.SetMember("castType", baseEffect->data.castingType);
				_data.SetMember("resistance", baseEffect->data.resistVariable);  // SkyUI renames this from magicType to resistance
			}
		}

		if (const auto spellItem = skyrim_cast<RE::SpellItem*>(_object)) {
			_data.SetMember("spellType", spellItem->data.spellType);
			_data.SetMember("trueCost", spellItem->data.costOverride);

			if (const auto equipSlot = spellItem->GetEquipSlot()) {
				_data.SetMember("equipSlot", equipSlot->formID);
			}
		}

		if (const auto alchemyItem = skyrim_cast<RE::AlchemyItem*>(_object)) {
			if (const auto sound = alchemyItem->data.consumptionSound) {
				_data.SetMember("useSound", GetBasicFormInfo(sound));
			}
		}

		if (const auto enchantmentItem = skyrim_cast<RE::EnchantmentItem*>(_object)) {
			_data.SetMember("flags", enchantmentItem->formFlags);
			_data.SetMember("baseEnchant", GetBasicFormInfo(enchantmentItem->data.baseEnchantment));
			_data.SetMember("restrictions", GetBasicFormInfo(enchantmentItem->data.wornRestrictions));
		}

		if (const auto shout = skyrim_cast<RE::TESShout*>(_object)) {
			_data.SetMember("fullName", shout->fullName.c_str());

			RE::GFxValue words;
			_view->CreateArray(&words);
			_data.SetMember("words", words);

			for (auto& shoutWord : shout->variations) {
				RE::GFxValue word = GetBasicFormInfo(shoutWord.word);
				words.PushBack(word);

				word.SetMember("word", shoutWord.word->fullName.c_str());
				word.SetMember("fullName", shoutWord.word->translation.c_str());
				word.SetMember("recoveryTime", shoutWord.recoveryTime);
			}
		}
	}
}
