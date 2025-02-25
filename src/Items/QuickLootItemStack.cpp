#include "QuickLootItemStack.h"

#include "Config/UserSettings.h"
#include "Integrations/APIServer.h"
#include "Integrations/Artifacts.h"
#include "Integrations/Completionist.h"

namespace QuickLoot::Items
{
	QuickLootItemStack::QuickLootItemStack(std::unique_ptr<ItemStack> stack) :
		_stack(std::move(stack)) {}

	ItemData& QuickLootItemStack::GetData() const
	{
		return _stack->GetData();
	}

	QuickLootItemData& QuickLootItemStack::GetQuickLootData() const
	{
		if (_dataInitialized) {
			return _data;
		}

		SetQuickLootData();

		return _data;
	}

	RE::GFxValue& QuickLootItemStack::BuildDataObject(RE::GFxMovieView* view)
	{
		auto& obj = _stack->BuildDataObject(view);

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

		SetDataMember(obj, "compNew", data.compNew);
		SetDataMember(obj, "compFound", data.compFound);

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

	void QuickLootItemStack::TakeStack(RE::Actor* actor) const
	{
		Take(actor, _stack->GetEntry()->countDelta);
	}

	void QuickLootItemStack::TakeOne(RE::Actor* actor) const
	{
		return Take(actor, 1);
	}

	void QuickLootItemStack::Take(RE::Actor* actor, int count) const
	{
		if (API::APIServer::DispatchTakingItemEvent(actor, GetContainer().get().get(), GetEntry(), GetDropRef().get().get()) == API::HandleResult::kStop) {
			return;
		}

		_stack->Take(actor, count);

		API::APIServer::DispatchTakeItemEvent(actor, GetContainer().get().get(), GetEntry(), GetDropRef().get().get());
	}

	std::vector<QuickLootItemStack> QuickLootItemStack::LoadContainerInventory(RE::TESObjectREFR* container, const std::function<bool(RE::TESBoundObject&)>& filter)
	{
		auto inventory = ItemStack::LoadContainerInventory(container, filter);
		std::vector<QuickLootItemStack> stacks{};
		stacks.reserve(inventory.size());

		for (auto& stack : inventory) {
			stacks.emplace_back(std::move(stack));
		}

		return stacks;
	}

	void QuickLootItemStack::SetQuickLootData() const
	{
		PROFILE_SCOPE;

		_data.displayName = _stack->GetEntry()->GetDisplayName();

		using Settings = Config::UserSettings;
		using namespace Integrations;

		const auto object = _stack->GetEntry()->object;

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

			const auto data = Completionist::GetItemInfo(object);

			_data.compNew = Settings::ShowCompletionistNeeded() && data.isNeeded;
			_data.compFound = Settings::ShowCompletionistCollected() && data.isCollected;
			_data.displayName = data.decoratedName;

			if (data.textColor != -1) {
				_data.textColor = data.textColor;
			}
		}
	}
}
