#include "SystemSettings.h"

#include "Util/FormUtil.h"

namespace QuickLoot::Config
{
	std::vector<std::string> LoadStringArray(const json& config, const char* key)
	{
		std::vector<std::string> vec{};

		if (!config.contains(key)) {
			logger::warn("Config key {} does not exist", key);
			return vec;
		}

		const auto array = config.at(key);

		if (!array.is_array()) {
			logger::warn("Config key {} is not an array", key);
			return vec;
		}

		for (auto& element : array) {
			if (!element.is_string()) {
				logger::warn("Config key {} contains non-string elements", key);
				continue;
			}

			vec.push_back(element.get<std::string>());
		}

		return vec;
	}

	void SystemSettings::Update(bool minimal)
	{
		json config{};

		try {
			std::ifstream ifs;
			ifs.open(CONFIG_PATH, std::ios::in);

			if (!ifs.is_open()) {
				logger::info("No system settings file present");
				return;
			}

			config = json::parse(ifs, nullptr, true, true);
		} catch (nlohmann::json::parse_error& error) {
			logger::error("Failed to parse system settings file: {}", error.what());
			return;
		}

		_skipOldSwfCheck = config.value("skipOldSwfCheck", false);

		_enableProfiler = config.value("enableProfiler", false);
		_profilerFlushInterval = config.value("profilerFlushInterval", 1000);

		if (minimal) {
			return;
		}

		UpdateLogLevel(config);
		UpdateMenuWhitelist(config);
		UpdateContainerBlacklist(config);
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
		_menuWhitelist = LoadStringArray(config, "menuWhitelist");
	}

	void SystemSettings::UpdateContainerBlacklist(const json& config)
	{
		const auto blacklist = LoadStringArray(config, "containerBlacklist");

		for (auto identifier : blacklist) {
			const auto formId = Util::FormUtil::ParseFormID(identifier);
			_containerBlacklist.insert(formId);
		}
	}
}
