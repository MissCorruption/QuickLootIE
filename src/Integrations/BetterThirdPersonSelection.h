#pragma once

namespace QuickLoot::Integrations
{
	class BetterThirdPersonSelection
	{
	public:
		static void Init()
		{
			_api = static_cast<API_V0*>(RequestPluginAPI(Version::V0));
		}

		static bool IsSelectionEnabled()
		{
			return _api && _api->GetSelectionEnabled();
		}

		static bool Is3DWidgetEnabled()
		{
			return _api && _api->GetWidget3DEnabled();
		}

		static bool HideSelectionWidget()
		{
			if (_api) {
				_api->HideSelectionWidget(std::string(Plugin::NAME));
			}

			return _api != nullptr;
		}

		static bool ShowSelectionWidget()
		{
			if (_api) {
				_api->ShowSelectionWidget(std::string(Plugin::NAME));
			}

			return _api != nullptr;
		}

	private:
		// BTPS API from https://gitlab.com/Shrimperator/skyrim-mod-betterthirdpersonselection/-/blob/3bcadfd9fc8424b891d26ec8230eb5aefb2a4541/src/API/BTPS_API_decl.h#L3

		enum class Version
		{
			V0,
		};

		class API_V0
		{
		public:
			virtual bool GetSelectionEnabled() noexcept = 0;
			virtual bool GetWidget3DEnabled() noexcept = 0;

			virtual void HideSelectionWidget(std::string source) noexcept = 0;
			virtual void ShowSelectionWidget(std::string source) noexcept = 0;
		};

		static inline API_V0* _api;
		static constexpr const char* _pluginName = "BetterThirdPersonSelection.dll";
		static [[nodiscard]] void* RequestPluginAPI(Version apiVersion = Version::V0)
		{
			HMODULE pluginHandle = GetModuleHandleA(_pluginName);
			if (!pluginHandle)
				return nullptr;

			typedef void* (*RequestPluginAPIProc)(Version apiVersion);
			if (const auto requestAPIFunction = reinterpret_cast<RequestPluginAPIProc>(GetProcAddress(pluginHandle, "RequestPluginAPI"))) {
				return requestAPIFunction(apiVersion);
			}

			return nullptr;
		}
	};
}
