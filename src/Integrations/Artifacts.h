#pragma once

#include "RE/B/BSCoreTypes.h"

namespace QuickLoot::Integrations
{
	class Artifacts
	{
	private:
		Artifacts() = default;
		~Artifacts() = default;

		static constexpr const char* TCC_PLUGIN_NAME = "DBM_RelicNotifications.esp";
		static constexpr const char* ETR_PLUGIN_NAME = "Artifact Tracker.esp";
		
		static inline bool _isReady;

		static inline RE::BGSListForm* _new = nullptr;
		static inline RE::BGSListForm* _found = nullptr;
		static inline RE::BGSListForm* _displayed = nullptr;

	public:
		Artifacts(Artifacts const&) = delete;
		Artifacts(Artifacts const&&) = delete;
		Artifacts operator=(Artifacts&) = delete;
		Artifacts operator=(Artifacts&&) = delete;

		static void Init()
		{
			_isReady = TryLoadForms(TCC_PLUGIN_NAME, 0x558285, 0x558286, 0x558287) || TryLoadForms(ETR_PLUGIN_NAME, 0x800, 0x801, 0x802);
		}

		static bool TryLoadForms(const char* plugin, RE::FormID newId, RE::FormID foundId, RE::FormID displayedId)
		{
			const auto dataHandler = RE::TESDataHandler::GetSingleton();

			if (dataHandler->LookupLoadedModByName(plugin) == nullptr &&
				dataHandler->LookupLoadedLightModByName(plugin) == nullptr) {
				SKSE::log::info("Plugin {} not found", plugin);
				return false;
			}

			_new = dataHandler->LookupForm<RE::BGSListForm>(newId, plugin);
			_found = dataHandler->LookupForm<RE::BGSListForm>(foundId, plugin);
			_displayed = dataHandler->LookupForm<RE::BGSListForm>(displayedId, plugin);

			if (_new && _found && _displayed) {
				logger::info("Located form lists in plugin {}", plugin);
				return true;
			}

			logger::info("Failed to locate form lists in plugin {}", plugin);
			return false;
		}

		static bool IsIntegrationEnabled()
		{
			return _isReady;
		}

		static bool IsNewArtifact(RE::FormID formID)
		{
			return _new && _new->HasForm(formID);
		}

		static bool IsFoundArtifact(RE::FormID formID)
		{
			return _found && _found->HasForm(formID);
		}

		static bool IsDisplayedArtifact(RE::FormID formID)
		{
			return _displayed && _displayed->HasForm(formID);
		}
	};
}
