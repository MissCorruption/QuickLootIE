#include "SystemSettings.h"

namespace QuickLoot::Config
{
	void SystemSettings::Update()
	{
		json config{};

		try {
			std::ifstream ifs;
			ifs.open(CONFIG_PATH, std::ios::in);

			if (!ifs.is_open()) {
				logger::info("No system settings file present");
				return;
			}

			ifs >> config;
		} catch (nlohmann::json::parse_error& error) {
			logger::error("Failed to parse system settings file: {}", error.what());
			return;
		}

		UpdateLogLevel(config);
		UpdateMenuWhitelist(config);
	}

	void SystemSettings::UpdateLogLevel(const json& config)
	{
		const auto logLevel = config.value("logLevel", "info");
		const auto flushLevel = config.value("logFlush", logLevel);

		spdlog::default_logger()->set_level(spdlog::level::from_str(logLevel));
		spdlog::default_logger()->flush_on(spdlog::level::from_str(flushLevel));
	}

	void SystemSettings::UpdateMenuWhitelist(const json& config)
	{
		_menuWhitelist.clear();

		if (!config.contains("menuWhitelist")) {
			return;
		}

		const auto whitelistJson = config.at("menuWhitelist");
		if (!whitelistJson.is_array()) {
			logger::warn("Menu whitelist wasn't an array");
			return;
		}

		for (auto& menuJson : whitelistJson) {
			if (!menuJson.is_string()) {
				logger::warn("Menu whitelist contained non-string value ({})", menuJson.type_name());
				continue;
			}

			const auto menu = menuJson.get<std::string>();
			_menuWhitelist.push_back(menu);
			logger::trace("Whitelisted menu: {}", menu);
		}
	}
}
