scriptname QuickLootIEMCM extends SKI_ConfigBase conditional

; Script Imports
import QuickLootIENative
import StringUtil
import utility
import debug

; Maintenance Script
QuickLootIEMaintenance property QLIEMaintenance auto

; General Settings
bool property QLIECloseInCombat = false auto hidden
bool property QLIECloseWhenEmpty = true auto hidden
bool property QLIEDispelInvisibility = true auto hidden
bool property QLIEOpenWhenContainerUnlocked = true auto hidden
bool property QLIEDisableForAnimals = false auto hidden

; Icon Settings
bool property QLIEIconShowBookRead = true auto hidden
bool property QLIEIconShowStealing = true auto hidden
bool property QLIEIconShowEnchanted = true auto hidden
bool property QLIEIconShowKnownEnchanted = true auto hidden
bool property QLIEIconShowSpecialEnchanted = true auto hidden

; LOTD Settings
bool property QLIEIconShowDBMDisplayed = true auto hidden
bool property QLIEIconShowDBMFound = true auto hidden
bool property QLIEIconShowDBMNew = true auto hidden

; Completionist Settings
bool property QLIEShowCompNeeded = true auto hidden
bool property QLIEShowCompCollected = true auto hidden

; Window Settings
int property QLIEWindowX = 100 auto hidden
int property QLIEWindowY = -200 auto hidden
float property QLIEWindowScale = 1.0 auto hidden

int property QLIEMinLines = 0 auto hidden
int property QLIEMaxLines = 7 auto hidden

float property QLIETransparency_Normal = 1.0 auto hidden
float property QLIETransparency_Empty = 0.3 auto hidden

; Menu Anchor Options
string[] Anchor_Options
int anchor_options_state
int property QLIEAnchorOptionChoice = 0 auto hidden

; Sort priortiy for Loot Menu
string[] property user_selected_sort_options auto hidden
int property SortOptionsChoice = 0 auto hidden
bool property user_selected_sort_options_initialised = false auto hidden

string[] Sort_Options
string[] SortOptionName
string[] SortOptionHigh
string marked_sort_option

int[] SortOptionSlot
int SortOptionIndx
int sort_options_state
int OID_QLIE_SaveSortPreset

; Preset Selections
string[] sort_presets 	; Presets Menu
int sort_presets_state	; State
int property Sort_Presets_Choice = 0 auto hidden

; Controls if the preset should be loaded from the DLL or the JSON path.
int max_dll_preset_count

; Variables
bool AutoLoaded
bool lotd_installed
bool comp_installed

;---------------------------------------------------
;-- Events -----------------------------------------
;---------------------------------------------------

event OnConfigInit()
	if (!AutoLoaded)
		AutoLoadConfig()
	endif
endevent

;---------------------------------------------------
;-- Events -----------------------------------------
;---------------------------------------------------

event OnConfigOpen()
	UpdateCompatibilityVariables()
	marked_sort_option = ""
	
	pages = new string[4]
	pages[0] = "$qlie_MainMCMPage"
	pages[1] = "$qlie_WindMCMPage"
	; pages[2] = "$qlie_SortMCMPage"
	pages[2] = "$qlie_CompMCMPage"
endevent

;---------------------------------------------------
;-- Events -----------------------------------------
;---------------------------------------------------

event OnPageReset(string page)
    if (CurrentPage == "$qlie_MainMCMPage")
		BuildMainPage()
		return
	endif

    if (CurrentPage == "$qlie_WindMCMPage")
		BuildWindPage()
		return
	endif

    ; if (CurrentPage == "$qlie_SortMCMPage")
	; 	BuildSortPage()
	; 	return
	; endif

    if (CurrentPage == "$qlie_CompMCMPage")
		BuildCompPage()
		return
	endif
endevent

;---------------------------------------------------
;-- Events -----------------------------------------
;---------------------------------------------------

event OnOptionHighlight(int val)
	if (val == OID_QLIE_SaveSortPreset)
		SetInfoText("$qlie_sort_presets_state_save_info")
	else
		SetInfoText(GetSortOptionHighlight(val))
	endif
endevent

;---------------------------------------------------
;-- Events -----------------------------------------
;---------------------------------------------------

event OnOptionInputOpen(int val)
	if (val == OID_QLIE_SaveSortPreset)
		SetInputDialogStartText("$qlie_SaveSortPreset")
	endif
endevent

;---------------------------------------------------
;-- Events -----------------------------------------
;---------------------------------------------------

event OnOptionInputAccept(int val, string presetName)
	if (val == OID_QLIE_SaveSortPreset)
		SaveSortPreset(presetName)
	endif
endevent

;---------------------------------------------------
;-- Events -----------------------------------------
;---------------------------------------------------

event OnOptionSelect(int val)
	
	string name = GetSortOptionName(val)
	if name == ""
		return
	endif
	
	if marked_sort_option == name
		if ShowMessage("$qlie_sort_option_click", true, "$qlie_sort_option_unmark", "$qlie_sort_option_remove")
			UnMarkSortOptionPosition(val)
		else
			RemoveSortOption(val)
		endif
	else		
		if ShowMessage("$qlie_sort_option_click", true, "$qlie_sort_option_mark", "$qlie_sort_option_remove")
			MarkSortOptionPosition(val)
		else
			RemoveSortOption(val)
		endif
	endif
endevent
	
;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function UpdateCompatibilityVariables()
	lotd_installed = ((game.GetModByName("DBM_RelicNotifications.esp") != 255) as bool)
	comp_installed = ((game.GetModByName("Completionist.esp") != 255) as bool)
	
	if (!lotd_installed)
		QLIEIconShowDBMNew = false
		QLIEIconShowDBMFound = false
		QLIEIconShowDBMDisplayed = false
	endif

	if (!comp_installed)
		QLIEShowCompNeeded = false
		QLIEShowCompCollected = false
	endif
endfunction

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

string function GetEnabledStatus(bool bValue)
	if (bValue)
		return "$qlie_Enabled"
	endif

	return "$qlie_Disabled"
endfunction

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

string function GetCompatibilityEnabledStatus(bool bValue, bool bInstalled)
	
	if !bInstalled
		bValue = false
		return "$qlie_NotInstalled"
	endif
	
	if (bValue)
		return "$qlie_Enabled"
	endif

	return "$qlie_Disabled"
endfunction

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

string function GetConfigSaveString()
	
	if papyrusutil.GetScriptVersion() > 31
		return "$qlie_SavePreset"
	endif
	
	return "$qlie_PapUtilError"
endfunction	

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

string function GetConfigLoadString()

	if papyrusutil.GetScriptVersion() > 31
		return "$qlie_LoadPreset"
	endif
	
	return "$qlie_PapUtilError"
endfunction	

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function BuildSortOptions(bool forceReset = false)
	
	; Get Default List
	Sort_Options = GetSortingPreset(1)
	
	; Reset to default on first load or if reset button is pressed.
	if forceReset || !user_selected_sort_options_initialised
		user_selected_sort_options = Sort_Options
		user_selected_sort_options_initialised = true
	endif
	
	; Remove entries from Sort List if they are in the User List
	Sort_Options = FormatSortOptionsList(Sort_Options, user_selected_sort_options)
endfunction

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function BuildAnchorOptions()
	Anchor_Options = new string[9]
	Anchor_Options[0] = "$qlie_anchorOption_0"
	Anchor_Options[1] = "$qlie_anchorOption_1"
	Anchor_Options[2] = "$qlie_anchorOption_2"
	Anchor_Options[3] = "$qlie_anchorOption_3"
	Anchor_Options[4] = "$qlie_anchorOption_4"
	Anchor_Options[5] = "$qlie_anchorOption_5"
	Anchor_Options[6] = "$qlie_anchorOption_6"
	Anchor_Options[7] = "$qlie_anchorOption_7"
	Anchor_Options[8] = "$qlie_anchorOption_8"
endfunction

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function BuildPresetList()

	; Grab presets from the DLL.
	sort_presets = GetSortingPresets()
	max_dll_preset_count = sort_presets.Length

	; Grab custom presets from the JSON Path
	string[] custom_presets = jsonutil.JsonInFolder("../QuickLootIE/Profiles/SortPresets/")
	if custom_presets.Length > 0
		sort_presets = AddPresetsToArray(sort_presets, custom_presets)
	endif
endfunction

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function load_preset()

	if Sort_Presets_Choice > 0 
		if Sort_Presets_Choice <= max_dll_preset_count 
			user_selected_sort_options = GetSortingPreset(Sort_Presets_Choice)
		else
			user_selected_sort_options = jsonutil.PathStringElements("../QuickLootIE/Profiles/SortPresets/" + sort_presets[Sort_Presets_Choice], ".!QLIESortOrder")
		endif
	endif
	
	Sort_Presets_Choice = 0
	ForcePageReset()
endfunction

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

string function GetSortOptionName(int val)
	
	int Index = SortOptionSlot.Find(val)
	if (Index != -1)
		return SortOptionName[Index]
	endif
		
	return ""
endfunction

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

string function GetSortOptionHighlight(int val)
	
	int Index = SortOptionSlot.Find(val)
	if (Index != -1)
		return SortOptionHigh[Index]
	endif
		
	return ""
endfunction

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function SaveSortPreset(string presetName)
	
	if presetName == ""
		return
	endif
	
	if papyrusutil.GetScriptVersion() > 31
		jsonutil.SetPathStringArray("../QuickLootIE/Profiles/SortPresets/" + presetName, ".!QLIESortOrder", user_selected_sort_options, false)
		jsonutil.Save("../QuickLootIE/Profiles/SortPresets/" + presetName, false)
		ShowMessage("$qlie_PresetSaveSuccessMenu")
		ForcePageReset()
	else
		ShowMessage("$qlie_PapUtilError")
		ForcePageReset()
	endif
endfunction

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function RemoveSortOption(int val)
	
	int Index = SortOptionSlot.Find(val)
	string name = GetSortOptionName(val)
	
	if (Index != -1)
		if ShowMessage("$qlie_confirm_sort_option_removal{" + GetSortOptionName(val) + "}", true, "$qlie_ConfirmY", "$qlie_ConfirmN")
			
			if marked_sort_option == name
				marked_sort_option = ""
			endif
			
			user_selected_sort_options = RemoveSortOptionPriority(user_selected_sort_options, Index)
			SortOptionsChoice = 0
			ForcePageReset()
		endif
	endif
endfunction

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function UnMarkSortOptionPosition(int val)
	marked_sort_option = ""
	ForcePageReset()
endfunction

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function MarkSortOptionPosition(int val)
	int Index = SortOptionSlot.Find(val)
	if (Index != -1)
		marked_sort_option = SortOptionName[Index]
		ForcePageReset()
	endif
endfunction

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function InsertSortOption()
		
	if marked_sort_option == ""
		user_selected_sort_options = InsertSortOptionPriority(user_selected_sort_options, Sort_Options[SortOptionsChoice], user_selected_sort_options.Length)
	else
		int index = user_selected_sort_options.find(marked_sort_option)
	
		if (Index != -1)
			user_selected_sort_options = InsertSortOptionPriority(user_selected_sort_options, Sort_Options[SortOptionsChoice], Index)
			marked_sort_option = user_selected_sort_options[Min(user_selected_sort_options.Length, index + 1)]
		endif
	endif
	
	SortOptionsChoice = 0
	ForcePageReset()
endfunction

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

int function Min(int first, int second)

	if first < second
		return first
	endif

	return second
endfunction

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function BuildMainPage()
	SetCursorFillMode(TOP_TO_BOTTOM)
	SetCursorPosition(0)
		
	AddHeaderOption("$qlie_GeneralSettingsHeader", 0)
	AddTextOptionST("close_in_combat", 					"$qlie_close_in_combat_text", 				GetEnabledStatus(QLIECloseInCombat), 0)
	AddTextOptionST("close_when_empty", 				"$qlie_close_when_empty_text", 				GetEnabledStatus(QLIECloseWhenEmpty), 0)
	AddTextOptionST("dispel_invis", 					"$qlie_dispel_invis_text", 					GetEnabledStatus(QLIEDispelInvisibility), 0)
	AddTextOptionST("open_when_container_unlocked", 	"$qlie_open_when_container_unlocked_text", 	GetEnabledStatus(QLIEOpenWhenContainerUnlocked), 0)

	AddEmptyOption()
	AddHeaderOption("$qlie_ProfileHeader")
	AddTextOptionST("ProfileSave", 						"$qlie_ProfileText1", 			GetConfigSaveString(), 0)
	AddTextOptionST("ProfileLoad", 						"$qlie_ProfileText1", 			GetConfigLoadString(), 0)

	SetCursorPosition(1)
	AddHeaderOption("$qlie_ModInformationHeader")
	AddTextOption("", "$qlie_Author", 0)
	AddEmptyOption()
	AddTextOption("", "$qlie_ModVersion{ " + QLIEMaintenance.ModVersion + "}", 0)
	AddTextOption("", "$qlie_DLLVersion{ " + QuickLootIENative.GetVersion() + "}", 0)

	AddEmptyOption()
	AddHeaderOption("")
	AddTextOptionST("ProfileReset", 					"$qlie_ProfileText2", 			"$qlie_ProfileText3", 0)
endfunction

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function BuildWindPage()
	BuildAnchorOptions()
	SetCursorFillMode(TOP_TO_BOTTOM)
	
	SetCursorPosition(0)
	AddHeaderOption("$qlie_WindowSettingsHeader", 0)
	AddMenuOptionST("anchor_options_state", 				"$qlie_anchor_options_state_text",  Anchor_Options[QLIEAnchorOptionChoice], 0)
	AddSliderOptionST("window_X", 							"$qlie_window_X_text", QLIEWindowX, "{0}", 0)
	AddSliderOptionST("window_Y", 							"$qlie_window_Y_text", QLIEWindowY, "{0}", 0)

	AddEmptyOption()	
	AddHeaderOption("$qlie_MenuTransparencyHeader", 0)	
	AddSliderOptionST("transparency_normal", 				"$qlie_window_transparency_normal_text", 	QLIETransparency_Normal, "{1}", 0)
	
	
	AddEmptyOption()	
	AddHeaderOption("$qlie_IconSettingsHeader", 0)	
	AddTextOptionST("show_book_read_icon", 					"$qlie_show_book_read_icon_text", 			GetEnabledStatus(QLIEIconShowBookRead), 0)
	AddTextOptionST("show_stealing_icon", 					"$qlie_show_stealing_icon_text", 			GetEnabledStatus(QLIEIconShowStealing), 0)
	AddEmptyOption()

	SetCursorPosition(1)
	AddHeaderOption("")
	AddSliderOptionST("window_scale", 						"$qlie_window_scale_text", QLIEWindowScale, "{1}", 0)
	AddSliderOptionST("window_minLines", 					"$qlie_window_minLines_text", QLIEMinLines, "{0}", 0)
	AddSliderOptionST("window_maxLines", 					"$qlie_window_maxLines_text", QLIEMaxLines, "{0}", 0)

	AddEmptyOption()	
	AddHeaderOption("", 0)	
	AddSliderOptionST("transparency_empty", 				"$qlie_window_transparency_empty_text", 	QLIETransparency_Empty, "{1}", 0)
	
	AddEmptyOption()
	AddHeaderOption("")
	AddTextOptionST("show_enchanted_icon", 					"$qlie_show_enchanted_icon_text", 			GetEnabledStatus(QLIEIconShowEnchanted), 0)
	AddTextOptionST("show_knownenchanted_icon", 			"$qlie_show_knownenchanted_icon_text", 		GetEnabledStatus(QLIEIconShowKnownEnchanted), 0)
	AddTextOptionST("show_specialenchanted_icon", 			"$qlie_show_specialenchanted_icon_text", 	GetEnabledStatus(QLIEIconShowSpecialEnchanted), 0)
endfunction

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function BuildSortPage()
	BuildPresetList()
	BuildSortOptions()

	SortOptionIndx = 0
	SortOptionSlot = new int[128]
	SortOptionName = new string[128]
	SortOptionHigh = new string[128]
	
	SetCursorFillMode(TOP_TO_BOTTOM)
	SetCursorPosition(0)
	AddHeaderOption("$qlie_SortOptionsHeader", 0)
	AddMenuOptionST("sort_options_state", 			"$qlie_sort_options_state_text",   		Sort_Options[SortOptionsChoice], (!(Sort_Options.Length) as bool) as int)
	
	AddEMptyOption()
	if Sort_Options.Length == 0
		AddTextOptionST("sort_options_insert", 		"$qlie_sort_options_insert_none_text", 		"$qlie_sort_option_insert", 1)
	else
		AddTextOptionST("sort_options_insert", 		"$qlie_sort_options_insert_text{" + Sort_Options[SortOptionsChoice] + "}", 		"$qlie_sort_option_insert", 0)
	endif
	AddTextOptionST("sort_options_reset", 			"$qlie_sort_options_reset_text", "Reset", 0)
	
	AddEmptyOption()
	AddHeaderOption("$qlie_sortPresetsHeader", 0)
	AddMenuOptionST("sort_presets_state", 			"$qlie_sort_presets_state_text",   		sort_presets[Sort_Presets_Choice])
	OID_QLIE_SaveSortPreset = AddInputOption("$qlie_sort_presets_state_save_text", "$qlie_SaveSortPreset", 0)
	
	SetCursorPosition(1)
	AddHeaderOption("$qlie_SortPriorityHeader", 0)
	
	int idx = 0	
	while idx < user_selected_sort_options.Length
		if user_selected_sort_options[idx] != ""
			SortOptionName[SortOptionIndx] = user_selected_sort_options[idx]
			SortOptionHigh[SortOptionIndx] = "$qlie_sort_option_highlight_text"
			
			if SortOptionName[SortOptionIndx] == marked_sort_option
				SortOptionSlot[SortOptionIndx] = AddTextOption("$qlie_marked_sort_option_display{" + marked_sort_option + "}", "{M}")
			else
				SortOptionSlot[SortOptionIndx] = AddTextOption(user_selected_sort_options[idx], "", 0)
			endif

			SortOptionIndx += 1
		endif
		idx += 1
	endwhile
endfunction

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function BuildCompPage()
	SetCursorFillMode(TOP_TO_BOTTOM)
	
	SetCursorPosition(0)
	AddHeaderOption("$qlie_LOTDCompatHeader", 0)
	AddTextOptionST("show_lotd_new_icon", 			"$qlie_show_lotd_new_icon_text", 		GetCompatibilityEnabledStatus(QLIEIconShowDBMNew, lotd_installed), (!lotd_installed as bool) as int)
	AddTextOptionST("show_lotd_found_icon", 		"$qlie_show_lotd_found_icon_text", 		GetCompatibilityEnabledStatus(QLIEIconShowDBMFound, lotd_installed), (!lotd_installed as bool) as int)
	AddTextOptionST("show_lotd_disp_icon", 			"$qlie_show_lotd_disp_icon_text", 		GetCompatibilityEnabledStatus(QLIEIconShowDBMDisplayed, lotd_installed), (!lotd_installed as bool) as int)
	
	AddEmptyOption()
	AddHeaderOption("$qlie_MiscCompatHeader", 0)
	AddTextOptionST("disable_for_animals", 			"$qlie_disable_for_animals_text", 		GetEnabledStatus(QLIEDisableForAnimals), 0)
	
	SetCursorPosition(1)
	AddHeaderOption("$qlie_CompCompatHeader", 0)
	AddTextOptionST("show_comp_needed_icon", 		"$qlie_show_comp_needed_icon_text", 	GetCompatibilityEnabledStatus(QLIEShowCompNeeded, comp_installed), (!comp_installed as bool) as int)
	AddTextOptionST("show_comp_collected_icon", 	"$qlie_show_comp_collected_icon_text", 	GetCompatibilityEnabledStatus(QLIEShowCompCollected, comp_installed), (!comp_installed as bool) as int)
endfunction

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function Begin_Config_Save()
	if papyrusutil.GetScriptVersion() > 31
		jsonutil.SetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIECloseInCombat", QLIECloseInCombat as int)
		jsonutil.SetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIECloseWhenEmpty", QLIECloseWhenEmpty as int)
		jsonutil.SetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEDispelInvisibility", QLIEDispelInvisibility as int)
		jsonutil.SetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEOpenWhenContainerUnlocked", QLIEOpenWhenContainerUnlocked as int)
		jsonutil.SetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEDisableForAnimals", QLIEDisableForAnimals as int)
		
		jsonutil.SetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEIconShowBookRead", QLIEIconShowBookRead as int)
		jsonutil.SetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEIconShowStealing", QLIEIconShowStealing as int)
		jsonutil.SetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEIconShowEnchanted", QLIEIconShowEnchanted as int)
		jsonutil.SetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEIconShowKnownEnchanted", QLIEIconShowKnownEnchanted as int)
		jsonutil.SetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEIconShowSpecialEnchanted", QLIEIconShowSpecialEnchanted as int)
		
		jsonutil.SetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEIconShowDBMDisplayed", QLIEIconShowDBMDisplayed as int)
		jsonutil.SetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEIconShowDBMFound", QLIEIconShowDBMFound as int)
		jsonutil.SetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEIconShowDBMNew", QLIEIconShowDBMNew as int)
		
		jsonutil.SetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEShowCompNeeded", QLIEShowCompNeeded as int)
		jsonutil.SetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEShowCompCollected", QLIEShowCompCollected as int)
		
		jsonutil.SetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEAnchorOptionChoice", QLIEAnchorOptionChoice)
		jsonutil.SetPathFloatValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEWindowX", QLIEWindowX)
		jsonutil.SetPathFloatValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEWindowY", QLIEWindowY)
		jsonutil.SetPathFloatValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEWindowScale", QLIEWindowScale)
		
		jsonutil.SetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEMinLines", QLIEMinLines)
		jsonutil.SetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEMaxLines", QLIEMaxLines)

		jsonutil.SetPathFloatValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIETransparency_Normal", QLIETransparency_Normal)
		jsonutil.SetPathFloatValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIETransparency_Empty", QLIETransparency_Empty)
		
		jsonutil.Save("../QuickLootIE/Profiles/MCMConfig", false)
		ShowMessage("$qlie_ProfileSaveSuccessMenu")
		ForcePageReset()
	else
		ShowMessage("$qlie_PapUtilError")
		ForcePageReset()
	endif
endfunction

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function Begin_Config_Load()

	if papyrusutil.GetScriptVersion() > 31
		if jsonutil.JsonExists("../QuickLootIE/Profiles/MCMConfig")
			if !jsonutil.IsGood("../QuickLootIE/Profiles/MCMConfig")
				if IsInMenuMode()
					ShowMessage("$qlie_ProfileLoadDamaged{" + jsonutil.GetErrors("../QuickLootIE/Profiles/MCMConfig") + "}", false, "$qlie_ConfirmY", "$qlie_ConfirmN")
					return
				else
					Notification("$qlie_ProfileLoadCorrupt");
					Begin_Config_Default()
					return 
				endif
			endif
			
			QLIECloseInCombat = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIECloseInCombat", QLIECloseInCombat as int))
			QLIECloseWhenEmpty = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIECloseWhenEmpty", QLIECloseWhenEmpty as int))
			QLIEDispelInvisibility = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEDispelInvisibility", QLIEDispelInvisibility as int))
			QLIEOpenWhenContainerUnlocked = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEOpenWhenContainerUnlocked", QLIEOpenWhenContainerUnlocked as int))
			QLIEDisableForAnimals = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEDisableForAnimals", QLIEDisableForAnimals as int))
			
			QLIEIconShowBookRead = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEIconShowBookRead", QLIEIconShowBookRead as int))
			QLIEIconShowStealing = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEIconShowStealing", QLIEIconShowStealing as int))
			QLIEIconShowEnchanted = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEIconShowEnchanted", QLIEIconShowEnchanted as int))
			QLIEIconShowKnownEnchanted = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEIconShowKnownEnchanted", QLIEIconShowKnownEnchanted as int))
			QLIEIconShowSpecialEnchanted = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEIconShowSpecialEnchanted", QLIEIconShowSpecialEnchanted as int))
			
			QLIEIconShowDBMDisplayed = lotd_installed && (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEIconShowDBMDisplayed", QLIEIconShowDBMDisplayed as int))
			QLIEIconShowDBMFound = lotd_installed && (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEIconShowDBMFound", QLIEIconShowDBMFound as int))
			QLIEIconShowDBMNew = lotd_installed && (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEIconShowDBMNew", QLIEIconShowDBMNew as int))
			
			QLIEShowCompNeeded = comp_installed && (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEShowCompNeeded", QLIEShowCompNeeded as int))
			QLIEShowCompCollected = comp_installed && (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEShowCompCollected", QLIEShowCompCollected as int))			
			
			QLIEAnchorOptionChoice = jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEAnchorOptionChoice", QLIEAnchorOptionChoice)
			QLIEWindowX = jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEWindowX", QLIEWindowX)
			QLIEWindowY = jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEWindowY", QLIEWindowY)
			QLIEWindowScale = jsonutil.GetPathFloatValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEWindowScale", QLIEWindowScale)
			
			QLIEMinLines = jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEMinLines", QLIEMinLines)
			QLIEMaxLines = jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEMaxLines", QLIEMaxLines)
			
			QLIETransparency_Normal = jsonutil.GetPathFloatValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIETransparency_Normal", QLIETransparency_Normal)
			QLIETransparency_Empty = jsonutil.GetPathFloatValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIETransparency_Empty", QLIETransparency_Empty)
			
			jsonutil.Load("../QuickLootIE/Profiles/MCMConfig")
			if IsInMenuMode()
				ShowMessage("$qlie_ProfileLoadSuccessMenu")
				ForcePageReset()
			endif
		else
			if IsInMenuMode()
				ShowMessage("$qlie_ProfileLoadMissingMenu")
				ForcePageReset()
			else
				Begin_Config_Default()
			endif
		endif
	else
		if IsInMenuMode()
			ShowMessage("$qlie_PapUtilError")
			ForcePageReset()
		endif
	endif
endfunction	

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function AutoLoadConfig()
	UpdateCompatibilityVariables()
	
	if (papyrusutil.GetScriptVersion() > 31) && (jsonutil.JsonExists("../QuickLootIE/Profiles/MCMConfig")) && (jsonutil.IsGood("../QuickLootIE/Profiles/MCMConfig"))
		QLIECloseInCombat = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIECloseInCombat", QLIECloseInCombat as int))
		QLIECloseWhenEmpty = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIECloseWhenEmpty", QLIECloseWhenEmpty as int))
		QLIEDispelInvisibility = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEDispelInvisibility", QLIEDispelInvisibility as int))
		QLIEOpenWhenContainerUnlocked = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEOpenWhenContainerUnlocked", QLIEOpenWhenContainerUnlocked as int))
		QLIEDisableForAnimals = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEDisableForAnimals", QLIEDisableForAnimals as int))
		
		QLIEIconShowBookRead = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEIconShowBookRead", QLIEIconShowBookRead as int))
		QLIEIconShowStealing = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEIconShowStealing", QLIEIconShowStealing as int))
		QLIEIconShowEnchanted = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEIconShowEnchanted", QLIEIconShowEnchanted as int))
		QLIEIconShowKnownEnchanted = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEIconShowKnownEnchanted", QLIEIconShowKnownEnchanted as int))
		QLIEIconShowSpecialEnchanted = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEIconShowSpecialEnchanted", QLIEIconShowSpecialEnchanted as int))
		
		if (lotd_installed)
			QLIEIconShowDBMDisplayed = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEIconShowDBMDisplayed", QLIEIconShowDBMDisplayed as int))
			QLIEIconShowDBMFound = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEIconShowDBMFound", QLIEIconShowDBMFound as int))
			QLIEIconShowDBMNew = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEIconShowDBMNew", QLIEIconShowDBMNew as int))
		endif
		
		if (comp_installed)
			QLIEShowCompNeeded = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEShowCompNeeded", QLIEShowCompNeeded as int))
			QLIEShowCompCollected = (jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEShowCompCollected", QLIEShowCompCollected as int))
		endif
		
		QLIEAnchorOptionChoice = jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEAnchorOptionChoice", QLIEAnchorOptionChoice)
		QLIEWindowX = jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEWindowX", QLIEWindowX)
		QLIEWindowY = jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEWindowY", QLIEWindowY)
		QLIEWindowScale = jsonutil.GetPathFloatValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEWindowScale", QLIEWindowScale)

		QLIEMinLines = jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEMinLines", QLIEMinLines)
		QLIEMaxLines = jsonutil.GetPathIntValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIEMaxLines", QLIEMaxLines)

		QLIETransparency_Normal = jsonutil.GetPathFloatValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIETransparency_Normal", QLIETransparency_Normal)
		QLIETransparency_Empty = jsonutil.GetPathFloatValue("../QuickLootIE/Profiles/MCMConfig", ".!QLIETransparency_Empty", QLIETransparency_Empty)
			
		jsonutil.Load("../QuickLootIE/Profiles/MCMConfig")
		Notification("$qlie_ProfileLoadSuccess")
	else
		Begin_Config_Default()
		Notification("$qlie_ProfileLoadMissing")
	endif
	AutoLoaded = true
	UpdateVariables()
endfunction

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function Begin_Config_Default()

	QLIECloseInCombat = false
	QLIECloseWhenEmpty = true
	QLIEDispelInvisibility = true
	QLIEOpenWhenContainerUnlocked = true
	QLIEDisableForAnimals = false
	
	QLIEIconShowBookRead = true
	QLIEIconShowStealing = true
	QLIEIconShowEnchanted = true
	QLIEIconShowKnownEnchanted = true
	QLIEIconShowSpecialEnchanted = true
	
	QLIEIconShowDBMDisplayed = true
	QLIEIconShowDBMFound = true
	QLIEIconShowDBMNew = true
	
	QLIEShowCompNeeded = true
	QLIEShowCompCollected = true
	
	QLIEAnchorOptionChoice = 0
	QLIEWindowX = 100
	QLIEWindowY = -200
	QLIEWindowScale = 1.0

	QLIEMinLines = 0
	QLIEMaxLines = 7
	
	QLIETransparency_Normal = 1.0
	QLIETransparency_Empty = 0.3
	
	if IsInMenuMode()
		ForcePageReset()
	endif
endfunction

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state ProfileSave
	event OnSelectST()
		SetTitleText("$qlie_SaveProfileTitleText") 
		Begin_Config_Save()
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_SaveProfileTitleInfo")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state ProfileLoad
	event OnSelectST()
		SetTitleText("$qlie_LoadProfileTitleText") 
		Begin_Config_Load()
		UpdateVariables()
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_LoadProfileTitleInfo")
	endevent
endstate
;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state ProfileReset
	event OnSelectST()
		SetTitleText("$qlie_ResetProfileTitleText") 
		Begin_Config_Default()
		UpdateVariables()
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_ResetProfileTitleInfo")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state anchor_options_state

	event OnMenuOpenST()
		SetMenuDialogStartIndex(QLIEAnchorOptionChoice)
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(Anchor_Options)
	endevent
					
	event OnMenuAcceptST(int index)
		QLIEAnchorOptionChoice = Index
		SetMenuOptionValueST(anchor_options_state, Anchor_Options[QLIEAnchorOptionChoice])
		ForcePageReset()
	endevent

	event OnDefaultST()
		QLIEAnchorOptionChoice = 0
		SetMenuOptionValueST(anchor_options_state, Anchor_Options[QLIEAnchorOptionChoice])
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_anchor_options_state_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state sort_options_state

	event OnMenuOpenST()
		SetMenuDialogStartIndex(SortOptionsChoice)
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(Sort_Options)
	endevent
					
	event OnMenuAcceptST(int index)
		SortOptionsChoice = Index
		SetMenuOptionValueST(sort_options_state, Sort_Options[SortOptionsChoice])
		ForcePageReset()
	endevent

	event OnDefaultST()
		SortOptionsChoice = 0
		SetMenuOptionValueST(sort_options_state, Sort_Options[SortOptionsChoice])
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_sort_options_state_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state sort_presets_state

	event OnMenuOpenST()
		SetMenuDialogStartIndex(Sort_Presets_Choice)
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(sort_presets)
	endevent
					
	event OnMenuAcceptST(int index)
		Sort_Presets_Choice = Index
		
		if Sort_Presets_Choice > 0
			load_preset()
		endif

		SetMenuOptionValueST(sort_presets_state, sort_presets[Sort_Presets_Choice])
		ForcePageReset()
	endevent

	event OnDefaultST()
		Sort_Presets_Choice = 0
		BuildSortOptions(true)
		SetMenuOptionValueST(sort_presets_state, sort_presets[Sort_Presets_Choice])
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_sort_presets_state_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state sort_options_reset

	event OnSelectST()
		BuildSortOptions(true)
		ForcePageReset()
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_sort_options_reset_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state sort_options_insert
	event OnSelectST()
		InsertSortOption()
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_sort_options_insert_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state close_in_combat

	event OnSelectST()
		QLIECloseInCombat = !QLIECloseInCombat
		SetTextOptionValueST(GetEnabledStatus(QLIECloseInCombat))
	endevent
	
	event OnDefaultST()
		QLIECloseInCombat = true
		SetTextOptionValueST(GetEnabledStatus(QLIECloseInCombat))
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_close_in_combat_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state close_when_empty

	event OnSelectST()
		QLIECloseWhenEmpty = !QLIECloseWhenEmpty
		SetTextOptionValueST(GetEnabledStatus(QLIECloseWhenEmpty))
	endevent
	
	event OnDefaultST()
		QLIECloseWhenEmpty = true
		SetTextOptionValueST(GetEnabledStatus(QLIECloseWhenEmpty))
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_close_when_empty_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state dispel_invis

	event OnSelectST()
		QLIEDispelInvisibility = !QLIEDispelInvisibility
		SetTextOptionValueST(GetEnabledStatus(QLIEDispelInvisibility))
	endevent
	
	event OnDefaultST()
		QLIEDispelInvisibility = true
		SetTextOptionValueST(GetEnabledStatus(QLIEDispelInvisibility))
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_dispel_invis_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state open_when_container_unlocked

	event OnSelectST()
		QLIEOpenWhenContainerUnlocked = !QLIEOpenWhenContainerUnlocked
		SetTextOptionValueST(GetEnabledStatus(QLIEOpenWhenContainerUnlocked))
	endevent
	
	event OnDefaultST()
		QLIEOpenWhenContainerUnlocked = true
		SetTextOptionValueST(GetEnabledStatus(QLIEOpenWhenContainerUnlocked))
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_open_when_container_unlocked_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state disable_for_animals

	event OnSelectST()
		QLIEDisableForAnimals = !QLIEDisableForAnimals
		SetTextOptionValueST(GetEnabledStatus(QLIEDisableForAnimals))
	endevent
	
	event OnDefaultST()
		QLIEDisableForAnimals = true
		SetTextOptionValueST(GetEnabledStatus(QLIEDisableForAnimals))
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_disable_for_animals_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state window_X
	event OnSliderAcceptST(float value)
		QLIEWindowX = value as int
		self.SetSliderOptionValueST(value, "{0}", false, "")
    endevent

	event OnSliderOpenST()
		self.SetSliderDialogStartValue(QLIEWindowX as float)
		self.SetSliderDialogDefaultValue(100 as float)
		self.SetSliderDialogRange(-960 as float, 960 as float)
		self.SetSliderDialogInterval(1 as float)
	endevent

	event OnDefaultST()
		QLIEWindowX = 100
		self.SetSliderOptionValueST(QLIEWindowX as float, "{0}", false, "")
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_window_X_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state window_Y
	event OnSliderAcceptST(float value)
		QLIEWindowY = value as int
		self.SetSliderOptionValueST(value, "{0}", false, "")
    endevent

	event OnSliderOpenST()
		self.SetSliderDialogStartValue(QLIEWindowY as float)
		self.SetSliderDialogDefaultValue(-200 as float)
		self.SetSliderDialogRange(-540 as float, 540 as float)
		self.SetSliderDialogInterval(1 as float)
	endevent

	event OnDefaultST()
		QLIEWindowY = -200
		self.SetSliderOptionValueST(QLIEWindowY as float, "{0}", false, "")
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_window_Y_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state window_scale
	event OnSliderAcceptST(float value)
		QLIEWindowScale = value
		self.SetSliderOptionValueST(value, "{1}", false, "")
    endevent

	event OnSliderOpenST()
		self.SetSliderDialogStartValue(QLIEWindowScale)
		self.SetSliderDialogDefaultValue(1.0 as float)
		self.SetSliderDialogRange(0.1 as float, 3.0 as float)
		self.SetSliderDialogInterval(0.1 as float)
	endevent

	event OnDefaultST()
		QLIEWindowScale = 1.0
		self.SetSliderOptionValueST(QLIEWindowScale as float, "{1}", false, "")
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_window_scale_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state window_minLines
	event OnSliderAcceptST(float value)
		QLIEMinLines = value as int
		self.SetSliderOptionValueST(value, "{0}", false, "")

		if QLIEMinLines > QLIEMaxLines
			QLIEMaxLines = QLIEMinLines
			ForcePageReset()
		endif
    endevent

	event OnSliderOpenST()
		self.SetSliderDialogStartValue(QLIEMinLines)
		self.SetSliderDialogDefaultValue(0 as float)
		self.SetSliderDialogRange(0 as float, 25 as float)
		self.SetSliderDialogInterval(1 as float)
	endevent

	event OnDefaultST()
		QLIEMinLines = 0
		self.SetSliderOptionValueST(QLIEMinLines as float, "{0}", false, "")
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_window_minLines_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state window_maxLines
	event OnSliderAcceptST(float value)
		QLIEMaxLines = value as int
		self.SetSliderOptionValueST(value, "{0}", false, "")
		
		if QLIEMaxLines < QLIEMinLines
			QLIEMinLines = QLIEMaxLines
			ForcePageReset()
		endif
    endevent

	event OnSliderOpenST()
		self.SetSliderDialogStartValue(QLIEMaxLines)
		self.SetSliderDialogDefaultValue(7 as float)
		self.SetSliderDialogRange(1 as float, 25 as float)
		self.SetSliderDialogInterval(1 as float)
	endevent

	event OnDefaultST()
		QLIEMaxLines = 0
		self.SetSliderOptionValueST(QLIEMaxLines as float, "{0}", false, "")
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_window_maxLines_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state transparency_normal
	event OnSliderAcceptST(float value)
		QLIETransparency_Normal = value
		self.SetSliderOptionValueST(value, "{1}", false, "")
    endevent

	event OnSliderOpenST()
		self.SetSliderDialogStartValue(QLIETransparency_Normal)
		self.SetSliderDialogDefaultValue(1.0 as float)
		self.SetSliderDialogRange(0.1 as float, 1.0 as float)
		self.SetSliderDialogInterval(0.1 as float)
	endevent

	event OnDefaultST()
		QLIETransparency_Normal = 1.0
		self.SetSliderOptionValueST(QLIETransparency_Normal as float, "{1}", false, "")
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_window_transparency_normal_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state transparency_empty
	event OnSliderAcceptST(float value)
		QLIETransparency_Empty = value
		self.SetSliderOptionValueST(value, "{1}", false, "")
    endevent

	event OnSliderOpenST()
		self.SetSliderDialogStartValue(QLIETransparency_Empty)
		self.SetSliderDialogDefaultValue(0.3 as float)
		self.SetSliderDialogRange(0.1 as float, 1.0 as float)
		self.SetSliderDialogInterval(0.1 as float)
	endevent

	event OnDefaultST()
		QLIETransparency_Empty = 0.3
		self.SetSliderOptionValueST(QLIETransparency_Empty as float, "{1}", false, "")
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_window_transparency_empty_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state show_book_read_icon

	event OnSelectST()
		QLIEIconShowBookRead = !QLIEIconShowBookRead
		SetTextOptionValueST(GetEnabledStatus(QLIEIconShowBookRead))
	endevent
	
	event OnDefaultST()
		QLIEIconShowBookRead = true
		SetTextOptionValueST(GetEnabledStatus(QLIEIconShowBookRead))
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_show_book_read_icon_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state show_stealing_icon

	event OnSelectST()
		QLIEIconShowStealing = !QLIEIconShowStealing
		SetTextOptionValueST(GetEnabledStatus(QLIEIconShowStealing))
	endevent
	
	event OnDefaultST()
		QLIEIconShowStealing = true
		SetTextOptionValueST(GetEnabledStatus(QLIEIconShowStealing))
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_show_stealing_icon_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state show_enchanted_icon

	event OnSelectST()
		QLIEIconShowEnchanted = !QLIEIconShowEnchanted
		SetTextOptionValueST(GetEnabledStatus(QLIEIconShowEnchanted))
	endevent
	
	event OnDefaultST()
		QLIEIconShowEnchanted = true
		SetTextOptionValueST(GetEnabledStatus(QLIEIconShowEnchanted))
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_show_enchanted_icon_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state show_knownenchanted_icon

	event OnSelectST()
		QLIEIconShowKnownEnchanted = !QLIEIconShowKnownEnchanted
		SetTextOptionValueST(GetEnabledStatus(QLIEIconShowKnownEnchanted))
	endevent
	
	event OnDefaultST()
		QLIEIconShowKnownEnchanted = true
		SetTextOptionValueST(GetEnabledStatus(QLIEIconShowKnownEnchanted))
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_show_knownenchanted_icon_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state show_specialenchanted_icon

	event OnSelectST()
		QLIEIconShowSpecialEnchanted = !QLIEIconShowSpecialEnchanted
		SetTextOptionValueST(GetEnabledStatus(QLIEIconShowSpecialEnchanted))
	endevent
	
	event OnDefaultST()
		QLIEIconShowSpecialEnchanted = true
		SetTextOptionValueST(GetEnabledStatus(QLIEIconShowSpecialEnchanted))
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_show_specialenchanted_icon_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state show_lotd_disp_icon

	event OnSelectST()
		QLIEIconShowDBMDisplayed = !QLIEIconShowDBMDisplayed
		SetTextOptionValueST(GetEnabledStatus(QLIEIconShowDBMDisplayed))
	endevent
	
	event OnDefaultST()
		QLIEIconShowDBMDisplayed = true
		SetTextOptionValueST(GetEnabledStatus(QLIEIconShowDBMDisplayed))
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_show_lotd_disp_icon_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state show_lotd_found_icon

	event OnSelectST()
		QLIEIconShowDBMFound = !QLIEIconShowDBMFound
		SetTextOptionValueST(GetEnabledStatus(QLIEIconShowDBMFound))
	endevent
	
	event OnDefaultST()
		QLIEIconShowDBMFound = true
		SetTextOptionValueST(GetEnabledStatus(QLIEIconShowDBMFound))
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_show_lotd_found_icon_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state show_lotd_new_icon

	event OnSelectST()
		QLIEIconShowDBMNew = !QLIEIconShowDBMNew
		SetTextOptionValueST(GetEnabledStatus(QLIEIconShowDBMNew))
	endevent
	
	event OnDefaultST()
		QLIEIconShowDBMNew = true
		SetTextOptionValueST(GetEnabledStatus(QLIEIconShowDBMNew))
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_show_lotd_new_icon_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state show_comp_needed_icon

	event OnSelectST()
		QLIEShowCompNeeded = !QLIEShowCompNeeded
		SetTextOptionValueST(GetEnabledStatus(QLIEShowCompNeeded))
	endevent
	
	event OnDefaultST()
		QLIEShowCompNeeded = true
		SetTextOptionValueST(GetEnabledStatus(QLIEShowCompNeeded))
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_show_comp_needed_icon_info")
	endevent
endstate

;---------------------------------------------------
;-- States -----------------------------------------
;---------------------------------------------------

state show_comp_collected_icon

	event OnSelectST()
		QLIEShowCompCollected = !QLIEShowCompCollected
		SetTextOptionValueST(GetEnabledStatus(QLIEShowCompCollected))
	endevent
	
	event OnDefaultST()
		QLIEShowCompCollected = true
		SetTextOptionValueST(GetEnabledStatus(QLIEShowCompCollected))
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_show_comp_collected_icon_info")
	endevent
endstate
