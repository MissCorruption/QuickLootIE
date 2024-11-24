#pragma once

#include "Util/ScriptObject.h"

namespace QuickLoot::Config
{
	static Util::ScriptObject MCMScript;

	inline bool QLIE_ShowInCombat;
	inline bool QLIE_ShowWhenEmpty;
	inline bool QLIE_ShowWhenUnlocked;
	inline bool QLIE_ShowInThirdPerson;
	inline bool QLIE_ShowWhenMounted;
	inline bool QLIE_EnableForAnimals;
	inline bool QLIE_EnableForDragons;
	inline bool QLIE_BreakInvisibility;

	inline int QLIE_WindowOffsetX;
	inline int QLIE_WindowOffsetY;
	inline float QLIE_WindowScale;
	inline int QLIE_WindowAnchor;
	inline int QLIE_WindowMinLines;
	inline int QLIE_WindowMaxLines;
	inline float QLIE_WindowOpacityNormal;
	inline float QLIE_WindowOpacityEmpty;

	inline bool QLIE_ShowIconRead;
	inline bool QLIE_ShowIconStolen;
	inline bool QLIE_ShowIconEnchanted;
	inline bool QLIE_ShowIconEnchantedKnown;
	inline bool QLIE_ShowIconEnchantedSpecial;

	inline std::vector<std::string> QLIE_SortRulesActive;

	inline int QLIE_KeybindingTake;
	inline int QLIE_KeybindingTakeAll;
	inline int QLIE_KeybindingTransfer;
	inline int QLIE_KeybindingTakeModifier;
	inline int QLIE_KeybindingTakeAllModifier;
	inline int QLIE_KeybindingTransferModifier;
	inline int QLIE_KeybindingTakeGamepad;
	inline int QLIE_KeybindingTakeAllGamepad;
	inline int QLIE_KeybindingTransferGamepad;

	inline bool QLIE_ShowIconLOTDNew;
	inline bool QLIE_ShowIconLOTDCarried;
	inline bool QLIE_ShowIconLOTDDisplayed;

	inline bool QLIE_ShowIconCompletionistNeeded;
	inline bool QLIE_ShowIconCompletionistCollected;

	class Papyrus
	{
	public:
		Papyrus() = delete;
		~Papyrus() = delete;
		Papyrus(Papyrus const&) = delete;
		Papyrus(Papyrus const&&) = delete;
		Papyrus& operator=(Papyrus&) = delete;
		Papyrus& operator=(Papyrus&&) = delete;

		static void Init();
		static void SetFrameworkQuest(RE::StaticFunctionTag*, RE::TESQuest* quest);
		static void LogWithPlugin(RE::StaticFunctionTag*, std::string message);
		static void UpdateVariables(RE::StaticFunctionTag* = nullptr);
		static std::string GetVersion(RE::StaticFunctionTag*);

		static std::vector<std::string> FormatSortOptionsList(RE::StaticFunctionTag*, std::vector<std::string> options, std::vector<std::string> userList);
		static std::vector<std::string> RemoveSortOptionPriority(RE::StaticFunctionTag*, std::vector<std::string> userList, int elementPos);
		static std::vector<std::string> InsertSortOptionPriority(RE::StaticFunctionTag*, std::vector<std::string> userList, std::string newElement, int elementPos);

		static std::vector<std::string> GetSortingPresets(RE::StaticFunctionTag*);
		static std::vector<std::string> GetSortingPreset(RE::StaticFunctionTag*, int presetChoice);
		static std::vector<std::string> AddPresetsToArray(RE::StaticFunctionTag*, std::vector<std::string> userList, std::vector<std::string> presetList);

	private:
		static bool RegisterFunctions(RE::BSScript::IVirtualMachine* vm);
		static std::string ReplaceStr(const std::string& in, const std::string& from, const std::string& to)
		{
			return std::regex_replace(in, std::regex(from), to);
		}

		static std::vector<std::string> ConvertScriptArrayToVector(ScriptArrayPtr scriptArrayPtr)
		{
			std::vector<std::string> result;

			if (!scriptArrayPtr) {
				return result;
			}

			std::size_t arraySize = scriptArrayPtr->size();
			result.reserve(arraySize);

			for (RE::BSScript::Array::size_type i = 0; i < arraySize; ++i) {
				const auto& variable = (*scriptArrayPtr)[i].GetString();
				result.push_back(variable.data());
			}

			return result;
		}

		template <size_t N>
		static std::vector<std::string> ConvertArrayToVector(const std::array<const char*, N>& arr)
		{
			std::vector<std::string> vec;
			vec.reserve(N);

			for (size_t i = 0; i < N; ++i) {
				vec.push_back(arr[i]);
			}

			return vec;
		}

		template <typename T>
		static void LoadSetting(T& variable, const std::string& propertyName, const T& defaultValue)
		{
			const auto* prop = MCMScript.GetProperty(propertyName);

			if (!prop) {
				variable = defaultValue;
				logger::trace("{}: not found", propertyName);
				return;
			}

			if constexpr (std::is_same_v<T, bool>) {
				variable = prop->GetBool();
				logger::trace("{}: {}", propertyName, variable);
				return;
			}

			if constexpr (std::is_same_v<T, std::string>) {
				variable = prop->GetString();
				logger::trace("{}: {}", propertyName, variable);
				return;
			}

			if constexpr (std::is_same_v<T, int>) {
				variable = prop->GetSInt();
				logger::trace("{}: {}", propertyName, variable);
				return;
			}

			if constexpr (std::is_same_v<T, float>) {
				variable = prop->GetFloat();
				logger::trace("{}: {}", propertyName, variable);
				return;
			}

			if constexpr (std::is_same_v<T, std::vector<std::string>>) {
				variable = ConvertScriptArrayToVector(prop->GetArray());
				logger::trace("{}: {} strings", propertyName, variable.size());
				return;
			}

			//logger::trace("{}: unsupported type {}", propertyName, typeid(T).name());
		}
	};
}

//These are just examples, we should localise these for the MCM e.g "The Goblin" would be "$qlie_preset_goblin"
constexpr std::array SortingPresets = {
	"Select Preset...",  //Mandatory Entry
	"Default Preset",
	"The Goblin",
	//Add more as you see fit.
};

constexpr std::array SortingPresets_Default = {
	"$qlie_SortRule_Gold",
	"$qlie_SortRule_Gems",
	"$qlie_SortRule_SoulGems",
	"$qlie_SortRule_Lockpicks",
	"$qlie_SortRule_Ingots",
	"$qlie_SortRule_Ores",
	"$qlie_SortRule_Potions",
	"$qlie_SortRule_FoodDrinks",
	"$qlie_SortRule_Books",
	"$qlie_SortRule_Notes",
	"$qlie_SortRule_Scrolls",
	"$qlie_SortRule_ArrowsBolts",
	"$qlie_SortRule_Jewelry",
	"$qlie_SortRule_Weapons",
	"$qlie_SortRule_Armors",
	"$qlie_SortRule_Clothes",
	"$qlie_SortRule_Weightless",
	"$qlie_SortRule_ByWeight",
	"$qlie_SortRule_ByValue",
	"$qlie_SortRule_ByName",
	"$qlie_SortRule_LOTDNeeded",
	"$qlie_SortRule_CompletionistNeeded",
};

constexpr std::array SortingPresets_Goblin = {
	"$qlie_SortRule_Gold",
	"$qlie_SortRule_Gems",
	"$qlie_SortRule_SoulGems",
	"$qlie_SortRule_Lockpicks",
	"$qlie_SortRule_Ingots",
	"$qlie_SortRule_Ores",
	"$qlie_SortRule_Potions",
	"$qlie_SortRule_FoodDrinks",
	"$qlie_SortRule_Books",
	"$qlie_SortRule_Notes",
	"$qlie_SortRule_Scrolls",
	"$qlie_SortRule_ArrowsBolts",
	"$qlie_SortRule_Jewelry",
	"$qlie_SortRule_Weapons",
	"$qlie_SortRule_Armors",
	"$qlie_SortRule_Clothes",
	"$qlie_SortRule_Weightless",
	"$qlie_SortRule_ByWeight",
	"$qlie_SortRule_ByValue",
	"$qlie_SortRule_ByName",
	"$qlie_SortRule_LOTDNeeded",
	"$qlie_SortRule_CompletionistNeeded",
};
