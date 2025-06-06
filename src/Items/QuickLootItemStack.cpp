#include "QuickLootItemStack.h"

#include "Config/UserSettings.h"
#include "Integrations/APIServer.h"
#include "Integrations/Artifacts.h"
#include "Integrations/Completionist.h"

namespace QuickLoot::Items
{
	QuickLootItemStack::QuickLootItemStack(RE::InventoryEntryData* entry, RE::ObjectRefHandle container, RE::ObjectRefHandle dropRef)
		: ItemStack(entry, std::move(container), std::move(dropRef)) {}

	QuickLootItemData& QuickLootItemStack::GetQuickLootData()
	{
		if (_dataInitialized) {
			return _data;
		}

		SetQuickLootData();

		return _data;
	}

	RE::GFxValue& QuickLootItemStack::BuildDataObject(RE::GFxMovieView* view)
	{
		auto& obj = ItemStack::BuildDataObject(view);

		const auto& data = GetQuickLootData();

		if (obj.HasMember("quickLoot")) {
			return obj;
		}

		SetDataMember(obj, "displayName", data.displayName.value.c_str());
		SetDataMember(obj, "textColor", data.textColor);

		SetDataMember(obj, "enchanted", data.enchanted);
		SetDataMember(obj, "knownEnchanted", data.knownEnchanted);
		SetDataMember(obj, "specialEnchanted", data.specialEnchanted);

		SetDataMember(obj, "dbmNew", data.dbmNew);
		SetDataMember(obj, "dbmFound", data.dbmFound);
		SetDataMember(obj, "dbmDisplayed", data.dbmDisplayed);

		SetDataMember(obj, "compNeeded", data.compNeeded);
		SetDataMember(obj, "compCollected", data.compCollected);
		SetDataMember(obj, "compDisplayable", data.compDisplayable);
		SetDataMember(obj, "compDisplayed", data.compDisplayed);
		SetDataMember(obj, "compOccupied", data.compOccupied);

		// For backwards compatibility
		const auto& baseData = GetData();
		SetDataMember(obj, "value", baseData.value);
		SetDataMember(obj, "weight", std::max(baseData.weight.value, 0.0f));
		SetDataMember(obj, "stolen", baseData.isStealing);
		if (baseData.formType == RE::FormType::Book) {
			SetDataMember(obj, "read", baseData.book.isRead);
		}

		obj.SetMember("quickLoot", true);
		return obj;
	}

	void QuickLootItemStack::OnSelected(RE::Actor* actor) const
	{
		API::APIServer::DispatchSelectItemEvent(actor, GetContainer().get().get(), GetEntry(), GetDropRef().get().get());
	}

	void QuickLootItemStack::Take(RE::Actor* actor, int count) const
	{
		if (API::APIServer::DispatchTakingItemEvent(actor, GetContainer().get().get(), GetEntry(), GetDropRef().get().get()) == API::HandleResult::kStop) {
			return;
		}

		ItemStack::Take(actor, count);

		API::APIServer::DispatchTakeItemEvent(actor, GetContainer().get().get(), GetEntry(), GetDropRef().get().get());
	}

	void QuickLootItemStack::SetQuickLootData()
	{
		PROFILE_SCOPE;

		_data.displayName = _entry->GetDisplayName();

		using Settings = Config::UserSettings;
		using namespace Integrations;

		const auto object = _entry->object;

		if (Settings::ShowIconEnchanted()) {
			PROFILE_SCOPE_NAMED("Enchantment Data");

			const auto enchantmentType = GetData().enchantmentType;
			_data.enchanted = enchantmentType != EnchantmentType::kNone;
			_data.knownEnchanted = Settings::ShowIconEnchantedKnown() && enchantmentType == EnchantmentType::kKnown;
			_data.specialEnchanted = Settings::ShowIconEnchantedSpecial() && enchantmentType == EnchantmentType::kCannotDisenchant;
		}

		if (Artifacts::IsIntegrationEnabled()) {
			PROFILE_SCOPE_NAMED("Artifact Data");

			_data.dbmNew = Settings::ShowArtifactNew() && Artifacts::IsNewArtifact(object->formID);
			_data.dbmFound = Settings::ShowArtifactFound() && Artifacts::IsFoundArtifact(object->formID);
			_data.dbmDisplayed = Settings::ShowArtifactDisplayed() && Artifacts::IsDisplayedArtifact(object->formID);
		}

		if (Completionist::IsReady()) {
			PROFILE_SCOPE_NAMED("Completionist Data");

			const auto data = Completionist::GetItemInfo(_entry.get());

			_data.displayName = data.decoratedName;

			_data.compNeeded = Settings::ShowCompletionistNeeded() && data.isNeeded;
			_data.compCollected = Settings::ShowCompletionistCollected() && data.isCollected;
			_data.compDisplayable = Settings::ShowCompletionistDisplayable() && data.isDisplayable;
			_data.compDisplayed = Settings::ShowCompletionistDisplayed() && data.isDisplayed;
			_data.compOccupied = Settings::ShowCompletionistOccupied() && data.isOccupied;

			if (data.textColor != -1) {
				_data.textColor = data.textColor;
			}
		}
	}
}
