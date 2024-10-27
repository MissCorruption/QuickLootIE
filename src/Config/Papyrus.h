#pragma once

#include "Util/ScriptObject.h"

namespace QuickLoot::Config
{
	static Util::ScriptObject MCMScript;

	inline bool QLIECloseInCombat;
	inline bool QLIECloseWhenEmpty;
	inline bool QLIEDispelInvisibility;
	inline bool QLIEOpenWhenContainerUnlocked;
	inline bool QLIEDisableForAnimals;

	inline bool QLIEIconShowBookRead;
	inline bool QLIEIconShowEnchanted;

	inline bool QLIEIconShowDBMDisplayed;
	inline bool QLIEIconShowDBMFound;
	inline bool QLIEIconShowDBMNew;

	inline bool QLIEShowCompNeeded;
	inline bool QLIEShowCompCollected;

	inline int32_t QLIEAnchorPoint;
	inline int32_t QLIEWindowX;
	inline int32_t QLIEWindowY;
	inline float QLIEWindowScale;

	inline int32_t QLIEMinLines;
	inline int32_t QLIEMaxLines;

	inline float QLIETransparency_Normal;
	inline float QLIETransparency_Empty;

	inline std::vector<std::string> QLIESortPriorityStrings;

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
		static void SetFrameworkQuest(RE::StaticFunctionTag*, RE::TESQuest* a_quest);
		static void LogWithPlugin(RE::StaticFunctionTag*, std::string a_message);
		static void UpdateVariables(RE::StaticFunctionTag* = nullptr);
		static std::string GetVersion(RE::StaticFunctionTag*);

		static std::vector<std::string> FormatSortOptionsList(RE::StaticFunctionTag*, std::vector<std::string> options, std::vector<std::string> userList);
		static std::vector<std::string> RemoveSortOptionPriority(RE::StaticFunctionTag*, std::vector<std::string> userList, int32_t elementPos);
		static std::vector<std::string> InsertSortOptionPriority(RE::StaticFunctionTag*, std::vector<std::string> userList, std::string newElement, int32_t elementPos);

		static std::vector<std::string> GetSortingPresets(RE::StaticFunctionTag*);
		static std::vector<std::string> GetSortingPreset(RE::StaticFunctionTag*, int32_t presetChoice);
		static std::vector<std::string> AddPresetsToArray(RE::StaticFunctionTag*, std::vector<std::string> userList, std::vector<std::string> presetList);

	private:
		static bool RegisterFunctions(RE::BSScript::IVirtualMachine* a_vm);
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

			if constexpr (std::is_same_v<T, int32_t>) {
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
constexpr std::array<const char*, 3> SortingPresets = {
	"Select Preset...",  //Mandatory Entry
	"Default Preset",
	"The Goblin",
	//Add more as you see fit.
};

constexpr std::array<const char*, 22> SortingPresets_Default = {
	"$qlie_sortOption_Gold",
	"$qlie_sortOption_Gems",
	"$qlie_sortOption_SoulGems",
	"$qlie_sortOption_Lockpicks",
	"$qlie_sortOption_Ingots",
	"$qlie_sortOption_Ores",
	"$qlie_sortOption_Potions",
	"$qlie_sortOption_FoodDrinks",
	"$qlie_sortOption_Books",
	"$qlie_sortOption_Notes",
	"$qlie_sortOption_Scrolls",
	"$qlie_sortOption_ArrowsBolts",
	"$qlie_sortOption_Jewelry",
	"$qlie_sortOption_Weapons",
	"$qlie_sortOption_Armors",
	"$qlie_sortOption_Clothes",
	"$qlie_sortOption_Weightless",
	"$qlie_sortOption_ByWeight",
	"$qlie_sortOption_ByValue",
	"$qlie_sortOption_ByName",
	"$qlie_sortOption_LOTDNeeded",
	"$qlie_sortOption_CompletionistNeeded",
};

constexpr std::array<const char*, 22> SortingPresets_Goblin = {
	"$qlie_sortOption_Gold",
	"$qlie_sortOption_Gems",
	"$qlie_sortOption_SoulGems",
	"$qlie_sortOption_Lockpicks",
	"$qlie_sortOption_Ingots",
	"$qlie_sortOption_Ores",
	"$qlie_sortOption_Potions",
	"$qlie_sortOption_FoodDrinks",
	"$qlie_sortOption_Books",
	"$qlie_sortOption_Notes",
	"$qlie_sortOption_Scrolls",
	"$qlie_sortOption_ArrowsBolts",
	"$qlie_sortOption_Jewelry",
	"$qlie_sortOption_Weapons",
	"$qlie_sortOption_Armors",
	"$qlie_sortOption_Clothes",
	"$qlie_sortOption_Weightless",
	"$qlie_sortOption_ByWeight",
	"$qlie_sortOption_ByValue",
	"$qlie_sortOption_ByName",
	"$qlie_sortOption_LOTD Needed",
	"$qlie_sortOption_CompletionistNeeded",
};
