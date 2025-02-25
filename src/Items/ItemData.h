#pragma once

#include "ItemDefines.h"

namespace QuickLoot::Items
{
	template <typename TValue>
	struct MandatoryField
	{
		TValue value;

		operator TValue() const { return value; }

		MandatoryField& operator=(TValue newValue)
		{
			value = newValue;
			return *this;
		}
	};

	template <typename TValue>
	struct OptionalField
	{
		TValue value{};
		bool valid = false;

		operator TValue() const { return value; }

		OptionalField& operator=(TValue newValue)
		{
			value = newValue;
			valid = true;
			return *this;
		}

		void unset()
		{
			valid = false;
		}
	};

	template <typename TEnum, typename TUnderlying = std::underlying_type_t<TEnum>>
	struct MandatoryEnumField
	{
		using TWrapper = SKSE::stl::enumeration<TEnum, TUnderlying>;

		TWrapper value;

		operator TEnum() const { return value.get(); }

		MandatoryEnumField& operator=(TWrapper newValue)
		{
			value = newValue;
			return *this;
		}

		MandatoryEnumField& operator=(TEnum newValue)
		{
			value = newValue;
			return *this;
		}

		bool operator==(TEnum other)
		{
			return value.get() == other;
		}

		bool operator!=(TEnum other)
		{
			return value.get() != other;
		}
	};

	template <typename TEnum, typename TUnderlying = std::underlying_type_t<TEnum>>
	struct OptionalEnumField
	{
		using TWrapper = SKSE::stl::enumeration<TEnum, TUnderlying>;

		TWrapper value{};
		bool valid = false;

		operator TEnum() const { return value.get(); }

		OptionalEnumField& operator=(TWrapper newValue)
		{
			value = newValue;
			valid = true;
			return *this;
		}

		OptionalEnumField& operator=(TEnum newValue)
		{
			value = newValue;
			valid = true;
			return *this;
		}

		bool operator==(TEnum other)
		{
			return value.get() == other;
		}

		bool operator!=(TEnum other)
		{
			return value.get() != other;
		}

		void unset()
		{
			valid = false;
		}
	};

	inline void SetDataMember(RE::GFxValue& obj, const char* name, const RE::GFxValue& value)
	{
		obj.SetMember(name, value);
	}

	template <typename T>
	void SetDataMember(RE::GFxValue& obj, const char* name, const MandatoryField<T>& field)
	{
		obj.SetMember(name, field.value);
	}

	template <typename T>
	void SetDataMember(RE::GFxValue& obj, const char* name, const OptionalField<T>& field)
	{
		if (field.valid) {
			obj.SetMember(name, field.value);
		}
	}

	template <typename TEnum, typename TUnderlying>
	void SetDataMember(RE::GFxValue& obj, const char* name, const MandatoryEnumField<TEnum, TUnderlying>& field)
	{
		obj.SetMember(name, field.value.underlying());
	}

	template <typename TEnum, typename TUnderlying>
	void SetDataMember(RE::GFxValue& obj, const char* name, const OptionalEnumField<TEnum, TUnderlying>& field)
	{
		if (field.valid) {
			obj.SetMember(name, field.value.underlying());
		}
	}

	struct MaterialItemData
	{
		OptionalField<MaterialType> material;
		MandatoryField<const char*> materialDisplay;
	};

	struct ArmorItemData : MaterialItemData
	{
		MandatoryEnumField<ArmorSlot> partMask;
		OptionalEnumField<ArmorWeightClass> weightClass;
		MandatoryField<float> armor;
		MandatoryField<float> infoArmor;
		MandatoryField<bool> isEnchanted;
		MandatoryField<const char*> weightClassDisplay;
		MandatoryEnumField<ArmorSlot> mainPartMask;
		OptionalEnumField<ArmorSubType> subType;
	};

	struct WeaponItemData : MaterialItemData
	{
		MandatoryEnumField<WeaponAnimationType, uint8_t> weaponType;
		MandatoryField<float> speed;
		MandatoryField<float> reach;
		MandatoryField<float> stagger;
		MandatoryField<uint16_t> critDamage;
		MandatoryField<float> minRange;
		MandatoryField<float> maxRange;
		MandatoryField<uint16_t> baseDamage;
		OptionalField<RE::FormID> equipSlot;
		MandatoryField<float> damage;
		MandatoryField<float> infoDamage;
		MandatoryField<bool> isPoisoned;
		MandatoryField<bool> isEnchanted;
		OptionalEnumField<WeaponType> subType;
	};

	struct AmmoItemData : MaterialItemData
	{
		MandatoryEnumField<AmmoFlags, uint8_t> flags;
		MandatoryField<float> damage;
		MandatoryField<float> infoDamage;
		MandatoryField<bool> isEnchanted;
		OptionalEnumField<AmmoType> subType;
	};

	struct MagicItemData
	{
		MandatoryField<const char*> spellName;
		MandatoryField<float> magnitude;
		MandatoryField<uint32_t> duration;
		MandatoryField<uint32_t> area;
		OptionalField<const char*> effectName;
		OptionalEnumField<RE::ActorValue> subType;
		OptionalEnumField<EffectFlags, uint32_t> effectFlags;
		OptionalEnumField<RE::ActorValue> school;
		OptionalField<uint32_t> skillLevel;
		OptionalEnumField<RE::EffectArchetype> archetype;
		OptionalEnumField<RE::MagicSystem::Delivery> deliveryType;
		OptionalField<float> castTime;
		OptionalField<float> delayTime;
		OptionalEnumField<RE::ActorValue> actorValue;
		OptionalEnumField<RE::MagicSystem::CastingType> castType;
		OptionalEnumField<RE::ActorValue> resistance;
	};

	struct AlchemyItemData : MagicItemData
	{
		MandatoryEnumField<AlchemyFlags, uint32_t> flags;
		OptionalField<RE::TESForm*> useSound;
		OptionalEnumField<PotionType> subType;
	};

	struct SpellItemData : MagicItemData
	{
		MandatoryEnumField<RE::MagicSystem::SpellType> spellType;
		MandatoryField<int32_t> trueCost;
		OptionalField<RE::FormID> equipSlot;
	};

	struct ScrollItemData : SpellItemData
	{
	};

	struct BookItemData
	{
		MandatoryEnumField<BookFlags, uint8_t> flags;
		MandatoryEnumField<BookType, uint8_t> bookType;
		OptionalField<RE::FormID> teachesSpell;
		OptionalEnumField<RE::ActorValue> teachesSkill;
		MandatoryField<bool> isRead;
		OptionalEnumField<BookSubType> subType;
	};

	struct MiscItemData
	{
		OptionalEnumField<MiscType> subType;
	};

	struct KeyItemData : MiscItemData
	{
	};

	struct SoulGemItemData  // doesn't inherit MiscItemData because subType uses a different enum
	{
		OptionalEnumField<SoulLevel> subType;
		MandatoryEnumField<SoulLevel, uint8_t> gemSize;
		MandatoryEnumField<SoulLevel, uint8_t> soulSize;
		MandatoryEnumField<SoulGemStatus, uint8_t> status;
	};

	struct EnchantmentData
	{
		MandatoryField<uint32_t> flags;
		MandatoryField<RE::TESForm*> baseEnchant;
		MandatoryField<RE::TESForm*> restrictions;
	};

	struct ShoutData
	{
		MandatoryField<const char*> fullName;
		struct ShoutWord
		{
			MandatoryField<const char*> word;
			MandatoryField<const char*> fullName;
			MandatoryField<float> recoveryTime;
		} words[3];
	};

	struct ItemData
	{
		MandatoryField<const char*> text;
		MandatoryField<uint32_t> count;
		MandatoryField<uint32_t> equipState;
		MandatoryField<uint32_t> filterFlag;
		MandatoryField<uint32_t> favorite;
		MandatoryField<bool> enabled;
		MandatoryField<bool> isStealing;
		MandatoryField<SoulLevel> soulLVL;

		MandatoryEnumField<RE::FormType, uint8_t> formType;
		MandatoryField<RE::FormID> formId;
		OptionalField<RE::BGSKeywordForm*> keywords;

		MandatoryField<int32_t> value;
		MandatoryField<float> weight;

		MandatoryField<RE::FormID> baseId;
		MandatoryEnumField<ItemType> type;
		MandatoryField<bool> isEquipped;
		MandatoryField<bool> isStolen;

		OptionalField<float> infoValue;
		OptionalField<float> infoWeight;
		OptionalField<float> infoValueWeight;

		MandatoryField<const char*> subTypeDisplay;
		MandatoryField<const char*> iconLabel;
		OptionalField<uint32_t> iconColor;

		MandatoryField<EnchantmentType> enchantmentType;

		union
		{
			MaterialItemData material;  // common prefix for the following
			ArmorItemData armor;
			WeaponItemData weapon;
			AmmoItemData ammo;

			MagicItemData magic;  // common prefix for the following
			AlchemyItemData potion;
			ScrollItemData scroll;
			SpellItemData spell;

			BookItemData book;

			MiscItemData misc;  // common prefix for the following
			KeyItemData key;
			SoulGemItemData soulGem;

			EnchantmentData enchantment;
			ShoutData shout;
		};
	};
}
