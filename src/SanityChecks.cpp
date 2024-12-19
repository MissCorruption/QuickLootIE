#include "SanityChecks.h"

#include "LootMenu.h"
#include "Config/SystemSettings.h"
#include "Util/FormUtil.h"

void ShowFatalError(const char* message)
{
	const int result = WIN32::MessageBoxA(nullptr, message, "QuickLoot IE", MB_YESNO);
	if (result == IDYES) {
		logger::info("Exiting game...");
		std::_Exit(EXIT_FAILURE);
	}
}

bool QuickLoot::SanityChecks::PerformChecks()
{
	return ValidatePlugins() && ValidateSWF();
}

bool QuickLoot::SanityChecks::ValidatePlugins()
{
	logger::info("Checking plugins...");

	if (!Util::FormUtil::FormExists("QuickLootIE.esp", 0x001)) {
		logger::error("MCM quest not found");

		ShowFatalError(
			"Unable to locate the QuickLoot MCM quest. "
			"Please make sure that QuickLootIE.esp is present and enabled."
			"\n\nExit Game now? (Recommend yes)");
		return false;
	}

	return true;
}

bool QuickLoot::SanityChecks::ValidateSWF()
{
	logger::info("Checking SWF files...");

	if (!Config::SystemSettings::SkipOldSwfCheck() && std::filesystem::exists(LEGACY_SWF)) {
		logger::error("LootMenu.swf present");

		ShowFatalError(
			"LootMenu.swf has been found. "
			"This file is no longer used by QuickLoot IE and was "
			"most likely provided by an incompatible UI patch. "
			"It will be ignored."
			"\n\nSet skipOldSwfCheck to true in QuickLootIE.json to disable this message."
			"\n\nExit Game now? (Recommend no)");

		// The presence of LootMenu.swf does not prevent QuickLoot from working,
		// so it shouldn't fail the sanity check.
		//return false;
	}

	switch (const int version = LootMenu::GetSwfVersion()) {
	case -2:
		if (!std::filesystem::exists(CURRENT_SWF)) {
			logger::error("LootMenuIE.swf missing");

			ShowFatalError(
				"LootMenuIE.swf is missing. "
				"This file is required by QuickLoot IE. "
				"Please make sure the mod is installed correctly."
				"\n\nExit Game now? (Recommend yes)");

			return false;
		}

		logger::error("Failed to load swf");

		ShowFatalError(
			"LootMenuIE.swf exists but failed to load. "
			"If you are using a reskin, make sure all of its dependencies are installed."
			"\n\nExit Game now? (Recommend yes)");

		return false;

	case -1:
		logger::error("Incompatible swf");

		ShowFatalError(
			"The installed version of LootMenuIE.swf is not compatible with QuickLoot IE."
			"\n\nExit Game now? (Recommend yes)");

		return false;

	default:
		if (version < LootMenu::CURRENT_MENU_VERSION) {
			logger::warn("LootMenuIE.swf reports version {}, but current loot menu version is {}", version, LootMenu::CURRENT_MENU_VERSION);
		}
		break;
	}

	return true;
}
