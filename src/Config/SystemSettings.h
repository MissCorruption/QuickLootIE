#pragma once

namespace QuickLoot::Config
{
	class SystemSettings
	{
	public:
		SystemSettings() = delete;
		~SystemSettings() = delete;
		SystemSettings(SystemSettings const&) = delete;
		SystemSettings(SystemSettings const&&) = delete;
		SystemSettings operator=(SystemSettings&) = delete;
		SystemSettings operator=(SystemSettings&&) = delete;

		static void Update(bool minimal = false);
		static void UpdateLogLevel(const json& config);
		static void UpdateMenuWhitelist(const json& config);
		static void UpdateContainerBlacklist(const json& config);

		static bool SkipOldSwfCheck() { return _skipOldSwfCheck; }
		static bool EnableProfiler() { return _enableProfiler; }
		static int ProfilerFlushInterval() { return _profilerFlushInterval; }
		static bool EnableMenuCaching() { return _enableMenuCaching; }
		static const std::vector<std::string>& GetMenuWhitelist() { return _menuWhitelist; }
		static const std::set<RE::FormID>& GetContainerBlacklist() { return _containerBlacklist; }

	private:
		static constexpr auto CONFIG_PATH = "Data\\SKSE\\Plugins\\QuickLootIE.json";

		static inline bool _skipOldSwfCheck = false;
		static inline bool _enableProfiler = false;
		static inline int _profilerFlushInterval = false;
		static inline bool _enableMenuCaching = true;
		static inline std::vector<std::string> _menuWhitelist{};
		static inline std::set<RE::FormID> _containerBlacklist{};
	};
}
