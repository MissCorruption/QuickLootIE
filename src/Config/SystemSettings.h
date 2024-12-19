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

		static void Update();
		static void UpdateLogLevel(const json& config);
		static void UpdateMenuWhitelist(const json& config);
		
		static const std::vector<std::string>& GetMenuWhitelist() { return _menuWhitelist; }

	private:
		static constexpr auto CONFIG_PATH = "Data\\SKSE\\Plugins\\QuickLootIE.json";

		static inline std::vector<std::string> _menuWhitelist{};
	};
}
