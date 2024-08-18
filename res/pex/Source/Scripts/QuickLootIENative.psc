scriptname QuickLootIENative hidden native

; Gets the Current Plugin Version As A Full String e.g "1.0.1.2"
string function GetVersion() global native

; Sets the Quest Property in the dll so it can read script properties.
function SetFrameworkQuest(Quest a_quest) global native

; Syncs the DLL settings to the MCM.
function UpdateVariables() global native

; Allows print log messages from papyrus to the QuickLootIE Dll log file.
function LogWithPlugin(string a_log) global native

; Returns an array of sort options minus user defined options.
string[] function FormatSortOptionsList(string[] original_list, string[] user_defined_list) global native

; Returns a new array with the element at elementPosition removed.
string[] function RemoveSortOptionPriority(string[] user_defined_list, int elementPosition) global native

; Returns a new array with the element at elementPosition inserted.
string[] function InsertSortOptionPriority(string[] user_defined_list, string option, int elementPosition) global native

; Returns a string array for the selected preset.
string[] function GetSortingPreset(int selection) global native

; Returns a string array of all available preset names
string[] function GetSortingPresets() global native

; Returns a string array of all available preset names + given Names
string[] function AddPresetsToArray(string[] user_defined_list, string[] presetNames) global native
