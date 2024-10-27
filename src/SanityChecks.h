#pragma once

#include "Util/FormUtil.h"

namespace QuickLoot
{
    class SanityChecks
    {
    public:
        SanityChecks() = delete;
        ~SanityChecks() = delete;
        SanityChecks(SanityChecks&&) = delete;
        SanityChecks(const SanityChecks&) = delete;
        SanityChecks& operator=(SanityChecks&&) = delete;
        SanityChecks& operator=(const SanityChecks&) = delete;

        static void PerformChecks()
        {
            logger::info("Starting sanity checks...");
            ValidateSWF();
            ValidatePlugins();
        }

    private:
        // Shockwave Flash Files
        static constexpr auto LEGACY_SWF = "Data\\Interface\\LootMenu.swf";
        static constexpr auto CURRENT_SWF = "Data\\Interface\\LootMenuIE.swf";

        static bool ShowMessageBoxAndExitIfYes(const char* message, const char* title)
        {
            int result = WIN32::MessageBoxA(nullptr, message, title, MB_YESNO);
            if (result == IDYES) {
                logger::info("Exiting game...");
                std::_Exit(EXIT_FAILURE);
            }
            return false;
            // Player decided to continue, disable QuickLoot IE entirely?
        }

        static void ValidateSWF()
        {
            logger::info("Checking SWF Files...");

            if (std::filesystem::exists(LEGACY_SWF))
            {
                logger::error("Old LootMenu found!");

                if (ShowMessageBoxAndExitIfYes(
                    "An incompatible LootMenu.swf has been found. This is usually due to patches not made for QuickLoot IE. Please uninstall any outdated patches.\n\nExit Game now? (Recommend yes)",
                    "QuickLoot IE Sanity Checks")) 
                {
                    return;
                }
            }

            if (std::filesystem::exists(CURRENT_SWF)) {
                logger::info("Correct LootMenu found!");
                // TODO:
                // Check if version exists in the .swf
                // if not exist OR lower than required, inform user for update
            }
        }

        static bool ValidatePlugins()
        {
            logger::info("Checking plugins...");

            if (!Util::FormUtil::FormExists("QuickLootIE.esp", 0x805)) {
                logger::error("QuickLoot IE MCM not found!");

                if (ShowMessageBoxAndExitIfYes(
                    "The QuickLoot IE MCM could not be loaded! This is usually due to plugins being disabled. Check if QuickLootIE.esp is present and active.\n\nExit Game now? (Recommend yes)",
                    "QuickLoot IE Sanity Checks"))
                {
                    return false;
                }
            }

            return true;
        }
    };
}
