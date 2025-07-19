#include <spdlog/sinks/basic_file_sink.h>

#include "Behaviors/ActivationPrompt.h"
#include "Behaviors/LockpickActivation.h"
#include "Config/Papyrus.h"
#include "Config/SystemSettings.h"
#include "Input/InputManager.h"
#include "Input/InputObserver.h"
#include "Integrations/Artifacts.h"
#include "Integrations/BetterThirdPersonSelection.h"
#include "Integrations/Completionist.h"
#include "Integrations/DismemberingFramework.h"
#include "LootMenu.h"
#include "Observers/CameraStateObserver.h"
#include "Observers/CombatStateObserver.h"
#include "Observers/ContainerObserver.h"
#include "Observers/CrosshairRefObserver.h"
#include "Observers/LifeStateObserver.h"
#include "Observers/LockChangedObserver.h"
#include "Observers/MenuObserver.h"
#include "SanityChecks.h"
#include "Util/Profiler.h"

static void OnSKSEMessage(SKSE::MessagingInterface::Message* msg)
{
	switch (msg->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		{
			PROFILE_SCOPE_NAMED("SKSE Message (kDataLoaded)");
			logger::info("--------------------------------[ kDataLoaded start ]--------------------------------");

			QuickLoot::Config::SystemSettings::Update();

			if (!QuickLoot::SanityChecks::PerformChecks()) {
				logger::error("Sanity checks failed. Disabling QuickLootIE.");
				return;
			}

			QuickLoot::LootMenu::Register();

			QuickLoot::Config::Papyrus::Install();

			QuickLoot::Input::InputManager::Install();
			QuickLoot::Input::InputObserver::Install();

			QuickLoot::Observers::CameraStateObserver::Install();
			QuickLoot::Observers::CombatStateObserver::Install();
			QuickLoot::Observers::ContainerObserver::Install();
			QuickLoot::Observers::CrosshairRefObserver::Install();
			QuickLoot::Observers::LifeStateObserver::Install();
			QuickLoot::Observers::LockChangedObserver::Install();
			QuickLoot::Observers::MenuObserver::Install();

			QuickLoot::Behaviors::ActivationPrompt::Install();
			QuickLoot::Behaviors::LockpickActivation::Install();

			QuickLoot::Integrations::Artifacts::Init();
			QuickLoot::Integrations::BetterThirdPersonSelection::Init();
			QuickLoot::Integrations::Completionist::Init();
			QuickLoot::Integrations::DismemberingFramework::Init();

			logger::info("--------------------------------[ kDataLoaded end ]--------------------------------");
			break;
		}

	default:
		break;
	}
}

static void InitializeLog(spdlog::level::level_enum level = spdlog::level::info)
{
	auto path = logger::log_directory();
	if (!path) {
		SKSE::stl::report_and_fail("Failed to find standard logging directory"sv);
	}

	const auto plugin = SKSE::PluginDeclaration::GetSingleton();

	*path /= std::format("{}.log"sv, plugin->GetName());
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
	auto log = std::make_shared<spdlog::logger>("global log", std::move(sink));

	log->set_level(level);
	log->flush_on(level);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%H:%M:%S.%e] [%l] [%t] %v");
}

extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse)
{
	InitializeLog();

	const auto plugin = SKSE::PluginDeclaration::GetSingleton();

	logger::info("{} {}", plugin->GetName(), plugin->GetVersion().string("."));

	QuickLoot::Config::SystemSettings::Update(true);
	QuickLoot::Util::Profiler::Init();

	PROFILE_SCOPE_NAMED("Plugin Startup");

	SKSE::Init(skse, false);
	SKSE::AllocTrampoline(1 << 8);

	return SKSE::GetMessagingInterface()->RegisterListener(OnSKSEMessage);
}

#ifdef IS_CMAKE_BUILD

// xmake automatically generates this boilerplate code, but for cmake builds we need to include it manually.

#	include "Plugin.h"

extern "C" [[maybe_unused]] __declspec(dllexport) constinit auto SKSEPlugin_Version = []() noexcept {
	SKSE::PluginVersionData v{};
	v.PluginName(Plugin::NAME.data());
	v.PluginVersion(Plugin::VERSION);
	v.UsesAddressLibrary();
	v.UsesNoStructs();
	return v;
}();

extern "C" [[maybe_unused]] __declspec(dllexport) bool SKSEPlugin_Query(SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo)
{
	pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
	pluginInfo->name = SKSEPlugin_Version.pluginName;
	pluginInfo->version = SKSEPlugin_Version.pluginVersion;
	return true;
}

#endif
