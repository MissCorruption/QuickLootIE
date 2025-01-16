#define DLLEXPORT __declspec(dllexport)

#include <spdlog/sinks/basic_file_sink.h>

#include "Behaviors/ActivationPrompt.h"
#include "Behaviors/LockpickActivation.h"
#include "Config/Papyrus.h"
#include "Config/SystemSettings.h"
#include "Input/InputManager.h"
#include "Input/InputObserver.h"
#include "Integrations/APIServer.h"
#include "Integrations/Artifacts.h"
#include "Integrations/Completionist.h"
#include "LootMenu.h"
#include "MenuVisibilityManager.h"
#include "SanityChecks.h"
#include "Util/Profiler.h"

void OnSKSEMessage(SKSE::MessagingInterface::Message* msg)
{
	switch (msg->type) {
	case SKSE::MessagingInterface::kPostLoad:
		{
			PROFILE_SCOPE_NAMED("SKSE Message (kPostLoad)");
			logger::info("--------------------------------[ kPostLoad start ]--------------------------------");

			QuickLoot::API::APIServer::Init(SKSE::GetMessagingInterface());
			logger::info("--------------------------------[ kPostLoad end ]--------------------------------");
			break;
		}

	case SKSE::MessagingInterface::kDataLoaded:
		{
			PROFILE_SCOPE_NAMED("SKSE Message (kDataLoaded)");
			logger::info("--------------------------------[ kDataLoaded start ]--------------------------------");

			QuickLoot::Config::SystemSettings::Update();

			if (!QuickLoot::SanityChecks::PerformChecks()) {
				logger::error("Sanity checks failed. Disabling QuickLootIE.");
				return;
			}

			QuickLoot::Config::Papyrus::Init();

			QuickLoot::Input::InputManager::Install();

			QuickLoot::LootMenu::Register();
			QuickLoot::MenuVisibilityManager::InstallHooks();

			QuickLoot::Behaviors::ActivationPrompt::Install();
			QuickLoot::Behaviors::LockpickActivation::Install();

			QuickLoot::Integrations::Artifacts::Init();
			QuickLoot::Integrations::Completionist::Init();

			QuickLoot::Input::InputObserver::StartListening();
			logger::info("--------------------------------[ kDataLoaded end ]--------------------------------");
			break;
		}
	}
}

void InitializeLog(spdlog::level::level_enum level = spdlog::level::info)
{
	auto path = logger::log_directory();
	if (!path) {
		SKSE::stl::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= std::format("{}.log"sv, Plugin::NAME);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
	auto log = std::make_shared<spdlog::logger>("global log", std::move(sink));

	log->set_level(level);
	log->flush_on(level);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%H:%M:%S.%e] [%t] [%l] [%s:%#] %v");
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() noexcept {
	SKSE::PluginVersionData v;
	v.PluginName(Plugin::NAME.data());
	v.PluginVersion(Plugin::VERSION);
	v.UsesAddressLibrary(true);
	v.HasNoStructUse();
	return v;
}();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo)
{
	pluginInfo->name = SKSEPlugin_Version.pluginName;
	pluginInfo->version = SKSEPlugin_Version.pluginVersion;
	pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
	return true;
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse)
{
	InitializeLog();

	logger::info("Loaded plugin {} {}", Plugin::NAME, Plugin::VERSION.string("."));

	QuickLoot::Config::SystemSettings::Update(true);
	QuickLoot::Util::Profiler::Init();

	PROFILE_SCOPE_NAMED("Plugin Startup");

	SKSE::Init(skse);
	SKSE::AllocTrampoline(1 << 8);

	return SKSE::GetMessagingInterface()->RegisterListener(OnSKSEMessage);
}
