#include "Papyrus.h"
#include "Helpers/ScriptObject.hpp"

namespace QuickLoot
{
	void Papyrus::Init()
	{
		RE::UI::GetSingleton()->AddEventSink(static_cast<RE::BSTEventSink<RE::MenuOpenCloseEvent>*>(GetSingleton()));
		SKSE::GetPapyrusInterface()->Register(Papyrus::RegisterFunctions);
	}

	//---------------------------------------------------
	//-- Framework Functions ( Papyrus Registrations ) --
	//---------------------------------------------------

	auto Papyrus::RegisterFunctions(RE::BSScript::IVirtualMachine* a_vm) -> bool
	{
		a_vm->RegisterFunction("GetVersion",				"QuickLootIENative", GetVersion);
		a_vm->RegisterFunction("SetFrameworkQuest",			"QuickLootIENative", SetFrameworkQuest);
		a_vm->RegisterFunction("LogWithPlugin",				"QuickLootIENative", LogWithPlugin);
		a_vm->RegisterFunction("UpdateVariables",			"QuickLootIENative", UpdateVariables);
		a_vm->RegisterFunction("FormatSortOptionsList",		"QuickLootIENative", FormatSortOptionsList);
		a_vm->RegisterFunction("RemoveSortOptionPriority",	"QuickLootIENative", RemoveSortOptionPriority);
		a_vm->RegisterFunction("InsertSortOptionPriority",	"QuickLootIENative", InsertSortOptionPriority);
		a_vm->RegisterFunction("GetSortingPreset",			"QuickLootIENative", GetSortingPreset);
		a_vm->RegisterFunction("GetSortingPresets",			"QuickLootIENative", GetSortingPresets);
		a_vm->RegisterFunction("AddPresetsToArray",			"QuickLootIENative", AddPresetsToArray);		
		return true;
	};

	//---------------------------------------------------
	//-- Variables Functions ( On Menu Closed ) ---------
	//---------------------------------------------------

	RE::BSEventNotifyControl Papyrus::ProcessEvent(RE::MenuOpenCloseEvent const* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
	{
		if (!a_event->opening && a_event->menuName == RE::JournalMenu::MENU_NAME)
		{
			UpdateVariables(nullptr);
		};

		return RE::BSEventNotifyControl::kContinue;
	}

	//---------------------------------------------------
	//-- Variables Functions ( Set MCM Pointer ) --------
	//---------------------------------------------------

	void Papyrus::SetFrameworkQuest(RE::StaticFunctionTag*, RE::TESQuest* a_quest)
	{
		if (!a_quest)
		{
			logger::info("No Quest Passed To Registration Function");
			return;
		};

		MCMScript = ScriptObject::FromForm(a_quest, "QuickLootIEMCM");
		if (!MCMScript)
		{
			logger::info("Unable To Locate MCM Script on Form");
			return;
		};

		logger::info("MCM Pointer Set Successfully");
		UpdateVariables(nullptr);
	};

	//---------------------------------------------------
	//-- Framework Functions ( Update From MCM ) --------
	//---------------------------------------------------

	void Papyrus::UpdateVariables(RE::StaticFunctionTag*)
	{
		LoadSetting(QLIECloseInCombat,				"QLIECloseInCombat", false);
		LoadSetting(QLIECloseWhenEmpty,				"QLIECloseWhenEmpty", true);
		LoadSetting(QLIEDispelInvisibility,			"QLIEDispelInvisibility", true);
		LoadSetting(QLIEOpenWhenContainerUnlocked,	"QLIEOpenWhenContainerUnlocked", true);
		LoadSetting(QLIEDisableForAnimals,			"QLIEDisableForAnimals", false);

		LoadSetting(QLIEIconShowBookRead,			"QLIEIconShowBookRead", true);
		LoadSetting(QLIEIconShowEnchanted,			"QLIEIconShowEnchanted", true);

		LoadSetting(QLIEIconShowDBMDisplayed,		"QLIEIconShowDBMDisplayed", false);
		LoadSetting(QLIEIconShowDBMFound,			"QLIEIconShowDBMFound", false);
		LoadSetting(QLIEIconShowDBMNew,				"QLIEIconShowDBMNew", false);

		LoadSetting(QLIEShowCompNeeded,				"QLIEShowCompNeeded", false);
		LoadSetting(QLIEShowCompCollected,			"QLIEShowCompCollected", false);

		LoadSetting(QLIEWindowX,					"QLIEWindowX", 0.f);
		LoadSetting(QLIEWindowY,					"QLIEWindowY", 0.f);
		LoadSetting(QLIEWindowW,					"QLIEWindowW", 0.f);
		LoadSetting(QLIEWindowH,					"QLIEWindowH", 0.f);

		LoadSetting(QLIESortPriorityStrings,		"user_selected_sort_options", {});
	}

	//---------------------------------------------------
	//-- Framework Functions ( Log From Papyrus ) -------
	//---------------------------------------------------

	void Papyrus::LogWithPlugin(RE::StaticFunctionTag*, std::string a_message) 
	{

		logger::info("Papyrus Message: {}", a_message);
	}

	//---------------------------------------------------
	//-- Framework Functions ( Get DLL Version ) --------
	//---------------------------------------------------

	std::string Papyrus::GetVersion(RE::StaticFunctionTag*) 
	{ 
		return std::string(ReplaceStr(Plugin::VERSION.string(), "-", ".")); 
	}

	//---------------------------------------------------
	//-- Framework Functions (Format Sort Options ) -----
	//---------------------------------------------------

	std::vector<std::string> Papyrus::FormatSortOptionsList(RE::StaticFunctionTag*, std::vector<std::string> options, std::vector<std::string> userList)
	{
		options.erase(
			std::remove_if(options.begin(), options.end(), [&userList](const std::string& option) {
				return std::find(userList.begin(), userList.end(), option) != userList.end();
				}),
			options.end()
		);

		return options;
	}

	//---------------------------------------------------
	//-- Framework Functions (Format Sort Options ) -----
	//---------------------------------------------------

	std::vector<std::string> Papyrus::RemoveSortOptionPriority(RE::StaticFunctionTag*, std::vector<std::string> userList, int32_t elementPos)
	{
		if (elementPos >= 0 && elementPos < static_cast<int32_t>(userList.size())) {
			userList.erase(userList.begin() + elementPos);
		}
		return userList;
	}

	//---------------------------------------------------
	//-- Framework Functions (Format Sort Options ) -----
	//---------------------------------------------------

	std::vector<std::string> Papyrus::InsertSortOptionPriority(RE::StaticFunctionTag*, std::vector<std::string> userList, std::string newElement, int32_t elementPos)
	{
		if (elementPos >= 0 && elementPos <= static_cast<int32_t>(userList.size())) {
			userList.insert(userList.begin() + elementPos, newElement);
		}

		return userList;
	}

	//---------------------------------------------------
	//-- Framework Functions (Format Sort Options ) -----
	//---------------------------------------------------

	std::vector<std::string> Papyrus::AddPresetsToArray(RE::StaticFunctionTag*, std::vector<std::string> userList, std::vector<std::string> presetList)
	{
		userList.insert(userList.end(), presetList.begin(), presetList.end());
		return userList;
	}

	//---------------------------------------------------
	//-- Framework Functions (Get Names of Presets) -----
	//---------------------------------------------------

	std::vector<std::string> Papyrus::GetSortingPresets(RE::StaticFunctionTag*) 
	{
		return ConvertArrayToVector(SortingPresets);
	}

	//---------------------------------------------------
	//-- Framework Functions (Get Preset Elements) ------
	//---------------------------------------------------

	std::vector<std::string> Papyrus::GetSortingPreset(RE::StaticFunctionTag*, int32_t presetChoice) {

		switch (presetChoice)
		{
		case 1: return ConvertArrayToVector(SortingPresets_Default);
		case 2: return ConvertArrayToVector(SortingPresets_Goblin);
		default: 
			return  ConvertArrayToVector(SortingPresets_Default);
		}
	}
}
