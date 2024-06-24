#pragma once

namespace QuickLoot {

	// Forward declaration of ScriptObjectPtr
	static ScriptObjectPtr MCMScript;

	// Forward declarations of Papyrus settings
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

	inline float QLIEWindowX;
	inline float QLIEWindowY;
	inline float QLIEWindowW;
	inline float QLIEWindowH;

	inline std::vector<std::string> QLIESortPriorityStrings;

	// Forward declaration of Papyrus class
	class Papyrus : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
	public:
		Papyrus(Papyrus const&) = delete;
		Papyrus(Papyrus const&&) = delete;
		Papyrus& operator=(Papyrus&) = delete;
		Papyrus& operator=(Papyrus&&) = delete;

		static Papyrus* GetSingleton() { static Papyrus singleton; return &singleton; }
		RE::BSEventNotifyControl ProcessEvent(RE::MenuOpenCloseEvent const* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* source) override;

		static void Init();
		static void SetFrameworkQuest(RE::StaticFunctionTag*, RE::TESQuest* a_quest);
		static void LogWithPlugin(RE::StaticFunctionTag*, std::string a_message);
		static void UpdateVariables(RE::StaticFunctionTag*);
		static std::string GetVersion(RE::StaticFunctionTag*);

		static std::vector<std::string> FormatSortOptionsList(RE::StaticFunctionTag*, std::vector<std::string> options, std::vector<std::string> userList);
		static std::vector<std::string> RemoveSortOptionPriority(RE::StaticFunctionTag*, std::vector<std::string> userList, int32_t elementPos);
		static std::vector<std::string> InsertSortOptionPriority(RE::StaticFunctionTag*, std::vector<std::string> userList, std::string newElement, int32_t elementPos);

		static std::vector<std::string> GetSortingPresets(RE::StaticFunctionTag*);
		static std::vector<std::string> GetSortingPreset(RE::StaticFunctionTag*, int32_t presetChoice);
		static std::vector<std::string> AddPresetsToArray(RE::StaticFunctionTag*, std::vector<std::string> userList, std::vector<std::string> presetList);

	private:
		Papyrus() = default;
		~Papyrus() = default;

		static bool RegisterFunctions(RE::BSScript::IVirtualMachine* a_vm);
		static std::string ReplaceStr(const std::string& in, const std::string& from, const std::string& to) {
			return std::regex_replace(in, std::regex(from), to);
		}

		static RE::BSScript::Variable* GetProperty(const std::string& a_prop) {
			if (!MCMScript || a_prop.empty()) {
				return nullptr;
			}

			if (auto* var = MCMScript->GetProperty(a_prop); var != nullptr) {
				return var;
			}

			logger::info("QuickLoot Papyrus: Unable To Get Property - [{}]", a_prop);
			return nullptr;
		}

		static std::vector<std::string> ConvertScriptArrayToVector(ScriptArrayPtr scriptArrayPtr) {
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
		static std::vector<std::string> ConvertArrayToVector(const std::array<const char*, N>& arr) {
			std::vector<std::string> vec;
			vec.reserve(N);

			for (size_t i = 0; i < N; ++i) {
				vec.push_back(arr[i]);
			}

			return vec;
		}

		template <typename T>
		static void LoadSetting(T& variable, const std::string& propertyName, const T& defaultValue) {
			variable = defaultValue;

			if constexpr (std::is_same_v<T, bool>) {
				if (const auto* prop = GetProperty(propertyName)) {
					variable = prop->GetBool();
				}
			}
			else if constexpr (std::is_same_v<T, std::string>) {
				if (const auto* prop = GetProperty(propertyName)) {
					variable = prop->GetString();
				}
			}
			else if constexpr (std::is_same_v<T, int32_t>) {
				if (const auto* prop = GetProperty(propertyName)) {
					variable = prop->GetSInt();
				}
			}
			else if constexpr (std::is_same_v<T, float>) {
				if (const auto* prop = GetProperty(propertyName)) {
					variable = prop->GetFloat();
				}
			}
			else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
				if (const auto* prop = GetProperty(propertyName)) {
					variable = ConvertScriptArrayToVector(prop->GetArray());
				}
			}
		}
	};
}

//These are just examples, we should localise these for the MCM e.g "The Goblin" would be "$qlie_preset_goblin"
constexpr std::array<const char*, 3> SortingPresets = {
	"Select Preset...", //Mandatory Entry
	"Default Preset",
	"The Goblin",
	//Add more as you see fit.
};

constexpr std::array<const char*, 5> SortingPresets_Default = {
	"Potions",
	"Armor",
	"Jewelry",
	"Weapons",
	"Gold",
};

constexpr std::array<const char*, 5> SortingPresets_Goblin = {
	"Gold",
	"Jewelry",
	"Potions",
	"Armor",
	"Weapons",
};
