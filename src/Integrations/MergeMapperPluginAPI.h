#pragma once
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
// Interface code based on https://github.com/adamhynek/higgs

namespace MergeMapperPluginAPI {
    constexpr const auto MergeMapperPluginName = "MergeMapper";
    // A message used to fetch MergeMapper's interface
    struct MergeMapperMessage {
        enum : uint32_t { kMessage_GetInterface = 0xe6cb8b59 };  // Randomly generated
        void* (*GetApiFunction)(unsigned int revisionNumber) = nullptr;
    };

    // Returns an IMergeMapperInterface001 object compatible with the API shown below
    // This should only be called after SKSE sends kMessage_PostLoad to your plugin
    struct IMergeMapperInterface001;
    IMergeMapperInterface001* GetMergeMapperInterface001();

    // This object provides access to MergeMapper's mod support API
    struct IMergeMapperInterface001 {
        // Gets the MergeMapper build number
        virtual unsigned int GetBuildNumber() = 0;

        /// @brief Get the new modName and formID
        /// @param oldName The original modName char* e.g., Dragonborn.esp
        /// @param oldFormID The original formID in hex format as an uint_32 e.g., 0x134ab
        /// @return a pair with char* modName and uint32 FormID. If no merge is found, it will return oldName and
        /// oldFormID.
        virtual std::pair<const char*, RE::FormID> GetNewFormID(const char* oldName, const RE::FormID oldFormID) = 0;

        /// @brief Get the original modName and formID
        /// @param newName The new merged modName char* e.g., Merge.esp
        /// @param newFormID The new merged formID in hex format as an uint_32 e.g., 0x134ab
        /// @return a pair with char* modName and uint32 FormID. If no merge is found, it will return newName and
        /// newFormID.
        virtual std::pair<const char*, RE::FormID> GetOriginalFormID(const char* newName,
                                                                     const RE::FormID newFormID) = 0;

        /// @brief Whether modName is a zmerge output file. To find old file use GetOldFormID(modName, 0)
        /// @param modName The modName to check, char* e.g., merged1.esp
        /// @return true if merged
        virtual bool isMerge(const char* modName) = 0;

        /// @brief Whether modName was an input to a zmerge file. To find new file use GetNewFormID(modName, 0)
        /// @param modName The modName to check, char* e.g., input1.esp
        /// @return true if was merged into some file
        virtual bool wasMerged(const char* modName) = 0;
    };

}  // namespace MergeMapperPluginAPI
extern MergeMapperPluginAPI::IMergeMapperInterface001* g_mergeMapperInterface;
