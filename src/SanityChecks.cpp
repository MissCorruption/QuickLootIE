#include "SanityChecks.h"

#include "LootMenu.h"
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
	return ValidateEsp() & ValidateSwf();
}

bool QuickLoot::SanityChecks::ValidateEsp()
{
	if (!Util::FormUtil::FormExists("QuickLootIE.esp", 0x001)) {
		logger::error("QuickLootIE.esp is not loaded");
		return false;
	}

	return true;
}

bool QuickLoot::SanityChecks::ValidateSwf()
{
	if (std::filesystem::exists(LEGACY_SWF)) {
		logger::warn("LootMenu.swf present");

		// The presence of LootMenu.swf does not prevent QuickLoot from working,
		// so it shouldn't fail the sanity check.
	}

	switch (const int version = LootMenu::GetSwfVersion()) {
	case -2:
		if (!std::filesystem::exists(CURRENT_SWF)) {
			logger::error("LootMenuIE.swf missing");

			ShowFatalError(
				"LootMenuIE.swf is missing. "
				"This file is required by QuickLoot IE. "
				"Please make sure the mod is installed correctly."
				"\n\nExit game now? (Recommend yes)");

			return false;
		}

		logger::error("Failed to load swf");

		ShowFatalError(
			"LootMenuIE.swf exists but failed to load. "
			"If you are using a reskin, make sure all of its dependencies are installed."
			"\n\nExit game now? (Recommend yes)");

		return false;

	case -1:
		logger::error("Incompatible swf");

		ShowFatalError(
			"The installed version of LootMenuIE.swf is not compatible with QuickLoot IE."
			"\n\nExit game now? (Recommend yes)");

		return false;

	default:
		if (version < LootMenu::CURRENT_MENU_VERSION) {
			logger::warn("LootMenuIE.swf reports version {}, but current loot menu version is {}", version, LootMenu::CURRENT_MENU_VERSION);
		}
		break;
	}

	return true;
}
