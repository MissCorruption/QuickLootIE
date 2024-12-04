scriptname QuickLootIENative hidden native

; Gets the current plugin version as a full string e.g "1.0.1.2"
string function GetDllVersion() global native

; Gets the reported version of LootMenuIE.swf
string function GetSwfVersion() global native

; Sets the quest property in the dll so it can read script properties.
function SetFrameworkQuest(Quest quest) global native

; Syncs the dll settings to the MCM.
function UpdateVariables() global native

; Allows print log messages from Papyrus to the QuickLootIE dll log file.
function LogWithPlugin(string log) global native

; Returns an array of sort options minus user defined options.
string[] function FormatSortOptionsList(string[] originalList, string[] userDefinedList) global native

; Returns a new array with the element at elementPosition removed.
string[] function RemoveSortOptionPriority(string[] userDefinedList, int elementPosition) global native

; Returns a new array with the element at elementPosition inserted.
string[] function InsertSortOptionPriority(string[] userDefinedList, string option, int elementPosition) global native

; Returns a string array for the selected preset.
string[] function GetSortingPreset(int selection) global native

; Returns a string array of all available preset names
string[] function GetSortingPresets() global native

; Returns a string array of all available preset names + given names
string[] function AddPresetsToArray(string[] userDefinedList, string[] presetNames) global native
