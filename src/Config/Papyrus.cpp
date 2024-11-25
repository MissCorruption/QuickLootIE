#include "Papyrus.h"

#include "Settings.h"
#include "Util/ScriptObject.h"

namespace QuickLoot::Config
{
	void Papyrus::Init()
	{
		SKSE::GetPapyrusInterface()->Register(RegisterFunctions);
	}

	bool Papyrus::RegisterFunctions(RE::BSScript::IVirtualMachine* vm)
	{
		vm->RegisterFunction("GetVersion",					"QuickLootIENative", GetVersion);
		vm->RegisterFunction("SetFrameworkQuest",			"QuickLootIENative", SetFrameworkQuest);
		vm->RegisterFunction("LogWithPlugin",				"QuickLootIENative", LogWithPlugin);
		vm->RegisterFunction("UpdateVariables",				"QuickLootIENative", UpdateVariables);
		vm->RegisterFunction("FormatSortOptionsList",		"QuickLootIENative", FormatSortOptionsList);
		vm->RegisterFunction("RemoveSortOptionPriority",	"QuickLootIENative", RemoveSortOptionPriority);
		vm->RegisterFunction("InsertSortOptionPriority",	"QuickLootIENative", InsertSortOptionPriority);
		vm->RegisterFunction("GetSortingPreset",			"QuickLootIENative", GetSortingPreset);
		vm->RegisterFunction("GetSortingPresets",			"QuickLootIENative", GetSortingPresets);
		vm->RegisterFunction("AddPresetsToArray",			"QuickLootIENative", AddPresetsToArray);
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

		Settings::Update();
	};

	void Papyrus::UpdateVariables(RE::StaticFunctionTag*)
	{
		// General > Behavior Settings
		LoadSetting(QLIE_ShowInCombat,						"QLIE_ShowInCombat", false);
		LoadSetting(QLIE_ShowWhenEmpty,						"QLIE_ShowWhenEmpty", true);
		LoadSetting(QLIE_ShowWhenUnlocked,					"QLIE_ShowWhenUnlocked", true);
		LoadSetting(QLIE_ShowInThirdPerson,					"QLIE_ShowInThirdPerson", true);
		LoadSetting(QLIE_ShowWhenMounted,					"QLIE_ShowWhenMounted", false);
		LoadSetting(QLIE_EnableForAnimals,					"QLIE_EnableForAnimals", false);
		LoadSetting(QLIE_EnableForDragons,					"QLIE_EnableForDragons", false);
		LoadSetting(QLIE_BreakInvisibility,					"QLIE_BreakInvisibility", true);

		// Display > Window Settings
		LoadSetting(QLIE_WindowOffsetX,						"QLIE_WindowOffsetX", 100);
		LoadSetting(QLIE_WindowOffsetY,						"QLIE_WindowOffsetY", -200);
		LoadSetting(QLIE_WindowScale,						"QLIE_WindowScale", 1.0f);
		LoadSetting(QLIE_WindowAnchor,						"QLIE_WindowAnchor", 0);
		LoadSetting(QLIE_WindowMinLines,					"QLIE_WindowMinLines", 0);
		LoadSetting(QLIE_WindowMaxLines,					"QLIE_WindowMaxLines", 7);
		LoadSetting(QLIE_WindowOpacityNormal,				"QLIE_WindowOpacityNormal", 1.0f);
		LoadSetting(QLIE_WindowOpacityEmpty,				"QLIE_WindowOpacityEmpty", 0.3f);

		// Display > Icon Settings
		LoadSetting(QLIE_ShowIconRead,						"QLIE_ShowIconRead", true);
		LoadSetting(QLIE_ShowIconStolen,					"QLIE_ShowIconStolen", true);
		LoadSetting(QLIE_ShowIconEnchanted,					"QLIE_ShowIconEnchanted", true);
		LoadSetting(QLIE_ShowIconEnchantedKnown,			"QLIE_ShowIconEnchantedKnown", true);
		LoadSetting(QLIE_ShowIconEnchantedSpecial,			"QLIE_ShowIconEnchantedSpecial", true);

		// Display > Info Columns
		LoadSetting(QLIE_InfoColumns,						"QLIE_InfoColumns", { "value", "weight", "valuePerWeight" });

		// Sorting
		LoadSetting(QLIE_SortRulesActive,					"QLIE_SortRulesActive", {});

		// Controls
		LoadSetting(QLIE_KeybindingTake,					"QLIE_KeybindingTake", 18);
		LoadSetting(QLIE_KeybindingTakeAll,					"QLIE_KeybindingTakeAll", 19);
		LoadSetting(QLIE_KeybindingTransfer,				"QLIE_KeybindingTransfer", 16);
		LoadSetting(QLIE_KeybindingTakeModifier,			"QLIE_KeybindingTakeModifier", 0);
		LoadSetting(QLIE_KeybindingTakeAllModifier,			"QLIE_KeybindingTakeAllModifier", 0);
		LoadSetting(QLIE_KeybindingTransferModifier,		"QLIE_KeybindingTransferModifier", 0);
		LoadSetting(QLIE_KeybindingTakeGamepad,				"QLIE_KeybindingTakeGamepad", 276);
		LoadSetting(QLIE_KeybindingTakeAllGamepad,			"QLIE_KeybindingTakeAllGamepad", 278);
		LoadSetting(QLIE_KeybindingTransferGamepad,			"QLIE_KeybindingTransferGamepad", 271);

		// Compatibility > LOTD Icons
		LoadSetting(QLIE_ShowIconLOTDNew,					"QLIE_ShowIconLOTDNew", false);
		LoadSetting(QLIE_ShowIconLOTDCarried,				"QLIE_ShowIconLOTDCarried", false);
		LoadSetting(QLIE_ShowIconLOTDDisplayed,				"QLIE_ShowIconLOTDDisplayed", false);

		// Compatibility > Completionist Icons
		LoadSetting(QLIE_ShowIconCompletionistNeeded,		"QLIE_ShowIconCompletionistNeeded", false);
		LoadSetting(QLIE_ShowIconCompletionistCollected,	"QLIE_ShowIconCompletionistCollected", false);
	}

	void Papyrus::LogWithPlugin(RE::StaticFunctionTag*, std::string message)
	{
		logger::info("! {}", message);
	}

	std::string Papyrus::GetVersion(RE::StaticFunctionTag*)
	{
		return std::string(ReplaceStr(Plugin::VERSION.string(), "-", "."));
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
