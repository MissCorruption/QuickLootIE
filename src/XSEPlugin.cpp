#define DLLEXPORT __declspec(dllexport)

#include <spdlog/sinks/basic_file_sink.h>

#include "Behaviors/ActivationPrompt.h"
#include "Behaviors/LockpickActivation.h"
#include "Config/Papyrus.h"
#include "Input/InputManager.h"
#include "Integrations/APIServer.h"
#include "Integrations/Completionist.h"
#include "Integrations/Artifacts.h"
#include "LootMenu.h"
#include "MenuVisibilityManager.h"
#include "SanityChecks.h"

void OnSKSEMessage(SKSE::MessagingInterface::Message* msg)
{
	switch (msg->type) {
	case SKSE::MessagingInterface::kPostLoad:
		QuickLoot::API::APIServer::Init(SKSE::GetMessagingInterface());
		break;

	case SKSE::MessagingInterface::kDataLoaded:
		if (!QuickLoot::SanityChecks::PerformChecks()) {
			logger::error("Sanity checks failed. Disabling QuickLootIE.");
			return;
		}

		QuickLoot::Config::Papyrus::Init();

		// Do this after settings are loaded
		QuickLoot::Input::InputManager::Install();

		QuickLoot::LootMenu::Register();
		QuickLoot::MenuVisibilityManager::InstallHooks();

		QuickLoot::Behaviors::ActivationPrompt::Install();
		QuickLoot::Behaviors::LockpickActivation::Install();

		QuickLoot::Integrations::Artifacts::Init();
		QuickLoot::Integrations::Completionist::Init();
		break;
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
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] [%s:%#] %v");
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
	InitializeLog(spdlog::level::trace);

	logger::info("Loaded plugin {} {}", Plugin::NAME, Plugin::VERSION.string());

	SKSE::Init(skse);
	SKSE::AllocTrampoline(1 << 8);

	return SKSE::GetMessagingInterface()->RegisterListener(OnSKSEMessage);
}
