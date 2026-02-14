#include "MergeMapperPluginAPI.h"
// Interface code based on https://github.com/adamhynek/higgs

// Stores the API after it has already been fetched
MergeMapperPluginAPI::IMergeMapperInterface001* g_mergeMapperInterface = nullptr;

// Fetches the interface to use from MergeMapper
MergeMapperPluginAPI::IMergeMapperInterface001* MergeMapperPluginAPI::GetMergeMapperInterface001() {
    // If the interface has already been fetched, return the same object
    if (g_mergeMapperInterface) {
        return g_mergeMapperInterface;
    }

    // Dispatch a message to get the plugin interface from MergeMapper
    MergeMapperMessage mergeMapperMessage;
    const auto skseMessaging = SKSE::GetMessagingInterface();
    skseMessaging->Dispatch(MergeMapperMessage::kMessage_GetInterface, (void*)&mergeMapperMessage,
                            sizeof(MergeMapperMessage*), MergeMapperPluginName);
    if (!mergeMapperMessage.GetApiFunction) {
        return nullptr;
    }

    // Fetch the API for this version of the MergeMapper interface
    g_mergeMapperInterface = static_cast<IMergeMapperInterface001*>(mergeMapperMessage.GetApiFunction(1));
    return g_mergeMapperInterface;
}
