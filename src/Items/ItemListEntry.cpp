#include "ItemListEntry.h"

#include "Integrations/Completionist.h"
#include "Integrations/LOTD.h"

#undef GetModuleHandle

static const char* strIcons[] = {
	"none",  // 00
	"default_weapon",
	"weapon_sword",
	"weapon_greatsword",
	"weapon_daedra",
	"weapon_dagger",
	"weapon_waraxe",
	"weapon_battleaxe",
	"weapon_mace",
	"weapon_hammer",
	"weapon_staff",  // 10
	"weapon_bow",
	"weapon_arrow",
	"weapon_pickaxe",
	"weapon_woodaxe",
	"weapon_crossbow",
	"weapon_bolt",
	"default_armor",
	"lightarmor_body",
	"lightarmor_head",
	"lightarmor_hands",  // 20
	"lightarmor_forearms",
	"lightarmor_feet",
	"lightarmor_calves",
	"lightarmor_shield",
	"lightarmor_mask",
	"armor_body",
	"armor_head",
	"armor_hands",
	"armor_forearms",
	"armor_feet",  // 30
	"armor_calves",
	"armor_shield",
	"armor_mask",
	"armor_bracer",
	"armor_daedra",
	"clothing_body",
	"clothing_robe",
	"clothing_head",
	"clothing_pants",
	"clothing_hands",  // 40
	"clothing_forearms",
	"clothing_feet",
	"clothing_calves",
	"clothing_shoes",
	"clothing_shield",
	"clothing_mask",
	"armor_amulet",
	"armor_ring",
	"armor_circlet",
	"default_scroll",  // 50
	"default_book",
	"default_book_read",
	"book_tome",
	"book_tome_read",
	"book_journal",
	"book_note",
	"book_map",
	"default_food",
	"food_wine",
	"food_beer",  // 60
	"default_ingredient",
	"default_key",
	"key_house",
	"default_potion",
	"potion_health",
	"potion_stam",
	"potion_magic",
	"potion_poison",
	"potion_frost",
	"potion_fire",  // 70
	"potion_shock",
	"default_misc",
	"misc_artifact",
	"misc_clutter",
	"misc_lockpick",
	"misc_soulgem",
	"soulgem_empty",
	"soulgem_partial",
	"soulgem_full",
	"soulgem_grandempty",  // 80
	"soulgem_grandpartial",
	"soulgem_grandfull",
	"soulgem_azura",
	"misc_gem",
	"misc_ore",
	"misc_ingot",
	"misc_hide",
	"misc_strips",
	"misc_leather",
	"misc_wood",  // 90
	"misc_remains",
	"misc_trollskull",
	"misc_torch",
	"misc_goldsack",
	"misc_gold",
	"misc_dragonclaw"
};

using namespace RE;
using namespace QuickLoot::Integrations;

enum ItemKind
{
    kInventory,
	kGround
};

namespace QuickLoot::Items
{
	ItemListEntry::ItemListEntry(std::ptrdiff_t a_count, bool a_stealing, SKSE::stl::observer<RE::InventoryEntryData*> a_item)
		: _src(a_item)
		, _count(a_count)
		, _stealing(a_stealing)
	{
		assert(a_item != nullptr);
	}

	ItemListEntry::ItemListEntry(std::ptrdiff_t a_count, bool a_stealing, std::span<const RE::ObjectRefHandle> a_items)
		: _src(a_items)
		, _count(a_count)
		, _stealing(a_stealing)
	{}

	int ItemListEntry::Compare(const ItemListEntry& a_rhs) const
	{
		const ItemListEntry& a_lhs = *this;

		if (a_lhs.IsQuestItem() != a_rhs.IsQuestItem()) return a_lhs.IsQuestItem() ? -1 : 1;
		if (a_lhs.IsKey() != a_rhs.IsKey()) return a_lhs.IsKey() ? -1 : 1;
        if (a_lhs.IsNote() != a_rhs.IsNote()) return a_lhs.IsNote() ? -1 : 1;
        if (a_lhs.IsBook() != a_rhs.IsBook()) return a_lhs.IsBook() ? -1 : 1;
        if (a_lhs.IsGold() != a_rhs.IsGold()) return a_lhs.IsGold() ? -1 : 1;
        if (a_lhs.IsAmmo() != a_rhs.IsAmmo()) return a_lhs.IsAmmo() ? -1 : 1;
		if (a_lhs.IsLockpick() != a_rhs.IsLockpick()) return a_lhs.IsLockpick() ? -1 : 1;

		const auto name1 = a_lhs.GetDisplayName().c_str();
		const auto name2 = a_rhs.GetDisplayName().c_str();
		const auto order = _stricmp(name1, name2);

		if (order != 0) {
            return order;
        }

        if (a_lhs.GetValue() != a_rhs.GetValue()) {
			return a_lhs.GetValue() > a_rhs.GetValue() ? -1 : 1;
        }

        if (a_lhs.GetFormID() != a_rhs.GetFormID()) {
			return a_lhs.GetFormID() < a_rhs.GetFormID() ? -1 : 1;
        }

        return 0;
    }

	const std::string& ItemListEntry::GetDisplayName() const
	{
		if (_cache.IsCached(kDisplayName)) {
			return _cache.GetDisplayName();
		}

		std::string result;

		if (result.empty()) {
			switch (_src.index()) {
			case kInventory:
				{
					const char* display_name = std::get<kInventory>(_src)->GetDisplayName();
					result = display_name ? display_name : ""sv;
					break;
				}
			case kGround:
				result = ""sv;
				for (const auto& handle : std::get<kGround>(_src)) {
					const auto item = handle.get();
					if (item && item->GetDisplayFullName()) {
						result = item->GetDisplayFullName();
						break;
					}
				}
				break;
			default:
				assert(false);
				break;
			}
		}

		if (Completionist::IsReady()) {
			result = Completionist::DecorateItemDisplayName(GetFormID(), result);
		}

		return _cache.SetDisplayName(std::move(result));
	}

	double ItemListEntry::GetEnchantmentCharge() const
	{
		if (_cache.IsCached(kEnchantmentCharge)) {
			return _cache.GetEnchantmentCharge();
		}

		double result = -1.0;
		switch (_src.index()) {
		case kInventory:
			result =
				std::get<kInventory>(_src)
					->GetEnchantmentCharge()
					.value_or(-1.0);
			break;
		case kGround:
			for (const auto& handle : std::get<kGround>(_src)) {
				const auto item = handle.get();
				if (item) {
					const auto charge = item->GetEnchantmentCharge();
					if (charge) {
						result = charge.value_or(-1.0);
						break;
					}
				}
			}
			break;
		default:
			assert(false);
			break;
		}

		return _cache.SetEnchantmentCharge(result);
	}

	bool ItemListEntry::IsEnchanted() const
	{
		if (!_cache.IsCached(kIsEnchanted)) {
			SetupEnchantmentFlags();
		}
		return _cache.IsEnchanted();
	}

	bool ItemListEntry::IsKnownEnchanted() const
	{
		if (!_cache.IsCached(kIsKnownEnchanted)) {
			SetupEnchantmentFlags();
		}
		return _cache.IsKnownEnchanted();
	}

	bool ItemListEntry::IsSpecialEnchanted() const
	{
		if (!_cache.IsCached(kIsSpecialEnchanted)) {
			SetupEnchantmentFlags();
		}
		return _cache.IsSpecialEnchanted();
	}

	RE::TESForm* ItemListEntry::GetObject() const
	{
		switch (_src.index()) {
		case kInventory:
			return std::get<kInventory>(_src)->GetObject();
		case kGround:
			for (const auto& handle : std::get<kGround>(_src)) {
				const auto item = handle.get();
				const auto obj = item ? item->GetObjectReference() : nullptr;
				if (obj) {
					return obj;
				}
			}
			break;
		default:
			assert(false);
			break;
		}

		return nullptr;
	}

	RE::FormID ItemListEntry::GetFormID() const
	{
		if (_cache.IsCached(kFormID)) {
			return _cache.GetFormID();
		}

		auto result = std::numeric_limits<RE::FormID>::max();
		switch (_src.index()) {
		case kInventory:
			if (const auto obj = std::get<kInventory>(_src)->GetObject(); obj) {
				result = obj->GetFormID();
			}
			break;
		case kGround:
			for (const auto& handle : std::get<kGround>(_src)) {
				const auto item = handle.get();
				const auto obj = item ? item->GetObjectReference() : nullptr;
				if (obj) {
					result = obj->GetFormID();
					break;
				}
			}
			break;
		default:
			assert(false);
			break;
		}

		return _cache.SetFormID(result);
	}

	ItemType ItemListEntry::GetItemType() const
	{
		if (_cache.IsCached(kItemType)) {
			return _cache.GetItemType();
		}

		auto result = ItemType::None;
		switch (_src.index()) {
		case kInventory:
			if (const auto obj = std::get<kInventory>(_src)->GetObject(); obj) {
				result = GetItemType(obj);
			}
			break;
		case kGround:
			for (const auto& handle : std::get<kGround>(_src)) {
				const auto item = handle.get();
				const auto obj = item ? item->GetObjectReference() : nullptr;
				if (obj) {
					result = GetItemType(obj);
					break;
				}
			}
			break;
		default:
			assert(false);
			break;
		}

		return _cache.SetItemType(result);
	}

	std::ptrdiff_t ItemListEntry::GetValue() const
	{
		if (_cache.IsCached(kValue)) {
			return _cache.GetValue();
		}

		auto result = std::numeric_limits<std::ptrdiff_t>::min();
		switch (_src.index()) {
		case kInventory:
			result = std::get<kInventory>(_src)->GetValue() * _count;
			break;
		case kGround:
			for (const auto& handle : std::get<kGround>(_src)) {
				const auto item = handle.get();
				const auto obj = item ? item->GetObjectReference() : nullptr;
				if (obj) {
					result = obj->GetGoldValue() * _count;
					break;
				}
			}
			break;
		default:
			assert(false);
			break;
		}

		return _cache.SetValue(result);
	}

	double ItemListEntry::GetWeight() const
	{
		if (_cache.IsCached(kWeight)) {
			return _cache.GetWeight();
		}

		double result = 0.0;
		switch (_src.index()) {
		case kInventory:
			result = std::get<kInventory>(_src)->GetWeight() * _count;
			break;
		case kGround:
			for (const auto& handle : std::get<kGround>(_src)) {
				const auto item = handle.get();
				const auto obj = item ? item->GetObjectReference() : nullptr;
				if (obj) {
					result = obj->GetWeight() * _count;
					break;
				}
			}
			break;
		default:
			assert(false);
			break;
		}

		return _cache.SetWeight(result);
	}

	RE::SOUL_LEVEL ItemListEntry::GetSoulSize() const
	{
		RE::SOUL_LEVEL result = RE::SOUL_LEVEL::kNone;
		const RE::InventoryEntryData* item_inventory_entry = nullptr;
		const RE::TESForm* item_form = nullptr;
		RE::TESObjectREFRPtr item_refr;

		switch (_src.index()) {
		case kInventory:
		{
			item_inventory_entry = std::get<kInventory>(_src);
			item_form = item_inventory_entry ? item_inventory_entry->GetObject() : nullptr;
			break;
		}
		case kGround:
		{
			for (const auto& handle : std::get<kGround>(_src)) {
				if (!handle.get())
					continue;
				item_refr = handle.get();
				item_form = item_refr.get();
			}
			break;
		}
		default:
			assert(false);
		}

		if (item_inventory_entry) {
			result = item_inventory_entry->GetSoulLevel();
		} else {
			auto extraSoul = item_refr ? item_refr->extraList.GetByType<RE::ExtraSoul>() : nullptr;
			if (extraSoul) {
				result = extraSoul->GetContainedSoul();
			} else {
				auto soulGem = item_form ? item_form->As<RE::TESSoulGem>() : nullptr;
				if (soulGem) {
					result = soulGem->GetContainedSoul();
				}
			}
		}

		return result;
	}

	bool ItemListEntry::IsAmmo() const
	{
		if (_cache.IsCached(kAmmo)) {
			return _cache.IsAmmo();
		}

		bool result = false;
		switch (_src.index()) {
		case kInventory:
			if (const auto obj = std::get<kInventory>(_src)->GetObject(); obj) {
				result = obj->IsAmmo();
			}
			break;
		case kGround:
			for (const auto& handle : std::get<kGround>(_src)) {
				const auto item = handle.get();
				const auto obj = item ? item->GetObjectReference() : nullptr;
				if (obj) {
					result = obj->IsAmmo();
					break;
				}
			}
			break;
		default:
			assert(false);
			break;
		}

		return _cache.SetAmmo(result);
	}

	bool ItemListEntry::IsBook() const
	{
		if (_cache.IsCached(kBook)) {
			return _cache.IsBook();
		}

		bool result = false;
		switch (_src.index()) {
		case kInventory:
			if (const auto obj = std::get<kInventory>(_src)->GetObject(); obj) {
				result = obj->IsBook();
			}
			break;
		case kGround:
			for (const auto& handle : std::get<kGround>(_src)) {
				const auto item = handle.get();
				const auto obj = item ? item->GetObjectReference() : nullptr;
				if (obj) {
					result = obj->IsBook();
					break;
				}
			}
			break;
		default:
			assert(false);
			break;
		}

		return _cache.SetBook(result);
	}

	bool ItemListEntry::IsRead() const
	{
		if (!IsBook()) {
			return false;
		}

		if (_cache.IsCached(kIsRead)) {
			return _cache.IsRead();
		}

		bool result = false;
		switch (_src.index()) {
		case kInventory:
			if (const auto obj = static_cast<RE::TESObjectBOOK*>(std::get<kInventory>(_src)->GetObject()); obj) {
				result = obj->IsRead();
			}
			break;
		case kGround:
			for (const auto& handle : std::get<kGround>(_src)) {
				const auto item = handle.get();
				const auto obj = item ? static_cast<RE::TESObjectBOOK*>(item->GetObjectReference()) : nullptr;
				if (obj) {
					result = obj->IsRead();
					break;
				}
			}
			break;
		default:
			assert(false);
			break;
		}

		return _cache.SetRead(result);
	}

	bool ItemListEntry::IsGold() const
	{
		if (_cache.IsCached(kGold)) {
			return _cache.IsGold();
		}

		bool result = false;
		switch (_src.index()) {
		case kInventory:
			if (const auto obj = std::get<kInventory>(_src)->GetObject(); obj) {
				result = obj->IsGold();
			}
			break;
		case kGround:
			for (const auto& handle : std::get<kGround>(_src)) {
				const auto item = handle.get();
				const auto obj = item ? item->GetObjectReference() : nullptr;
				if (obj) {
					result = obj->IsGold();
					break;
				}
			}
			break;
		default:
			assert(false);
			break;
		}

		return _cache.SetGold(result);
	}

	bool ItemListEntry::IsKey() const
	{
		if (_cache.IsCached(kKey)) {
			return _cache.IsKey();
		}

		bool result = false;
		switch (_src.index()) {
		case kInventory:
			if (const auto obj = std::get<kInventory>(_src)->GetObject(); obj) {
				result = obj->IsKey();
			}
			break;
		case kGround:
			for (const auto& handle : std::get<kGround>(_src)) {
				const auto item = handle.get();
				const auto obj = item ? item->GetObjectReference() : nullptr;
				if (obj) {
					result = obj->IsKey();
					break;
				}
			}
			break;
		default:
			assert(false);
			break;
		}

		return _cache.SetKey(result);
	}

	bool ItemListEntry::IsLockpick() const
	{
		if (_cache.IsCached(kLockpick)) {
			return _cache.IsLockpick();
		}

		bool result = false;
		switch (_src.index()) {
		case kInventory:
			if (const auto obj = std::get<kInventory>(_src)->GetObject(); obj) {
				result = obj->IsLockpick();
			}
			break;
		case kGround:
			for (const auto& handle : std::get<kGround>(_src)) {
				const auto item = handle.get();
				const auto obj = item ? item->GetObjectReference() : nullptr;
				if (obj) {
					result = obj->IsLockpick();
					break;
				}
			}
			break;
		default:
			assert(false);
			break;
		}

		return _cache.SetLockpick(result);
	}

	bool ItemListEntry::IsNote() const
	{
	    if (_cache.IsCached(kNote)) {
	        return _cache.IsNote();
	    }

	    auto isNoteBook = [](const RE::TESObjectBOOK* book) {
	        if (!book) return false;
	        auto stem = std::filesystem::path(book->model.c_str()).stem().string();
	        auto range = std::ranges::search(stem, "Note"sv, [](char ch1, char ch2) {
	            return std::toupper(ch1) == std::toupper(ch2);
	        });
	        return !range.empty();
	    };

	    auto lookupBook = [](RE::TESForm* obj) -> RE::TESObjectBOOK* {
	        if (!obj) return nullptr;
	        RE::FormID objFormID = obj->GetFormID();
	        if (!objFormID) return nullptr;
	        return RE::TESForm::LookupByID<RE::TESObjectBOOK>(objFormID);
	    };

	    bool result = false;

	    switch (_src.index()) {
	    case kInventory: {
	        if (const auto obj = std::get<kInventory>(_src)->GetObject(); obj) {
	            if (auto book = lookupBook(obj); isNoteBook(book)) {
	                result = true;
	            }
	        }
	        break;
	    }
	    case kGround: {
	        for (const auto& handle : std::get<kGround>(_src)) {
	            if (const auto item = handle.get(); item) {
	                if (const auto obj = item->GetObjectReference(); obj) {
	                    if (auto book = lookupBook(obj); isNoteBook(book)) {
	                        result = true;
	                        break;
	                    }
	                }
	            }
	        }
	        break;
	    }
	    default:
	        assert(false);
	        break;
	    }

	    return _cache.SetNote(result);
	}


	bool ItemListEntry::IsQuestItem() const
	{
		if (_cache.IsCached(kIsQuestItem)) {
			return _cache.IsQuestItem();
		}

		bool result = false;
		switch (_src.index()) {
		case kInventory:
			result = std::get<kInventory>(_src)->IsQuestObject();
			break;
		case kGround:
			for (const auto& handle : std::get<kGround>(_src)) {
				const auto item = handle.get();
				if (item && item->HasQuestObject()) {
					result = true;
					break;
				}
			}
			break;
		default:
			assert(false);
			break;
		}

		return _cache.SetQuestItem(result);
	}

	bool ItemListEntry::IsStolen() const
	{
		if (_cache.IsCached(kIsStolen)) {
			return _cache.IsStolen();
		}

		bool result = false;
		auto player = RE::PlayerCharacter::GetSingleton();
		if (player) {
			switch (_src.index()) {
			case kInventory:
				result = !std::get<kInventory>(_src)->IsOwnedBy(player, !_stealing);
				break;
			case kGround:
				for (const auto& handle : std::get<kGround>(_src)) {
					const auto item = handle.get();
					if (item && item->IsCrimeToActivate()) {
						result = true;
						break;
					}
				}
				break;
			default:
				assert(false);
				break;
			}
		}

		return _cache.SetStolen(result);
	}

	bool ItemListEntry::ItemIsDbmNew() const
	{
		if (_cache.IsCached(kIsDbmNew)) {
			return _cache.IsDbmNew();
		}

		return _cache.SetDbmNew(LOTD::IsItemNew(GetFormID()));
	}

	bool ItemListEntry::ItemIsDbmFound() const
	{
		if (_cache.IsCached(kIsDbmFound)) {
			return _cache.IsDbmFound();
		}

		return _cache.SetDbmFound(LOTD::IsItemFound(GetFormID()));
	}

	bool ItemListEntry::ItemIsDbmDisplayed() const
	{
		if (_cache.IsCached(kIsDbmDisplayed)) {
			return _cache.IsDbmDisplayed();
		}

		return _cache.SetDbmDisplayed(LOTD::IsItemDisplayed(GetFormID()));
	}

	bool ItemListEntry::ItemIsCompNew() const
	{
		if (_cache.IsCached(kIsCompNew)) {
			return _cache.IsCompNew();
		}

		return _cache.SetCompNew(Completionist::IsItemNeeded(GetFormID()));
	}

	bool ItemListEntry::ItemIsCompFound() const
	{
		if (_cache.IsCached(kIsCompFound)) {
			return _cache.IsCompFound();
		}

		return _cache.SetCompFound(Completionist::IsItemCollected(GetFormID()));
	}
	
	static RE::GFxValue GetKeywords(RE::GFxMovieView& view, TESForm* form)
	{
		GFxValue keywords;
		view.CreateObject(&keywords);

		const auto keywordForm = form->As<BGSKeywordForm>();
		if (!keywordForm) {
			return "";
		}

		for (uint32_t k = 0; k < keywordForm->GetNumKeywords(); k++) {
			const auto keyword = keywordForm->GetKeywordAt(k).value_or(nullptr);
			if (!keyword)
				continue;

			const auto editorId = keyword->GetFormEditorID();
			if (!editorId || !editorId[0])
				continue;

			keywords.SetMember(editorId, true);
		}

		return keywords;
	}

	RE::GFxValue ItemListEntry::GFxValue(RE::GFxMovieView& a_view) const
	{
		RE::GFxValue value;
		a_view.CreateObject(std::addressof(value));
		auto obj = GetObject();
		auto formType = obj ? obj->GetFormType() : RE::FormType::None;
		value.SetMember("formType", formType);
		value.SetMember("formId", obj ? obj->GetFormID() : 0);
		value.SetMember("keywords", GetKeywords(a_view, obj));
		value.SetMember("isEnchanted", IsEnchanted());
		

		switch (formType) {
		case RE::FormType::Armor:
		{
			// Not implemented yet (with examples):
			// parts | [32, 51]
			// mainPart | 4
			// partMask | 5
		    RE::TESObjectARMO* armor = skyrim_cast<RE::TESObjectARMO*>(obj);
		    if (!armor) break;

		    value.SetMember("partMask", armor->bipedModelData.bipedObjectSlots.underlying());
		    value.SetMember("weightClass", armor->bipedModelData.armorType.underlying());
		    value.SetMember("armor", armor->armorRating);
		    RE::BGSEquipSlot* equipSlot = armor->equipSlot;
		    if (equipSlot) {
		        value.SetMember("equipSlot", equipSlot->formID);
		    }

		    int subType = -1;
		    std::unordered_map<BGSBipedObjectForm::BipedObjectSlot, int> partMap = {
		        {BGSBipedObjectForm::BipedObjectSlot::kHead, 0},		// EquipType::Head
		        {BGSBipedObjectForm::BipedObjectSlot::kHair, 1},		// EquipType::Hair
		        {BGSBipedObjectForm::BipedObjectSlot::kLongHair, 2},	// EquipType::LongHair
		        {BGSBipedObjectForm::BipedObjectSlot::kBody, 3},		// EquipType::Body
		        {BGSBipedObjectForm::BipedObjectSlot::kForearms, 4},	// EquipType::Forearms
		        {BGSBipedObjectForm::BipedObjectSlot::kHands, 5},		// EquipType::Hands
		        {BGSBipedObjectForm::BipedObjectSlot::kShield, 6},		// EquipType::Shield
		        {BGSBipedObjectForm::BipedObjectSlot::kCalves, 7},		// EquipType::Calves
		        {BGSBipedObjectForm::BipedObjectSlot::kFeet, 8},		// EquipType::Feet
		        {BGSBipedObjectForm::BipedObjectSlot::kCirclet, 9},		// EquipType::Circlet
		        {BGSBipedObjectForm::BipedObjectSlot::kAmulet, 10},		// EquipType::Amulet
		        {BGSBipedObjectForm::BipedObjectSlot::kEars, 11},		// EquipType::Ears
		        {BGSBipedObjectForm::BipedObjectSlot::kRing, 12},		// EquipType::Ring
		        {BGSBipedObjectForm::BipedObjectSlot::kTail, 13}		// EquipType::Tail
		    };

		    for (const auto& pair : partMap) {
		        if (armor->HasPartOf(pair.first)) {
		            subType = pair.second;
		            break;
		        }
		    }

		    value.SetMember("subType", subType);
		    break;
		}

		case RE::FormType::Ammo:
		{
			RE::TESAmmo* ammo = skyrim_cast<RE::TESAmmo*>(obj);
			if (!ammo) break;

			value.SetMember("flags", ammo->data.flags.underlying());
			value.SetMember("damage", ammo->data.damage);
			value.SetMember("subType", ammo->IsBolt() ? 1 : 0);

			break;
		}
		case RE::FormType::Weapon:
		{
		    // TODO: isPoisoned | bool

		    RE::TESObjectWEAP* weapon = skyrim_cast<RE::TESObjectWEAP*>(obj);
			if (!weapon) break;

	        auto& weaponData = weapon->weaponData;
	        auto& criticalData = weapon->criticalData;

	        value.SetMember("weaponType", weaponData.animationType.underlying());
	        value.SetMember("subType", weaponData.animationType.underlying());
	        value.SetMember("speed", weaponData.speed);
	        value.SetMember("reach", weaponData.reach);
	        value.SetMember("stagger", weaponData.staggerValue);
	        value.SetMember("critDamage", criticalData.damage);
	        value.SetMember("minRange", weaponData.minRange);
	        value.SetMember("maxRange", weaponData.maxRange);
	        value.SetMember("baseDamage", weapon->attackDamage);

	        RE::BGSEquipSlot* equipSlot = weapon->equipSlot;
	        if (equipSlot) {
	            value.SetMember("equipSlot", equipSlot->formID);
	        }

	        int subType = -1;  // Default unknown weapon type
			const bool isWarhammer = weapon->HasKeywordID(0x0006d930);

	        if (weapon->IsOneHandedSword()) {
	            subType = 1;
	        } else if (weapon->IsOneHandedDagger()) {
	            subType = 2;
	        } else if (weapon->IsOneHandedAxe()) {
	            subType = 3;
	        } else if (weapon->IsOneHandedMace()) {
	            subType = 4;
	        } else if (weapon->IsTwoHandedSword()) {
	            subType = 5;
	        } else if (weapon->IsTwoHandedAxe() && !isWarhammer) {
	            subType = 6;
	        } else if (weapon->IsTwoHandedAxe() && isWarhammer) {
	            subType = 7;
	        } else if (weapon->IsBow()) {
	            subType = 8;
	        } else if (weapon->IsCrossbow()) {
	            subType = 9;
	        } else if (weapon->IsStaff()) {
	            subType = 10;
	        }

	        value.SetMember("subType", subType);

			break;
		}

		case RE::FormType::SoulGem: 
		{
			auto* soulGem = skyrim_cast<RE::TESSoulGem*>(obj);
			if (!soulGem) break;

			const static UINT32 DA01SoulGemAzurasStar = 0x063B27;
			const static UINT32 DA01SoulGemBlackStar = 0x063B29;

			RE::SOUL_LEVEL currentSoul = GetSoulSize();
			int subType = (soulGem->formID == DA01SoulGemBlackStar || soulGem->formID == DA01SoulGemAzurasStar) ? 6 : soulGem->soulCapacity.underlying();
			int status = (currentSoul == RE::SOUL_LEVEL::kNone) ? 0 : (currentSoul >= soulGem->soulCapacity) ? 2 : 1;

			value.SetMember("subType", subType);
			value.SetMember("gemSize", soulGem->soulCapacity.underlying());
			value.SetMember("soulSize", currentSoul);
			value.SetMember("status", status);

			break;
		}
		case RE::FormType::Book:
		{
			RE::TESObjectBOOK* book = skyrim_cast<RE::TESObjectBOOK*>(obj);
			if (!book) break;

			const bool isNote = IsNote();
			const bool hasRecipeKeyword = book->HasKeywordID(0x000F5CB0);

			if (isNote) {
				value.SetMember("bookType", 255);
				value.SetMember("subType", hasRecipeKeyword ? 2 : 1);
			} else {
				value.SetMember("bookType", 0);
				value.SetMember("subType", 0);
			}

			value.SetMember("flags", book->data.flags.underlying());

			if (book->data.flags.all(RE::OBJ_BOOK::Flag::kAdvancesActorValue)) {
				value.SetMember("teachesSkill", book->data.teaches.actorValueToAdvance);
			} else if (book->data.flags.all(RE::OBJ_BOOK::Flag::kTeachesSpell)) {
				double formID = -1;
				if (auto spell = book->data.teaches.spell; spell) {
					formID = spell->GetFormID();
				}
				value.SetMember("teachesSpell", formID);
			}
			break;
		}
		case RE::FormType::Scroll:
		{
			RE::ScrollItem* scroll = skyrim_cast<RE::ScrollItem*>(obj);
			if (!scroll) break;

			value.SetMember("flags", scroll->formFlags);
			value.SetMember("school", scroll->GetAVEffect()->data.associatedSkill);
		
			break;
		}
		case RE::FormType::AlchemyItem:
		{
		    RE::AlchemyItem* alchemy = skyrim_cast<RE::AlchemyItem*>(obj);
		    if (!alchemy) break;

			// subType 13 (drink) is somehow assigned properly without us
			// doing anything, so we just check the others here.

			RE::ActorValue potAV = alchemy->GetAVEffect()->data.primaryAV;

			int subType = 12;	// Default subType is Potion
		    static const std::unordered_map<ActorValue, int> avSubtypes = {
		        {ActorValue::kHealth, 0},
		        {ActorValue::kHealRate, 1},
		        {ActorValue::kHealRateMult, 2},
		        {ActorValue::kMagicka, 3},
		        {ActorValue::kMagickaRate, 4},
		        {ActorValue::kMagickaRateMult, 5},
		        {ActorValue::kStamina, 6},
		        {ActorValue::kStaminaRate, 7},
		        {ActorValue::kStaminaRateMult, 8},
		        {ActorValue::kResistFire, 9},
		        {ActorValue::kResistShock, 10},
		        {ActorValue::kResistFrost, 11}
		    };

			for (const auto& skill : avSubtypes) {
				if (alchemy->IsFood()) {
					subType = 14;
				} else if (alchemy->IsPoison()) {
					subType = 15;
				} else if (potAV == skill.first) {
					subType = skill.second;
					break;
				}
			}

			value.SetMember("subType", subType);
			value.SetMember("flags", alchemy->data.flags.underlying());
		    break;
		}
		}

		value.SetMember("displayName", static_cast<std::string_view>(GetDisplayName()));
		value.SetMember("count", _count);
		value.SetMember("value", GetValue());
		value.SetMember("weight", std::max(GetWeight(), 0.0));
		value.SetMember("iconLabel", GetItemIconLabel(GetItemType()));

		value.SetMember("stolen", IsStolen());
	    value.SetMember("read", Settings::ShowBookRead() && IsRead());

		if (Settings::ShowEnchanted()) {
			value.SetMember("enchanted", IsEnchanted());
			value.SetMember("knownEnchanted", IsKnownEnchanted());
			value.SetMember("specialEnchanted", IsSpecialEnchanted());
		}

		if (LOTD::IsReady()) {
			value.SetMember("dbmNew", Settings::ShowDBMNew() && ItemIsDbmNew());
			value.SetMember("dbmFound", Settings::ShowDBMFound() && ItemIsDbmFound());
			value.SetMember("dbmDisplayed", Settings::ShowDBMDisplayed() && ItemIsDbmDisplayed());
		}

		if (Completionist::IsIntegrationEnabled()) {
			value.SetMember("compNew", Settings::ShowCompNeeded() && ItemIsCompNew());
			value.SetMember("compFound", Settings::ShowCompCollected() && ItemIsCompFound());

			if (const auto colorInt = Completionist::GetItemDynamicTextColor(GetFormID()); colorInt != -1)
				value.SetMember("textColor", colorInt);
		}

		return value;
	}

	static ItemType GetItemTypeWeapon(TESObjectWEAP* weap)
	{
		ItemType type = ItemType::DefaultWeapon;

		switch (weap->GetWeaponType()) {
		case RE::WEAPON_TYPE::kOneHandSword:
			type = ItemType::WeaponSword;
			break;
		case RE::WEAPON_TYPE::kOneHandDagger:
			type = ItemType::WeaponDagger;
			break;
		case RE::WEAPON_TYPE::kOneHandAxe:
			type = ItemType::WeaponWarAxe;
			break;
		case RE::WEAPON_TYPE::kOneHandMace:
			type = ItemType::WeaponMace;
			break;
		case RE::WEAPON_TYPE::kTwoHandSword:
			type = ItemType::WeaponGreatSword;
			break;
		case RE::WEAPON_TYPE::kTwoHandAxe:
			type = ItemType::WeaponBattleAxe;
			break;
		case RE::WEAPON_TYPE::kBow:
			type = ItemType::WeaponBow;
			break;
		case RE::WEAPON_TYPE::kStaff:
			type = ItemType::WeaponStaff;
			break;
		case RE::WEAPON_TYPE::kCrossbow:
			type = ItemType::WeaponCrossbow;
			break;
		}

		return type;
	}

	static ItemType GetItemTypeArmor(TESObjectARMO* armor)
	{
		static ItemType types[] = {
			ItemType::LightArmorBody,  // 0
			ItemType::LightArmorHead,
			ItemType::LightArmorHands,
			ItemType::LightArmorForearms,
			ItemType::LightArmorFeet,
			ItemType::LightArmorCalves,
			ItemType::LightArmorShield,
			ItemType::LightArmorMask,

			ItemType::ArmorBody,  // 8
			ItemType::ArmorHead,
			ItemType::ArmorHands,
			ItemType::ArmorForearms,
			ItemType::ArmorFeet,
			ItemType::ArmorCalves,
			ItemType::ArmorShield,
			ItemType::ArmorMask,

			ItemType::ClothingBody,  // 16
			ItemType::ClothingHead,
			ItemType::ClothingHands,
			ItemType::ClothingForearms,
			ItemType::ClothingFeet,
			ItemType::ClothingCalves,
			ItemType::ClothingShield,
			ItemType::ClothingMask,

			ItemType::ArmorAmulet,  // 24
			ItemType::ArmorRing,
			ItemType::Circlet,

			ItemType::DefaultArmor  // 27
		};

		UINT32 index = 0;

		if (armor->IsLightArmor()) {
			index = 0;
		} else if (armor->IsHeavyArmor()) {
			index = 8;
		} else {
			if (armor->HasKeywordID(0x08F95B)) {  // VendorItemClothing
				index = 16;
			} else if (armor->HasKeywordID(0x08F95A)) {  // VendorItemJewelry
				if (armor->HasPartOf(BGSBipedObjectForm::BipedObjectSlot::kAmulet))
					index = 24;
				else if (armor->HasPartOf(BGSBipedObjectForm::BipedObjectSlot::kRing))
					index = 25;
				else if (armor->HasPartOf(BGSBipedObjectForm::BipedObjectSlot::kCirclet))
					index = 26;
				else
					index = 27;
			} else {
				index = 27;
			}
		}

		if (index >= 24)
			return types[index];

		if (armor->HasPartOf(BGSBipedObjectForm::BipedObjectSlot::kBody) || armor->HasPartOf(BGSBipedObjectForm::BipedObjectSlot::kTail))
			index += 0;  // body
		else if (armor->HasPartOf(BGSBipedObjectForm::BipedObjectSlot::kHead) || armor->HasPartOf(BGSBipedObjectForm::BipedObjectSlot::kHair) || armor->HasPartOf(BGSBipedObjectForm::BipedObjectSlot::kLongHair)) {
			index += 1;  // head
			if (armor->formID >= 0x061C8B && armor->formID < 0x061CD7)
				index += 6;  // mask
		} else if (armor->HasPartOf(BGSBipedObjectForm::BipedObjectSlot::kHands))
			index += 2;  // hands
		else if (armor->HasPartOf(BGSBipedObjectForm::BipedObjectSlot::kForearms))
			index += 3;  // forarms
		else if (armor->HasPartOf(BGSBipedObjectForm::BipedObjectSlot::kFeet))
			index += 4;  // forarms
		else if (armor->HasPartOf(BGSBipedObjectForm::BipedObjectSlot::kCalves))
			index += 5;  // calves
		else if (armor->HasPartOf(BGSBipedObjectForm::BipedObjectSlot::kShield))
			index += 6;  // shield
		else
			index = 27;

		return types[index];
	}

	static ItemType GetItemTypePotion(AlchemyItem* potion)
	{
		ItemType type = ItemType::DefaultPotion;

		if (potion->IsFood()) {
			type = ItemType::DefaultFood;

			const static UINT32 ITMPosionUse = 0x000B6435;
			if (potion->data.consumptionSound && potion->data.consumptionSound->formID == ITMPosionUse)
				type = ItemType::FoodWine;
		} else if (potion->IsPoison()) {
			type = ItemType::PotionPoison;
		} else {
			type = ItemType::DefaultPotion;

			//MagicItem::EffectItem* pEffect = CALL_MEMBER_FN(potion, GetCostliestEffectItem)(5, false);
			Effect* pEffect = potion->GetCostliestEffectItem(RE::MagicSystem::Delivery::kTotal, false);
			if (pEffect && pEffect->baseEffect) {
				ActorValue primaryValue = pEffect->baseEffect->GetMagickSkill();

				if (primaryValue == ActorValue::kNone) {
					primaryValue = pEffect->baseEffect->data.primaryAV;
				}

				switch (primaryValue) {
				case ActorValue::kHealth:
					type = ItemType::PotionHealth;
					break;
				case ActorValue::kMagicka:
					type = ItemType::PotionMagic;
					break;
				case ActorValue::kStamina:
					type = ItemType::PotionStam;
					break;
				case ActorValue::kResistFire:
					type = ItemType::PotionFire;
					break;
				case ActorValue::kResistShock:
					type = ItemType::PotionShock;
					break;
				case ActorValue::kResistFrost:
					type = ItemType::PotionFrost;
					break;
				}
			}
		}

		return type;
	}

	static ItemType GetItemTypeMisc(TESObjectMISC* misc)
	{
		ItemType type = ItemType::DefaultMisc;

		static const UINT32 LockPick = 0x00000A;
		static const UINT32 Gold = 0x00000F;
		static const UINT32 Leather01 = 0x000DB5D2;
		static const UINT32 LeatherStrips = 0x000800E4;

		static const UINT32 VendorItemAnimalHideFormId = 0x0914EA;
		static const UINT32 VendorItemDaedricArtifactFormId = 0x000917E8;
		static const UINT32 VendorItemGemFormId = 0x000914ED;
		static const UINT32 VendorItemToolFormId = 0x000914EE;
		static const UINT32 VendorItemAnimalPartFormId = 0x000914EB;
		static const UINT32 VendorItemOreIngotFormId = 0x000914EC;
		static const UINT32 VendorItemClutterFormId = 0x000914E9;
		static const UINT32 VendorItemFirewoodFormId = 0x000BECD7;

		static const UINT32 RubyDragonClaw = 0x04B56C;
		static const UINT32 IvoryDragonClaw = 0x0AB7BB;
		static const UINT32 GlassCraw = 0x07C260;
		static const UINT32 EbonyCraw = 0x05AF48;
		static const UINT32 EmeraldDragonClaw = 0x0ED417;
		static const UINT32 DiamondClaw = 0x0AB375;
		static const UINT32 IronClaw = 0x08CDFA;
		static const UINT32 CoralDragonClaw = 0x0B634C;
		static const UINT32 E3GoldenClaw = 0x0999E7;
		static const UINT32 SapphireDragonClaw = 0x0663D7;
		static const UINT32 MS13GoldenClaw = 0x039647;

		if (misc->formID == LockPick)
			type = ItemType::MiscLockPick;
		else if (misc->formID == Gold)
			type = ItemType::MiscGold;
		else if (misc->formID == Leather01)
			type = ItemType::MiscLeather;
		else if (misc->formID == LeatherStrips)
			type = ItemType::MiscStrips;
		else if (misc->HasKeywordID(VendorItemAnimalHideFormId))
			type = ItemType::MiscHide;
		else if (misc->HasKeywordID(VendorItemDaedricArtifactFormId))
			type = ItemType::MiscArtifact;
		else if (misc->HasKeywordID(VendorItemGemFormId))
			type = ItemType::MiscGem;
		else if (misc->HasKeywordID(VendorItemAnimalPartFormId))
			type = ItemType::MiscRemains;
		else if (misc->HasKeywordID(VendorItemOreIngotFormId))
			type = ItemType::MiscIngot;
		else if (misc->HasKeywordID(VendorItemClutterFormId))
			type = ItemType::MiscClutter;
		else if (misc->HasKeywordID(VendorItemFirewoodFormId))
			type = ItemType::MiscWood;
		else if (misc->formID == RubyDragonClaw || misc->formID == IvoryDragonClaw || misc->formID == GlassCraw || misc->formID == EbonyCraw || misc->formID == EmeraldDragonClaw || misc->formID == DiamondClaw || misc->formID == IronClaw || misc->formID == CoralDragonClaw || misc->formID == E3GoldenClaw || misc->formID == SapphireDragonClaw || misc->formID == MS13GoldenClaw)
			type = ItemType::MiscDragonClaw;

		return type;
	}

	static ItemType GetItemTypeSoulGem(TESSoulGem* gem)
	{
		ItemType type = ItemType::MiscSoulGem;

		const static UINT32 DA01SoulGemAzurasStar = 0x063B27;
		const static UINT32 DA01SoulGemBlackStar = 0x063B29;

		if (gem->formID == DA01SoulGemBlackStar || gem->formID == DA01SoulGemAzurasStar) {
			type = ItemType::SoulGemAzura;
		} else {
			if (gem->GetMaximumCapacity() < SOUL_LEVEL::kGrand) {
				if (gem->GetContainedSoul() == SOUL_LEVEL::kNone)
					type = ItemType::SoulGemEmpty;
				else if (gem->GetContainedSoul() >= gem->GetMaximumCapacity())
					type = ItemType::SoulGemFull;
				else
					type = ItemType::SoulGemPartial;
			} else {
				if (gem->GetContainedSoul() == SOUL_LEVEL::kNone)
					type = ItemType::SoulGemGrandEmpty;
				else if (gem->GetContainedSoul() >= gem->GetMaximumCapacity())
					type = ItemType::SoulGemGrandFull;
				else
					type = ItemType::SoulGemGrandPartial;
			}
		}

		return type;
	}

	const ItemType GetItemTypeBook(TESObjectBOOK* book)
	{
		ItemType type = ItemType::DefaultBook;

		const static UINT32 VendorItemRecipeFormID = 0x000F5CB0;
		const static UINT32 VendorItemSpellTomeFormID = 0x000937A5;

		if (book->data.type.underlying() == 0xFF || book->HasKeywordID(VendorItemRecipeFormID)) {
			type = ItemType::BookNote;
		} else if (book->HasKeywordID(VendorItemSpellTomeFormID)) {
			type = ItemType::BookTome;
		}

		return type;
	}
	ItemType ItemListEntry::GetItemType(TESForm* form) const
	{
		ItemType type = ItemType::None;

		switch (form->formType.get()) {
		case FormType::Scroll:
			type = ItemType::DefaultScroll;
			break;
		case FormType::Armor:
			type = GetItemTypeArmor(static_cast<TESObjectARMO*>(form));
			break;
		case FormType::Book:
			type = GetItemTypeBook(static_cast<TESObjectBOOK*>(form));
			break;
		case FormType::Ingredient:
			type = ItemType::DefaultIngredient;
			break;
		case FormType::Light:
			type = ItemType::MiscTorch;
			break;
		case FormType::Misc:
			type = GetItemTypeMisc(static_cast<TESObjectMISC*>(form));
			break;
		case FormType::Weapon:
			type = GetItemTypeWeapon(static_cast<TESObjectWEAP*>(form));
			break;
		case FormType::Ammo:
			type = (static_cast<TESAmmo*>(form)->IsBolt()) ? ItemType::WeaponBolt : ItemType::WeaponArrow;
			break;
		case FormType::KeyMaster:
			type = ItemType::DefaultKey;
			break;
		case FormType::AlchemyItem:
			type = GetItemTypePotion(static_cast<AlchemyItem*>(form));
			break;
		case FormType::SoulGem:
			type = GetItemTypeSoulGem(static_cast<TESSoulGem*>(form));
			break;
		}

		return type;
	}

	// Straight up copied from MoreHudSE. https://github.com/ahzaab/moreHUDSE
	static bool MagicDisallowEnchanting(const RE::BGSKeywordForm* pKeywords)
	{
		if (pKeywords) {
			for (uint32_t k = 0; k < pKeywords->numKeywords; k++) {
				if (pKeywords->keywords[k]) {
					auto keyword = pKeywords->GetKeywordAt(k).value_or(nullptr);
					if (keyword) {
						// Had to add this check because https://www.nexusmods.com/skyrimspecialedition/mods/34175?
						// sets the editor ID for 'MagicDisallowEnchanting' to null (╯°□°）╯︵ ┻━┻
						auto asCstr = keyword->GetFormEditorID();
						std::string keyWordName = asCstr ? asCstr : "";
						if (keyWordName == "MagicDisallowEnchanting") {
							return true;  // Is enchanted, but cannot be enchanted by player
						}
					}
				}
			}
		}
		return false;
	}

	EnchantmentType ItemListEntry::GetEnchantmentType() const
	{
		EnchantmentType result = EnchantmentType::None;
		const RE::InventoryEntryData* item_inventory_entry = nullptr;
		const RE::TESForm* item_form = nullptr;
		RE::TESObjectREFR* item_refr = nullptr;

		switch (_src.index()) {
		case kInventory:
		{
			item_inventory_entry = std::get<kInventory>(_src);
			item_form = item_inventory_entry ? item_inventory_entry->GetObject() : nullptr;
			break;
		}
		case kGround:
		{
			for (const auto& handle : std::get<kGround>(_src)) {
				if (!handle.get())
					continue;
				item_refr = handle.get().get();
				item_form = item_refr;
			}
			break;
		}
		default:
			assert(false);
		};

		if (!item_form) 
		{
			return result;
		}

		const auto item_form_type = item_form->GetFormType();

		if (item_form_type != RE::FormType::Weapon 
			&& item_form_type != RE::FormType::Armor 
			&& item_form_type != RE::FormType::Ammo 
			&& item_form_type != RE::FormType::Projectile)
		{
			return result;
		}

		RE::EnchantmentItem* enchantment = nullptr;
		auto keyWordForm = item_form->As<RE::BGSKeywordForm>();
		auto enchantable = item_form->As<RE::TESEnchantableForm>();

		bool wasExtra = false;
		if (enchantable) {  // Check the item for a base enchantment
			enchantment = enchantable->formEnchanting;
		}

		if (item_refr) {
			if (auto extraEnchant = static_cast<RE::ExtraEnchantment*>(item_refr->extraList.GetByType(RE::ExtraDataType::kEnchantment))) {
				wasExtra = true;
				enchantment = extraEnchant->enchantment;
			}
		}

		if (enchantment) {
			// It has an enchantment be it may not be known.
			result = EnchantmentType::Unknown;

			if ((enchantment->formFlags & RE::TESForm::RecordFlags::kKnown) == RE::TESForm::RecordFlags::kKnown) {
				return MagicDisallowEnchanting(enchantment) ? EnchantmentType::CannotDisenchant : EnchantmentType::Known;
			} 
			
			if (MagicDisallowEnchanting(enchantment)) {
				return EnchantmentType::CannotDisenchant;
			} 
			
			auto baseEnchantment = static_cast<RE::EnchantmentItem*>(enchantment->data.baseEnchantment);
			if (baseEnchantment) {
				if ((baseEnchantment->formFlags & RE::TESForm::RecordFlags::kKnown) == RE::TESForm::RecordFlags::kKnown) {
					return MagicDisallowEnchanting(baseEnchantment) ? EnchantmentType::CannotDisenchant : EnchantmentType::Known;
				} 
				
				if (MagicDisallowEnchanting(baseEnchantment)) {
					return EnchantmentType::CannotDisenchant;
				}
			}
		}

		// Its safe to assume that if it not a base enchanted item, that it was enchanted by the player and therefore, they
		// know the enchantment
		if (wasExtra) {
			return EnchantmentType::Known;
		} 
		
		if (enchantable) {
			return MagicDisallowEnchanting(keyWordForm) ? EnchantmentType::CannotDisenchant : result;
		}

		return result;
	}

	// Almost straight up copied from MoreHudSE. Had to change some things to work with this. https://github.com/ahzaab/moreHUDSE
	void ItemListEntry::SetupEnchantmentFlags() const
	{
		EnchantmentType ench_type = GetEnchantmentType();

		_cache.SetEnchanted(ench_type != EnchantmentType::None);
		_cache.SetKnownEnchanted(ench_type == EnchantmentType::Known);
		_cache.SetSpecialEnchanted(ench_type == EnchantmentType::CannotDisenchant);
	}

	const char* ItemListEntry::GetItemIconLabel(ItemType form) const
	{
		size_t form_num = static_cast<size_t>(form);
		if (form_num < sizeof(strIcons) / sizeof(strIcons[0]))
			return strIcons[form_num];

		return strIcons[0];
	}
}
