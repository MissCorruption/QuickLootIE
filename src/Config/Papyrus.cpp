#include "Papyrus.h"

#include "LootMenu.h"
#include "UserSettings.h"
#include "Util/ScriptObject.h"

namespace QuickLoot::Config
{
	void Papyrus::Init()
	{
		SKSE::GetPapyrusInterface()->Register(RegisterFunctions);
	}

	bool Papyrus::RegisterFunctions(RE::BSScript::IVirtualMachine* vm)
	{
#define RegisterScriptFunction(name) vm->RegisterFunction(#name, "QuickLootIENative", name);

		RegisterScriptFunction(GetDllVersion);
		RegisterScriptFunction(GetSwfVersion);
		RegisterScriptFunction(SetFrameworkQuest);
		RegisterScriptFunction(LogWithPlugin);
		RegisterScriptFunction(UpdateVariables);
		RegisterScriptFunction(FormatSortOptionsList);
		RegisterScriptFunction(RemoveSortOptionPriority);
		RegisterScriptFunction(InsertSortOptionPriority);
		RegisterScriptFunction(GetSortingPreset);
		RegisterScriptFunction(GetSortingPresets);
		RegisterScriptFunction(AddPresetsToArray);

#undef RegisterScriptFunction
		return true;
	};

	void Papyrus::SetFrameworkQuest(RE::StaticFunctionTag*, RE::TESQuest* quest)
	{
		if (!quest) {
			logger::info("No quest passed to registration function");
			return;
		};

		MCMScript = Util::ScriptObject::FromForm(quest, "QuickLootIEMCM");
		if (!MCMScript.IsValid()) {
			logger::info("Unable to locate MCM script on form");
			return;
		};

		logger::info("MCM pointer set successfully");

		UserSettings::Update();
	};

	void Papyrus::UpdateVariables(RE::StaticFunctionTag*)
	{
#define LoadSettingsVar(name, ...) LoadSetting(name, #name, __VA_ARGS__)

		// General > Behavior Settings
		LoadSettingsVar(QLIE_ShowInCombat, false);
		LoadSettingsVar(QLIE_ShowWhenEmpty, true);
		LoadSettingsVar(QLIE_ShowWhenUnlocked, true);
		LoadSettingsVar(QLIE_ShowInThirdPerson, true);
		LoadSettingsVar(QLIE_ShowWhenMounted, false);
		LoadSettingsVar(QLIE_EnableForAnimals, false);
		LoadSettingsVar(QLIE_EnableForDragons, false);
		LoadSettingsVar(QLIE_BreakInvisibility, true);

		// Display > Window Settings
		LoadSettingsVar(QLIE_WindowOffsetX, 100);
		LoadSettingsVar(QLIE_WindowOffsetY, -200);
		LoadSettingsVar(QLIE_WindowScale, 1.0f);
		LoadSettingsVar(QLIE_WindowAnchor, 0);
		LoadSettingsVar(QLIE_WindowMinLines, 0);
		LoadSettingsVar(QLIE_WindowMaxLines, 7);
		LoadSettingsVar(QLIE_WindowOpacityNormal, 1.0f);
		LoadSettingsVar(QLIE_WindowOpacityEmpty, 0.3f);

		// Display > Icon Settings
		LoadSettingsVar(QLIE_ShowIconItem, true);
		LoadSettingsVar(QLIE_ShowIconRead, true);
		LoadSettingsVar(QLIE_ShowIconStolen, true);
		LoadSettingsVar(QLIE_ShowIconEnchanted, true);
		LoadSettingsVar(QLIE_ShowIconEnchantedKnown, true);
		LoadSettingsVar(QLIE_ShowIconEnchantedSpecial, true);

		// Display > Info Columns
		LoadSettingsVar(QLIE_InfoColumns, { "value", "weight", "valuePerWeight" });

		// Sorting
		LoadSettingsVar(QLIE_SortRulesActive, {});

		// Controls
		LoadSettingsVar(QLIE_KeybindingTake, 18);
		LoadSettingsVar(QLIE_KeybindingTakeAll, 19);
		LoadSettingsVar(QLIE_KeybindingTransfer, 16);
		LoadSettingsVar(QLIE_KeybindingDisable, -1);
		LoadSettingsVar(QLIE_KeybindingEnable, -1);

		LoadSettingsVar(QLIE_KeybindingTakeModifier, 0);
		LoadSettingsVar(QLIE_KeybindingTakeAllModifier, 0);
		LoadSettingsVar(QLIE_KeybindingTransferModifier, 0);
		LoadSettingsVar(QLIE_KeybindingDisableModifier, 0);
		LoadSettingsVar(QLIE_KeybindingEnableModifier, 0);

		LoadSettingsVar(QLIE_KeybindingTakeGamepad, 276);
		LoadSettingsVar(QLIE_KeybindingTakeAllGamepad, 278);
		LoadSettingsVar(QLIE_KeybindingTransferGamepad, 271);
		LoadSettingsVar(QLIE_KeybindingDisableGamepad, -1);
		LoadSettingsVar(QLIE_KeybindingEnableGamepad, -1);

		// Compatibility > LOTD Icons
		LoadSettingsVar(QLIE_ShowIconArtifactNew, false);
		LoadSettingsVar(QLIE_ShowIconArtifactCarried, false);
		LoadSettingsVar(QLIE_ShowIconArtifactDisplayed, false);

		// Compatibility > Completionist Icons
		LoadSettingsVar(QLIE_ShowIconCompletionistNeeded, false);
		LoadSettingsVar(QLIE_ShowIconCompletionistCollected, false);

#undef LoadSettingsVar
	}

	void Papyrus::LogWithPlugin(RE::StaticFunctionTag*, std::string message)
	{
		logger::info("! {}", message);
	}

	std::string Papyrus::GetDllVersion(RE::StaticFunctionTag*)
	{
		return Plugin::VERSION.string(".");
	}

	std::string Papyrus::GetSwfVersion(RE::StaticFunctionTag*)
	{
		return std::to_string(LootMenu::GetSwfVersion());
	}

	std::vector<std::string> Papyrus::FormatSortOptionsList(RE::StaticFunctionTag*, std::vector<std::string> options, std::vector<std::string> userList)
	{
		std::erase_if(options, [&userList](const std::string& option) {
			return std::ranges::find(userList, option) != userList.end();
		});

		return options;
	}

	std::vector<std::string> Papyrus::RemoveSortOptionPriority(RE::StaticFunctionTag*, std::vector<std::string> userList, int elementPos)
	{
		if (elementPos >= 0 && elementPos < static_cast<int>(userList.size())) {
			userList.erase(userList.begin() + elementPos);
		}

		return userList;
	}

	std::vector<std::string> Papyrus::InsertSortOptionPriority(RE::StaticFunctionTag*, std::vector<std::string> userList, std::string newElement, int elementPos)
	{
		if (elementPos >= 0 && elementPos <= static_cast<int>(userList.size())) {
			userList.insert(userList.begin() + elementPos, newElement);
		}

		return userList;
	}

	std::vector<std::string> Papyrus::AddPresetsToArray(RE::StaticFunctionTag*, std::vector<std::string> userList, std::vector<std::string> presetList)
	{
		userList.insert(userList.end(), presetList.begin(), presetList.end());
		return userList;
	}

	std::vector<std::string> Papyrus::GetSortingPresets(RE::StaticFunctionTag*)
	{
		return ConvertArrayToVector(SortingPresets);
	}

	std::vector<std::string> Papyrus::GetSortingPreset(RE::StaticFunctionTag*, int presetChoice)
	{
		switch (presetChoice) {
		case 1:
			return ConvertArrayToVector(SortingPresets_Default);
		case 2:
			return ConvertArrayToVector(SortingPresets_Goblin);
		default:
			return ConvertArrayToVector(SortingPresets_Default);
		}
	}
}
