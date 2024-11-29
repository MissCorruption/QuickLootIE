scriptname QuickLootIEMCM extends SKI_ConfigBase conditional

import QuickLootIENative
import PapyrusUtil
import StringUtil
import Utility
import Debug

;---------------------------------------------------
;-- Properties and Fields --------------------------
;---------------------------------------------------

bool AutoLoadedProfile = false

string ConfigPath = "../QuickLootIE/DefaultConfig"
string SortPresetPath = "../QuickLootIE/SortPresets/"
string ControlPresetPath = "../QuickLootIE/ControlPresets/"

; General > Behavior Settings
bool property QLIE_ShowInCombat = true auto hidden
bool property QLIE_ShowWhenEmpty = false auto hidden
bool property QLIE_ShowWhenUnlocked = true auto hidden
bool property QLIE_ShowInThirdPerson = true auto hidden
bool property QLIE_ShowWhenMounted = false auto hidden
bool property QLIE_EnableForAnimals = true auto hidden
bool property QLIE_EnableForDragons = true auto hidden
bool property QLIE_BreakInvisibility = true auto hidden

; Display > Window Settings
int property QLIE_WindowOffsetX = 100 auto hidden
int property QLIE_WindowOffsetY = -200 auto hidden
float property QLIE_WindowScale = 1.0 auto hidden
int property QLIE_WindowAnchor = 0 auto hidden
int property QLIE_WindowMinLines = 0 auto hidden
int property QLIE_WindowMaxLines = 7 auto hidden
float property QLIE_WindowOpacityNormal = 1.0 auto hidden
float property QLIE_WindowOpacityEmpty = 0.3 auto hidden
string[] WindowAnchorNames

; Display > Icon Settings
bool property QLIE_ShowIconRead = true auto hidden
bool property QLIE_ShowIconStolen = true auto hidden
bool property QLIE_ShowIconEnchanted = true auto hidden
bool property QLIE_ShowIconEnchantedKnown = true auto hidden
bool property QLIE_ShowIconEnchantedSpecial = true auto hidden

; Display > Info Columns
string[] property QLIE_InfoColumns auto hidden
int InfoColumnPresetIndex = 2
string[] InfoColumnPresetNames
string[] InfoColumnPresetStrings

; Sorting
string[] property QLIE_SortRulesActive auto hidden
string[] SortRulesAvailable			; Options available for insertion
int[] SortRulesActiveIds			; Option IDs for each index in the active list
int SortSelectedRuleIndex = -1		; Index of the selected option in the active list
string[] SortPresetNames			; Load preset dropdown values
int SortPredefinedPresetCount		; How many presets are defined in the dll

; Controls
int property QLIE_KeybindingTake = 18 auto hidden
int property QLIE_KeybindingTakeAll = 19 auto hidden
int property QLIE_KeybindingTransfer = 16 auto hidden
int property QLIE_KeybindingTakeModifier = 0 auto hidden
int property QLIE_KeybindingTakeAllModifier = 0 auto hidden
int property QLIE_KeybindingTransferModifier = 0 auto hidden
int property QLIE_KeybindingTakeGamepad = 276 auto hidden
int property QLIE_KeybindingTakeAllGamepad = 278 auto hidden
int property QLIE_KeybindingTransferGamepad = 271 auto hidden
string[] KeyModifierOptions
string[] ControlPresetNames
int ControlPredefinedPresetCount
bool GamepadMode

; Compatibility > LOTD Icons
bool property QLIE_ShowIconLOTDNew = true auto hidden
bool property QLIE_ShowIconLOTDCarried = true auto hidden
bool property QLIE_ShowIconLOTDDisplayed = true auto hidden

; Compatibility > Completionist Icons
bool property QLIE_ShowIconCompletionistNeeded = true auto hidden
bool property QLIE_ShowIconCompletionistCollected = true auto hidden

;---------------------------------------------------
;-- SkyUI Events -----------------------------------
;---------------------------------------------------

event OnConfigInit()
	AutoLoadConfig()
endevent

event OnConfigOpen()
	Initialize()

	Pages = new string[5]
	Pages[0] = "$qlie_GeneralPage"
	Pages[1] = "$qlie_DisplayPage"
	Pages[2] = "$qlie_SortingPage"
	Pages[3] = "$qlie_ControlsPage"
	Pages[4] = "$qlie_CompatibilityPage"

	SendModEvent("SKICP_pageSelected", "$qlie_GeneralPage", 0) ; Select Geleral page
endevent

event OnPageReset(string page)
    if (page == "$qlie_GeneralPage")
		BuildGeneralPage()
		return
	endif

    if (page == "$qlie_DisplayPage")
		BuildDisplayPage()
		return
	endif

    if (page == "$qlie_SortingPage")
		BuildSortingPage()
		return
	endif

    if (page == "$qlie_ControlsPage")
		BuildControlsPage()
		return
	endif

    if (page == "$qlie_CompatibilityPage")
		BuildCompatibilityPage()
		return
	endif
endevent

;---------------------------------------------------
;-- Initialization ---------------------------------
;---------------------------------------------------

function Initialize()
	InitWindowAnchorNames()
	InitInfoColumnPresetData()
	InitSortPresetList()
	InitSortRuleLists()
	InitModifierOptions()
	InitControlPresets()

	GamepadMode = Game.UsingGamepad()
endfunction

function InitWindowAnchorNames()
	WindowAnchorNames = new string[9]
	WindowAnchorNames[0] = "$qlie_WindowAnchor_name0"
	WindowAnchorNames[1] = "$qlie_WindowAnchor_name1"
	WindowAnchorNames[2] = "$qlie_WindowAnchor_name2"
	WindowAnchorNames[3] = "$qlie_WindowAnchor_name3"
	WindowAnchorNames[4] = "$qlie_WindowAnchor_name4"
	WindowAnchorNames[5] = "$qlie_WindowAnchor_name5"
	WindowAnchorNames[6] = "$qlie_WindowAnchor_name6"
	WindowAnchorNames[7] = "$qlie_WindowAnchor_name7"
	WindowAnchorNames[8] = "$qlie_WindowAnchor_name8"
endfunction

function InitInfoColumnPresetData()
	InfoColumnPresetNames = new string[4]
	InfoColumnPresetNames[0] = "$qlie_InfoColumnPreset_v_w_vpw"
	InfoColumnPresetNames[1] = "$qlie_InfoColumnPreset_v_vpw_w"
	InfoColumnPresetNames[2] = "$qlie_InfoColumnPreset_v_w"
	InfoColumnPresetNames[3] = "$qlie_InfoColumnPreset_none"

	InfoColumnPresetStrings = new string[4]
	InfoColumnPresetStrings[0] = "value,weight,valuePerWeight"
	InfoColumnPresetStrings[1] = "value,valuePerWeight,weight"
	InfoColumnPresetStrings[2] = "value,weight"
	InfoColumnPresetStrings[3] = ""
endfunction

function InitSortPresetList()

	; Grab presets from the DLL.
	SortPresetNames = GetSortingPresets()
	SortPredefinedPresetCount = SortPresetNames.Length

	; Grab custom presets from the JSON Path
	string[] custom_presets = JsonUtil.JsonInFolder(SortPresetPath)
	if custom_presets.Length > 0
		SortPresetNames = AddPresetsToArray(SortPresetNames, custom_presets)
	endif
endfunction

function InitSortRuleLists(bool forceReset = false)
	; Get Default List
	SortRulesAvailable = GetSortingPreset(1)

	; Reset to default on first load or if reset button is pressed.
	if forceReset || QLIE_SortRulesActive.Length == 0
		QLIE_SortRulesActive = SortRulesAvailable
	endif

	; Remove entries from available list if they are in the active list
	SortRulesAvailable = FormatSortOptionsList(SortRulesAvailable, QLIE_SortRulesActive)
	SortRulesActiveIds = Utility.CreateIntArray(QLIE_SortRulesActive.Length, -1)
endfunction

function InitModifierOptions()
	KeyModifierOptions = new string[5]
	KeyModifierOptions[0] = "$qlie_ModifierKey_ignore"
	KeyModifierOptions[1] = "$qlie_ModifierKey_none"
	KeyModifierOptions[2] = "$qlie_ModifierKey_shift"
	KeyModifierOptions[3] = "$qlie_ModifierKey_control"
	KeyModifierOptions[4] = "$qlie_ModifierKey_alt"
endfunction

function InitControlPresets()
	ControlPresetNames = new string[2]
	ControlPresetNames[0] = "Default (E, R, Q)"
	ControlPresetNames[1] = "Fallout 4 Style (E, Shift+E, R)"
	ControlPredefinedPresetCount = ControlPresetNames.Length

	ControlPresetNames = MergeStringArray(ControlPresetNames, JsonUtil.JsonInFolder(ControlPresetPath))
endfunction

;---------------------------------------------------
;-- Events -----------------------------------------
;---------------------------------------------------

event OnHighlightST()
	string currentState = GetState()

	if Substring(currentState, 0, 6) != "state_"
		SetInfoText("")
		return
	endif

	SetInfoText("$qlie_" + Substring(currentState, 6) + "_info")
endevent

event OnOptionHighlight(int optionID)
	; Check whether the selection option is one of the dynamically generated sort options
	int index = SortRulesActiveIds.Find(optionID)
	if index < 0 || index >= QLIE_SortRulesActive.Length
		return
	endif

	SetInfoText("$qlie_SortRule_info")
endevent

event OnOptionSelect(int optionID)
	; Check whether the selection option is one of the dynamically generated sort options
	int index = SortRulesActiveIds.Find(optionID)
	if index < 0 || index >= QLIE_SortRulesActive.Length
		return
	endif

	if index == SortSelectedRuleIndex
		SortSelectedRuleIndex = -1
	else
		SortSelectedRuleIndex = index
	endif

	ForcePageReset()
endevent

;---------------------------------------------------
;-- Pages ------------------------------------------
;---------------------------------------------------

function BuildGeneralPage()
	SetCursorFillMode(TOP_TO_BOTTOM)

	SetCursorPosition(0)
	AddHeaderOption("$qlie_BehaviorSettingsHeader")
	AddTextOptionST("state_ShowInCombat",			"$qlie_ShowInCombat_text",			GetEnabledStatusText(QLIE_ShowInCombat))
	AddTextOptionST("state_ShowWhenEmpty",			"$qlie_ShowWhenEmpty_text",			GetEnabledStatusText(QLIE_ShowWhenEmpty))
	AddTextOptionST("state_ShowWhenUnlocked",		"$qlie_ShowWhenUnlocked_text",		GetEnabledStatusText(QLIE_ShowWhenUnlocked))
	AddTextOptionST("state_ShowInThirdPerson",		"$qlie_ShowInThirdPerson_text",		GetEnabledStatusText(QLIE_ShowInThirdPerson))
	AddTextOptionST("state_ShowWhenMounted",		"$qlie_ShowWhenMounted_text",		GetEnabledStatusText(QLIE_ShowWhenMounted))
	AddTextOptionST("state_EnableForAnimals",		"$qlie_EnableForAnimals_text",		GetEnabledStatusText(QLIE_EnableForAnimals))
	AddTextOptionST("state_EnableForDragons",		"$qlie_EnableForDragons_text",		GetEnabledStatusText(QLIE_EnableForDragons))
	AddTextOptionST("state_BreakInvisibility",		"$qlie_BreakInvisibility_text",		GetEnabledStatusText(QLIE_BreakInvisibility))

	SetCursorPosition(1)
	AddHeaderOption("$qlie_ModInformationHeader")
	AddTextOption("", "$qlie_ModName")
	;AddTextOption("", "$qlie_Author1")
	;AddTextOption("", "$qlie_Author2")
	AddEmptyOption()
	AddTextOption("", "$qlie_ModVersion{" + ((self as Quest) as QuickLootIEMaintenance).CurrentVersionString + "}")
	AddTextOption("", "$qlie_DllVersion{" + QuickLootIENative.GetVersion() + "}")
	AddEmptyOption()

	if PapyrusUtil.GetScriptVersion() > 31
		AddHeaderOption("$qlie_ProfileActionsHeader")
		AddTextOptionST("state_ProfileReset",	"", "$qlie_ProfileReset_text")
		AddTextOptionST("state_ProfileSave",	"", "$qlie_ProfileSave_text")
		AddTextOptionST("state_ProfileLoad",	"", "$qlie_ProfileLoad_text")
	else
		AddHeaderOption("$qlie_ProfileActionsHeader")
		AddTextOptionST("state_ProfileReset",	"", "$qlie_ProfileReset_text")
		AddTextOptionST("state_ProfileSave",	"", "$qlie_ProfileSave_unavailable", OPTION_FLAG_DISABLED)
		AddTextOptionST("state_ProfileLoad",	"", "$qlie_ProfileLoad_unavailable", OPTION_FLAG_DISABLED)
	endif
endfunction

function BuildDisplayPage()
	SetCursorFillMode(TOP_TO_BOTTOM)

	SetCursorPosition(0)
	AddHeaderOption("$qlie_WindowSettingsHeader")
	AddMenuOptionST("state_WindowAnchor",				"$qlie_WindowAnchor_text",				WindowAnchorNames[QLIE_WindowAnchor])
	AddSliderOptionST("state_WindowOffsetX",			"$qlie_WindowOffsetX_text",				QLIE_WindowOffsetX, "{0}")
	AddSliderOptionST("state_WindowOffsetY",			"$qlie_WindowOffsetY_text",				QLIE_WindowOffsetY, "{0}")
	AddSliderOptionST("state_WindowScale",				"$qlie_WindowScale_text",				QLIE_WindowScale, "{1}")
	AddSliderOptionST("state_WindowOpacityNormal",		"$qlie_WindowOpacityNormal_text",		QLIE_WindowOpacityNormal, "{1}")
	AddSliderOptionST("state_WindowOpacityEmpty",		"$qlie_WindowOpacityEmpty_text",		QLIE_WindowOpacityEmpty, "{1}")
	AddSliderOptionST("state_WindowMinLines",			"$qlie_WindowMinLines_text",			QLIE_WindowMinLines, "{0}")
	AddSliderOptionST("state_WindowMaxLines",			"$qlie_WindowMaxLines_text",			QLIE_WindowMaxLines, "{0}")

	SetCursorPosition(1)
	AddHeaderOption("$qlie_IconSettingsHeader")
	AddTextOptionST("state_ShowIconRead",				"$qlie_ShowIconRead_text",				GetEnabledStatusText(QLIE_ShowIconRead))
	AddTextOptionST("state_ShowIconStolen",				"$qlie_ShowIconStolen_text",			GetEnabledStatusText(QLIE_ShowIconStolen))
	AddTextOptionST("state_ShowIconEnchanted",			"$qlie_ShowIconEnchanted_text",			GetEnabledStatusText(QLIE_ShowIconEnchanted))
	AddTextOptionST("state_ShowIconEnchantedKnown",		"$qlie_ShowIconEnchantedKnown_text",	GetEnabledStatusText(QLIE_ShowIconEnchantedKnown))
	AddTextOptionST("state_ShowIconEnchantedSpecial",	"$qlie_ShowIconEnchantedSpecial_text",	GetEnabledStatusText(QLIE_ShowIconEnchantedSpecial))

	AddEmptyOption()
	AddHeaderOption("$qlie_InfoColumnLayoutHeader")
	if InfoColumnPresetIndex < 0
		AddMenuOptionST("state_InfoColumnPreset",		"$qlie_InfoColumnPreset_text",			"$qlie_InfoColumnPreset_custom")
	else
		AddMenuOptionST("state_InfoColumnPreset",		"$qlie_InfoColumnString_text",			InfoColumnPresetNames[InfoColumnPresetIndex])
	endif
	AddInputOptionST("state_InfoColumnString",			"$qlie_InfoColumnString_text",			"$qlie_InfoColumnString_button")
endfunction

function BuildSortingPage()
	SetCursorFillMode(TOP_TO_BOTTOM)

	SetCursorPosition(0)
	AddHeaderOption("$qlie_SortRulesHeader")

	; Dynamically generate a list of options and save their ids in SortRulesActiveIds.
	int i = 0
	while i < QLIE_SortRulesActive.Length
		if i == SortSelectedRuleIndex
			SortRulesActiveIds[i] = AddTextOption("$qlie_SortRule_selected{" + QLIE_SortRulesActive[i] + "}", "&lt;&lt;&lt;")
		else
			SortRulesActiveIds[i] = AddTextOption(QLIE_SortRulesActive[i], "")
		endif

		i += 1
	endwhile

	SetCursorPosition(1)
	AddHeaderOption("$qlie_SortOptionsHeader")
	if SortSelectedRuleIndex >= 0 && SortSelectedRuleIndex < QLIE_SortRulesActive.Length
		string selectedRuleName = QLIE_SortRulesActive[SortSelectedRuleIndex]
		AddMenuOptionST("state_SortInsert",		"", "$qlie_SortInsert_text{" + selectedRuleName + "}", (SortRulesAvailable.Length == 0) as int)
		AddTextOptionST("state_SortRemove",		"", "$qlie_SortRemove_text{" + selectedRuleName + "}")
	else
		AddMenuOptionST("state_SortInsert",		"", "$qlie_SortInsert_text", (SortRulesAvailable.Length == 0) as int)
		AddTextOptionST("state_SortRemove",		"", "$qlie_SortRemove_text", OPTION_FLAG_DISABLED)
	endif

	SetCursorPosition(13)
	AddHeaderOption("$qlie_SortPresetsHeader")
	AddTextOptionST("state_SortReset",			"", "$qlie_SortReset_text")
	AddInputOptionST("state_SortPresetSave", 	"", "$qlie_SortPresetSave_text")
	AddMenuOptionST("state_SortPresetLoad",		"", "$qlie_SortPresetLoad_text")
endfunction

function BuildControlsPage()
	SetCursorFillMode(LEFT_TO_RIGHT)

	SetCursorPosition(0)
	AddHeaderOption("$qlie_KeybindingsHeader")
	AddHeaderOption("")

	if GamepadMode
		AddKeyMapOptionST("state_ControlsTake",				"$qlie_ControlsTake_text", QLIE_KeybindingTakeGamepad)
		AddEmptyOption()
		; AddMenuOptionST("state_ControlsTakeModifier",		"$qlie_ControlsModifier_text", KeyModifierOptions[0], OPTION_FLAG_DISABLED)
		AddKeyMapOptionST("state_ControlsTakeAll",			"$qlie_ControlsTakeAll_text", QLIE_KeybindingTakeAllGamepad)
		AddEmptyOption()
		; AddMenuOptionST("state_ControlsTakeAllModifier",	"$qlie_ControlsModifier_text", KeyModifierOptions[0], OPTION_FLAG_DISABLED)
		AddKeyMapOptionST("state_ControlsTransfer",			"$qlie_ControlsTransfer_text", QLIE_KeybindingTransferGamepad)
		AddEmptyOption()
		; AddMenuOptionST("state_ControlsTransferModifier",	"$qlie_ControlsModifier_text", KeyModifierOptions[0], OPTION_FLAG_DISABLED)
	else
		AddKeyMapOptionST("state_ControlsTake",				"$qlie_ControlsTake_text", QLIE_KeybindingTake)
		AddMenuOptionST("state_ControlsTakeModifier",		"$qlie_ControlsModifier_text", KeyModifierOptions[QLIE_KeybindingTakeModifier])
		AddKeyMapOptionST("state_ControlsTakeAll",			"$qlie_ControlsTakeAll_text", QLIE_KeybindingTakeAll)
		AddMenuOptionST("state_ControlsTakeAllModifier",	"$qlie_ControlsModifier_text", KeyModifierOptions[QLIE_KeybindingTakeAllModifier])
		AddKeyMapOptionST("state_ControlsTransfer",			"$qlie_ControlsTransfer_text", QLIE_KeybindingTransfer)
		AddMenuOptionST("state_ControlsTransferModifier",	"$qlie_ControlsModifier_text", KeyModifierOptions[QLIE_KeybindingTransferModifier])
	endif

	SetCursorFillMode(TOP_TO_BOTTOM)
	SetCursorPosition(13)
	AddHeaderOption("$qlie_ControlPresetsHeader")
	AddTextOptionST("state_ControlReset",				"", "$qlie_ControlReset_text")

	if PapyrusUtil.GetScriptVersion() > 31
		AddInputOptionST("state_ControlPresetSave",		"", "$qlie_ControlPresetSave_text")
		AddMenuOptionST("state_ControlPresetLoad",		"", "$qlie_ControlPresetLoad_text")
	else
		AddInputOptionST("state_ControlPresetSave",		"", "$qlie_ControlPresetSave_text", OPTION_FLAG_DISABLED)
		AddMenuOptionST("state_ControlPresetLoad",		"", "$qlie_ControlPresetLoad_text", OPTION_FLAG_DISABLED)
	endif
endfunction

function BuildCompatibilityPage()
	SetCursorFillMode(TOP_TO_BOTTOM)

	bool hasLOTD = Game.GetModByName("DBM_RelicNotifications.esp") != 255
	bool hasCompletionist = Game.GetModByName("Completionist.esp") != 255

	SetCursorPosition(0)
	AddHeaderOption("$qlie_LOTDIconsHeader")
	AddTextOptionST("state_ShowIconLOTDNew",				"$qlie_ShowIconLOTDNew_text",					GetEnabledStatusText(QLIE_ShowIconLOTDNew, hasLOTD), (!hasLOTD) as int)
	AddTextOptionST("state_ShowIconLOTDCarried",			"$qlie_ShowIconLOTDCarried_text",				GetEnabledStatusText(QLIE_ShowIconLOTDCarried, hasLOTD), (!hasLOTD) as int)
	AddTextOptionST("state_ShowIconLOTDDisplayed",			"$qlie_ShowIconLOTDDisplayed_text",				GetEnabledStatusText(QLIE_ShowIconLOTDDisplayed, hasLOTD), (!hasLOTD) as int)

	SetCursorPosition(1)
	AddHeaderOption("$qlie_CompletionistIconsHeader")
	AddTextOptionST("state_ShowIconCompletionistNeeded",	"$qlie_ShowIconCompletionistNeeded_text",		GetEnabledStatusText(QLIE_ShowIconCompletionistNeeded, hasCompletionist), (!hasCompletionist) as int)
	AddTextOptionST("state_ShowIconCompletionistCollected",	"$qlie_ShowIconCompletionistCollected_text",	GetEnabledStatusText(QLIE_ShowIconCompletionistCollected, hasCompletionist), (!hasCompletionist) as int)
endfunction

;---------------------------------------------------
;-- Helper Functions -------------------------------
;---------------------------------------------------

string function GetEnabledStatusText(bool enabled, bool installed = true)
	if !installed
		return "$qlie_NotInstalled"
	endif

	if enabled
		return "$qlie_Enabled"
	endif

	return "$qlie_Disabled"
endfunction

function ShowMsg(string message)
	ShowMessage(message, false, "$qlie_ConfirmY", "$qlie_ConfirmN")
endfunction

;---------------------------------------------------
;-- General > Profile Actions ----------------------
;---------------------------------------------------

state state_ProfileReset
	event OnSelectST()
		SetTextOptionValueST("$qlie_ProfileReset_inprogress")
		ResetSettings()
		SetTextOptionValueST("$qlie_ProfileReset_text")
	endevent
endstate

state state_ProfileSave
	event OnSelectST()
		SetTextOptionValueST("$qlie_ProfileSave_inprogress")
		SaveProfile()
		SetTextOptionValueST("$qlie_ProfileSave_text")
	endevent
endstate

state state_ProfileLoad
	event OnSelectST()
		SetTextOptionValueST("$qlie_ProfileLoad_inprogress")
		LoadProfile()
		SetTextOptionValueST("$qlie_ProfileLoad_text")
	endevent
endstate

;---------------------------------------------------
;-- General > Behavior Settings --------------------
;---------------------------------------------------

state state_ShowInCombat
	event OnSelectST()
		QLIE_ShowInCombat = !QLIE_ShowInCombat
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowInCombat))
	endevent

	event OnDefaultST()
		QLIE_ShowInCombat = true
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowInCombat))
	endevent
endstate

state state_ShowWhenEmpty
	event OnSelectST()
		QLIE_ShowWhenEmpty = !QLIE_ShowWhenEmpty
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowWhenEmpty))
	endevent

	event OnDefaultST()
		QLIE_ShowWhenEmpty = false
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowWhenEmpty))
	endevent
endstate

state state_ShowWhenUnlocked

	event OnSelectST()
		QLIE_ShowWhenUnlocked = !QLIE_ShowWhenUnlocked
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowWhenUnlocked))
	endevent

	event OnDefaultST()
		QLIE_ShowWhenUnlocked = true
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowWhenUnlocked))
	endevent
endstate

state state_ShowInThirdPerson
	event OnSelectST()
		QLIE_ShowInThirdPerson = !QLIE_ShowInThirdPerson
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowInThirdPerson))
	endevent

	event OnDefaultST()
		QLIE_ShowInThirdPerson = true
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowInThirdPerson))
	endevent
endstate

state state_ShowWhenMounted
	event OnSelectST()
		QLIE_ShowWhenMounted = !QLIE_ShowWhenMounted
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowWhenMounted))
	endevent

	event OnDefaultST()
		QLIE_ShowWhenMounted = false
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowWhenMounted))
	endevent
endstate

state state_EnableForAnimals
	event OnSelectST()
		QLIE_EnableForAnimals = !QLIE_EnableForAnimals
		SetTextOptionValueST(GetEnabledStatusText(QLIE_EnableForAnimals))
	endevent

	event OnDefaultST()
		QLIE_EnableForAnimals = true
		SetTextOptionValueST(GetEnabledStatusText(QLIE_EnableForAnimals))
	endevent
endstate

state state_EnableForDragons
	event OnSelectST()
		QLIE_EnableForDragons = !QLIE_EnableForDragons
		SetTextOptionValueST(GetEnabledStatusText(QLIE_EnableForDragons))
	endevent

	event OnDefaultST()
		QLIE_EnableForDragons = true
		SetTextOptionValueST(GetEnabledStatusText(QLIE_EnableForDragons))
	endevent
endstate

state state_BreakInvisibility
	event OnSelectST()
		QLIE_BreakInvisibility = !QLIE_BreakInvisibility
		SetTextOptionValueST(GetEnabledStatusText(QLIE_BreakInvisibility))
	endevent

	event OnDefaultST()
		QLIE_BreakInvisibility = true
		SetTextOptionValueST(GetEnabledStatusText(QLIE_BreakInvisibility))
	endevent
endstate

;---------------------------------------------------
;-- Display > Window Settings ----------------------
;---------------------------------------------------

state state_WindowOffsetX
	event OnSliderAcceptST(float value)
		QLIE_WindowOffsetX = value as int
		SetSliderOptionValueST(value)
    endevent

	event OnSliderOpenST()
		SetSliderDialogStartValue(QLIE_WindowOffsetX)
		SetSliderDialogDefaultValue(100)
		SetSliderDialogRange(-960, 960)
		SetSliderDialogInterval(1)
	endevent

	event OnDefaultST()
		QLIE_WindowOffsetX = 100
		SetSliderOptionValueST(QLIE_WindowOffsetX)
	endevent
endstate

state state_WindowOffsetY
	event OnSliderAcceptST(float value)
		QLIE_WindowOffsetY = value as int
		SetSliderOptionValueST(value)
    endevent

	event OnSliderOpenST()
		SetSliderDialogStartValue(QLIE_WindowOffsetY)
		SetSliderDialogDefaultValue(-200)
		SetSliderDialogRange(-540, 540)
		SetSliderDialogInterval(1)
	endevent

	event OnDefaultST()
		QLIE_WindowOffsetY = -200
		SetSliderOptionValueST(QLIE_WindowOffsetY)
	endevent
endstate

state state_WindowScale
	event OnSliderAcceptST(float value)
		QLIE_WindowScale = value
		SetSliderOptionValueST(value, "{1}")
    endevent

	event OnSliderOpenST()
		SetSliderDialogStartValue(QLIE_WindowScale)
		SetSliderDialogDefaultValue(1.0)
		SetSliderDialogRange(0.1, 3.0)
		SetSliderDialogInterval(0.1)
	endevent

	event OnDefaultST()
		QLIE_WindowScale = 1.0
		SetSliderOptionValueST(QLIE_WindowScale, "{1}")
	endevent
endstate

state state_WindowAnchor
	event OnMenuOpenST()
		SetMenuDialogStartIndex(QLIE_WindowAnchor)
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(WindowAnchorNames)
	endevent

	event OnMenuAcceptST(int index)
		QLIE_WindowAnchor = index
		SetMenuOptionValueST(WindowAnchorNames[QLIE_WindowAnchor])
	endevent

	event OnDefaultST()
		QLIE_WindowAnchor = 0
		SetMenuOptionValueST(WindowAnchorNames[QLIE_WindowAnchor])
	endevent
endstate

state state_WindowMinLines
	event OnSliderAcceptST(float value)
		QLIE_WindowMinLines = value as int
		SetSliderOptionValueST(value)

		if QLIE_WindowMinLines > QLIE_WindowMaxLines
			QLIE_WindowMaxLines = QLIE_WindowMinLines
			SetSliderOptionValueST(QLIE_WindowMaxLines, "{0}", false, "state_WindowMaxLines")
		endif
    endevent

	event OnSliderOpenST()
		SetSliderDialogStartValue(QLIE_WindowMinLines)
		SetSliderDialogDefaultValue(0)
		SetSliderDialogRange(0, 25)
		SetSliderDialogInterval(1)
	endevent

	event OnDefaultST()
		QLIE_WindowMinLines = 0
		SetSliderOptionValueST(QLIE_WindowMinLines)
	endevent
endstate

state state_WindowMaxLines
	event OnSliderAcceptST(float value)
		QLIE_WindowMaxLines = value as int
		SetSliderOptionValueST(value)

		if QLIE_WindowMaxLines < QLIE_WindowMinLines
			QLIE_WindowMinLines = QLIE_WindowMaxLines
			SetSliderOptionValueST(QLIE_WindowMinLines, "{0}", false, "state_WindowMinLines")
		endif
    endevent

	event OnSliderOpenST()
		SetSliderDialogStartValue(QLIE_WindowMaxLines)
		SetSliderDialogDefaultValue(7)
		SetSliderDialogRange(1, 25)
		SetSliderDialogInterval(1)
	endevent

	event OnDefaultST()
		QLIE_WindowMaxLines = 7
		SetSliderOptionValueST(QLIE_WindowMaxLines)
	endevent
endstate

state state_WindowOpacityNormal
	event OnSliderAcceptST(float value)
		QLIE_WindowOpacityNormal = value
		SetSliderOptionValueST(value, "{1}")
    endevent

	event OnSliderOpenST()
		SetSliderDialogStartValue(QLIE_WindowOpacityNormal)
		SetSliderDialogDefaultValue(1.0)
		SetSliderDialogRange(0.1, 1.0)
		SetSliderDialogInterval(0.1)
	endevent

	event OnDefaultST()
		QLIE_WindowOpacityNormal = 1.0
		SetSliderOptionValueST(QLIE_WindowOpacityNormal, "{1}")
	endevent
endstate

state state_WindowOpacityEmpty
	event OnSliderAcceptST(float value)
		QLIE_WindowOpacityEmpty = value
		SetSliderOptionValueST(value, "{1}")
    endevent

	event OnSliderOpenST()
		SetSliderDialogStartValue(QLIE_WindowOpacityEmpty)
		SetSliderDialogDefaultValue(0.3)
		SetSliderDialogRange(0.1, 1.0)
		SetSliderDialogInterval(0.1)
	endevent

	event OnDefaultST()
		QLIE_WindowOpacityEmpty = 0.3
		SetSliderOptionValueST(QLIE_WindowOpacityEmpty, "{1}")
	endevent
endstate

;---------------------------------------------------
;-- Display > Icon Settings ------------------------
;---------------------------------------------------

state state_ShowIconRead
	event OnSelectST()
		QLIE_ShowIconRead = !QLIE_ShowIconRead
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowIconRead))
	endevent

	event OnDefaultST()
		QLIE_ShowIconRead = true
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowIconRead))
	endevent
endstate

state state_ShowIconStolen
	event OnSelectST()
		QLIE_ShowIconStolen = !QLIE_ShowIconStolen
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowIconStolen))
	endevent

	event OnDefaultST()
		QLIE_ShowIconStolen = true
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowIconStolen))
	endevent
endstate

state state_ShowIconEnchanted
	event OnSelectST()
		QLIE_ShowIconEnchanted = !QLIE_ShowIconEnchanted
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowIconEnchanted))
	endevent

	event OnDefaultST()
		QLIE_ShowIconEnchanted = true
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowIconEnchanted))
	endevent
endstate

state state_ShowIconEnchantedKnown
	event OnSelectST()
		QLIE_ShowIconEnchantedKnown = !QLIE_ShowIconEnchantedKnown
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowIconEnchantedKnown))
	endevent

	event OnDefaultST()
		QLIE_ShowIconEnchantedKnown = true
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowIconEnchantedKnown))
	endevent
endstate

state state_ShowIconEnchantedSpecial
	event OnSelectST()
		QLIE_ShowIconEnchantedSpecial = !QLIE_ShowIconEnchantedSpecial
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowIconEnchantedSpecial))
	endevent

	event OnDefaultST()
		QLIE_ShowIconEnchantedSpecial = true
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowIconEnchantedSpecial))
	endevent
endstate

;---------------------------------------------------
;-- Display > Info Columns -------------------------
;---------------------------------------------------

state state_InfoColumnPreset
	event OnMenuOpenST()
		SetMenuDialogDefaultIndex(2)
		SetMenuDialogStartIndex(InfoColumnPresetIndex)
		SetMenuDialogOptions(InfoColumnPresetNames)
	endevent

	event OnMenuAcceptST(int presetIndex)
		SetInfoColumns(InfoColumnPresetStrings[presetIndex], presetIndex)
	endevent

	event OnDefaultST()
		SetInfoColumns(InfoColumnPresetStrings[0], 0) ; Default
	endevent
endstate

state state_InfoColumnString
	event OnInputOpenST()
		SetInputDialogStartText(StringJoin(QLIE_InfoColumns))
	endevent

	event OnInputAcceptST(string input)
		SetInfoColumns(input, -1) ; Custom
	endevent

	event OnDefaultST()
		SetInfoColumns(InfoColumnPresetStrings[0], 0) ; Default
	endevent
endstate

function SetInfoColumns(string infoColumnsString, int presetIndex) ; Pass presetIndex = -1 to validate and auto detect
	string[] columns = StringSplit(infoColumnsString)

	if GetLength(infoColumnsString) == 0
		columns = None
	endif

	if presetIndex < 0
		if !ValidateInfoColumns(columns)
			return
		endif

		presetIndex = InfoColumnPresetStrings.Find(infoColumnsString)
	endif

	InfoColumnPresetIndex = presetIndex
	if presetIndex < 0
		SetMenuOptionValueST("$qlie_InfoColumnPreset_custom", false, "state_InfoColumnPreset")
	else
		SetMenuOptionValueST(InfoColumnPresetNames[InfoColumnPresetIndex], false, "state_InfoColumnPreset")
	endif

	QLIE_InfoColumns = columns
endfunction

bool function ValidateInfoColumns(string[] columns)
	int i = 0
	while i < columns.Length
		string col = columns[i]
		if col != "value" && col != "weight" && col != "valuePerWeight"
			ShowMsg("Invalid info column name: " + col)
			return false
		endif
		i += 1
	endwhile

	return true
endfunction

;---------------------------------------------------
;-- Sorting ----------------------------------------
;---------------------------------------------------

state state_SortInsert
	event OnMenuOpenST()
		SetMenuDialogStartIndex(0)
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(SortRulesAvailable)
	endevent

	event OnMenuAcceptST(int index)
		InsertSortOption(SortRulesAvailable[index])
	endevent

	event OnHighlightST()
		if SortSelectedRuleIndex >= 0 && SortSelectedRuleIndex < QLIE_SortRulesActive.Length
			SetInfoText("$qlie_SortInsert_info{" + QLIE_SortRulesActive[SortSelectedRuleIndex] + "}")
		else
			SetInfoText("$qlie_SortInsert_info")
		endif
	endevent
endstate

state state_SortRemove
	event OnSelectST()
		RemoveSelectedSortOption()
	endevent

	event OnHighlightST()
		if SortSelectedRuleIndex >= 0 && SortSelectedRuleIndex < QLIE_SortRulesActive.Length
			SetInfoText("$qlie_SortRemove_info{" + QLIE_SortRulesActive[SortSelectedRuleIndex] + "}")
		else
			SetInfoText("$qlie_SortRemove_info")
		endif
	endevent
endstate

state state_SortReset
	event OnSelectST()
		InitSortRuleLists(true)
		ForcePageReset()
	endevent
endstate

state state_SortPresetSave
	event OnInputAcceptST(string presetName)
		SaveSortPreset(presetName)
	endevent
endstate

state state_SortPresetLoad
	event OnMenuOpenST()
		SetMenuDialogStartIndex(0)
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(SortPresetNames)
	endevent

	event OnMenuAcceptST(int index)
		LoadSortPreset(index)
	endevent
endstate

function InsertSortOption(string optionName)
	int index = SortSelectedRuleIndex
	if index < 0 || index >= QLIE_SortRulesActive.Length
		index = QLIE_SortRulesActive.Length
	else
		SortSelectedRuleIndex = index + 1
	endif

	QLIE_SortRulesActive = InsertSortOptionPriority(QLIE_SortRulesActive, optionName, index)

	ForcePageReset()
endfunction

function RemoveSelectedSortOption()
	QLIE_SortRulesActive = RemoveSortOptionPriority(QLIE_SortRulesActive, SortSelectedRuleIndex)

	if SortSelectedRuleIndex >= QLIE_SortRulesActive.Length
		SortSelectedRuleIndex = QLIE_SortRulesActive.Length - 1
	endif

	ForcePageReset()
endfunction

function SaveSortPreset(string presetName)
	if presetName == ""
		return
	endif

	if PapyrusUtil.GetScriptVersion() <= 31
		ShowMsg("$qlie_SortPresetSave_unsupported")
		return
	endif

	JsonUtil.SetPathStringArray(SortPresetPath + presetName, ".SortRules", QLIE_SortRulesActive, false)
	JsonUtil.Save(SortPresetPath + presetName, false)
	ShowMsg("$qlie_SortPresetSave_success")
	ForcePageReset()
endfunction

function LoadSortPreset(int index)
	if index <= 0
		return
	endif

	if PapyrusUtil.GetScriptVersion() <= 31
		ShowMsg("$qlie_SortPresetLoad_unsupported")
		return
	endif

	if index < SortPredefinedPresetCount
		QLIE_SortRulesActive = GetSortingPreset(index)
	else
		QLIE_SortRulesActive = JsonUtil.PathStringElements(SortPresetPath + SortPresetNames[index], ".SortRules")
	endif

	ForcePageReset()
endfunction

;---------------------------------------------------
;-- Controls ---------------------------------------
;---------------------------------------------------

state state_ControlsTake
	event OnKeyMapChangeST(int keyCode, string conflictControl, string conflictName)
		SetKeyMapOptionValueST(keyCode)

		bool isGamepad = keyCode >= 266
		if isGamepad
			QLIE_KeybindingTakeGamepad = keyCode
		else
			QLIE_KeybindingTake = keyCode
		endif

		if isGamepad != GamepadMode
			GamepadMode = isGamepad
			ForcePageReset()
		endif
	endevent
endstate

state state_ControlsTakeAll
	event OnKeyMapChangeST(int keyCode, string conflictControl, string conflictName)
		SetKeyMapOptionValueST(keyCode)

		bool isGamepad = keyCode >= 266
		if isGamepad
			QLIE_KeybindingTakeAllGamepad = keyCode
		else
			QLIE_KeybindingTakeAll = keyCode
		endif

		if isGamepad != GamepadMode
			GamepadMode = isGamepad
			ForcePageReset()
		endif
	endevent
endstate

state state_ControlsTransfer
	event OnKeyMapChangeST(int keyCode, string conflictControl, string conflictName)
		SetKeyMapOptionValueST(keyCode)

		bool isGamepad = keyCode >= 266
		if isGamepad
			QLIE_KeybindingTransferGamepad = keyCode
		else
			QLIE_KeybindingTransfer = keyCode
		endif

		if isGamepad != GamepadMode
			GamepadMode = isGamepad
			ForcePageReset()
		endif
	endevent
endstate

state state_ControlsTakeModifier
	event OnMenuOpenST()
		SetMenuDialogStartIndex(0)
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(KeyModifierOptions)
	endevent

	event OnMenuAcceptST(int index)
		QLIE_KeybindingTakeModifier = index
		SetMenuOptionValueST(KeyModifierOptions[QLIE_KeybindingTakeModifier])
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_ControlsModifier_info")
	endevent
endstate

state state_ControlsTakeAllModifier
	event OnMenuOpenST()
		SetMenuDialogStartIndex(0)
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(KeyModifierOptions)
	endevent

	event OnMenuAcceptST(int index)
		QLIE_KeybindingTakeAllModifier = index
		SetMenuOptionValueST(KeyModifierOptions[QLIE_KeybindingTakeAllModifier])
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_ControlsModifier_info")
	endevent
endstate

state state_ControlsTransferModifier
	event OnMenuOpenST()
		SetMenuDialogStartIndex(0)
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(KeyModifierOptions)
	endevent

	event OnMenuAcceptST(int index)
		QLIE_KeybindingTransferModifier = index
		SetMenuOptionValueST(KeyModifierOptions[QLIE_KeybindingTransferModifier])
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_ControlsModifier_info")
	endevent
endstate

state state_ControlReset
	event OnSelectST()
		ResetControls()
		ForcePageReset()
	endevent
endstate

state state_ControlPresetSave
	event OnInputAcceptST(string presetName)
		SaveControlPreset(presetName)
	endevent
endstate

state state_ControlPresetLoad
	event OnMenuOpenST()
		SetMenuDialogStartIndex(0)
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(ControlPresetNames)
	endevent

	event OnMenuAcceptST(int index)
		LoadControlPreset(index)
	endevent
endstate

function ResetControls()
	QLIE_KeybindingTake = 18
	QLIE_KeybindingTakeModifier = 0
	QLIE_KeybindingTakeAll = 19
	QLIE_KeybindingTakeAllModifier = 0
	QLIE_KeybindingTransfer = 16
	QLIE_KeybindingTransferModifier = 0
endfunction

function SaveControlPreset(string presetName)
	if presetName == ""
		return
	endif

	if PapyrusUtil.GetScriptVersion() <= 31
		ShowMsg("$qlie_ControlPresetSave_unsupported")
		return
	endif

	string path = ControlPresetPath + presetName

	JsonUtil.SetPathIntValue(path, "KeybindingTake", QLIE_KeybindingTake)
	JsonUtil.SetPathIntValue(path, "KeybindingTakeAll", QLIE_KeybindingTakeAll)
	JsonUtil.SetPathIntValue(path, "KeybindingTransfer", QLIE_KeybindingTransfer)
	JsonUtil.SetPathIntValue(path, "KeybindingTakeModifier", QLIE_KeybindingTakeModifier)
	JsonUtil.SetPathIntValue(path, "KeybindingTakeAllModifier", QLIE_KeybindingTakeAllModifier)
	JsonUtil.SetPathIntValue(path, "KeybindingTransferModifier", QLIE_KeybindingTransferModifier)
	JsonUtil.SetPathIntValue(path, "KeybindingTakeGamepad", QLIE_KeybindingTakeGamepad)
	JsonUtil.SetPathIntValue(path, "KeybindingTakeAllGamepad", QLIE_KeybindingTakeAllGamepad)
	JsonUtil.SetPathIntValue(path, "KeybindingTransferGamepad", QLIE_KeybindingTransferGamepad)

	JsonUtil.Save(path, false)
	ShowMsg("$qlie_ControlPresetSave_success")
	ForcePageReset()
endfunction

function LoadControlPreset(int index)
	if index <= 0
		return
	endif

	if PapyrusUtil.GetScriptVersion() <= 31
		ShowMsg("$qlie_ControlPresetLoad_unsupported")
		return
	endif

	if index < ControlPredefinedPresetCount
		if index == 1
			QLIE_KeybindingTake = 18				; E
			QLIE_KeybindingTakeAll = 18				; E
			QLIE_KeybindingTransfer = 19			; R
			QLIE_KeybindingTakeModifier = 1			; None
			QLIE_KeybindingTakeAllModifier = 2		; Shift
			QLIE_KeybindingTransferModifier = 0		; Ignore
			QLIE_KeybindingTakeGamepad = 276		; Gamepad A
			QLIE_KeybindingTakeAllGamepad = 278		; Gamepad X
			QLIE_KeybindingTransferGamepad = 273	; Gamepad Right Stick
		else
			QLIE_KeybindingTake = 18				; E
			QLIE_KeybindingTakeAll = 19				; R
			QLIE_KeybindingTransfer = 16			; Q
			QLIE_KeybindingTakeModifier = 0			; Ignore
			QLIE_KeybindingTakeAllModifier = 0		; Ignore
			QLIE_KeybindingTransferModifier = 0		; Ignore
			QLIE_KeybindingTakeGamepad = 276		; Gamepad A
			QLIE_KeybindingTakeAllGamepad = 278		; Gamepad X
			QLIE_KeybindingTransferGamepad = 271	; Gamepad Back
		endif
	else
		string path = ControlPresetPath + ControlPresetNames[index]
		JsonUtil.Load(path)

		QLIE_KeybindingTake = JsonUtil.GetPathIntValue(path, "KeybindingTake", QLIE_KeybindingTake)
		QLIE_KeybindingTakeAll = JsonUtil.GetPathIntValue(path, "KeybindingTakeAll", QLIE_KeybindingTakeAll)
		QLIE_KeybindingTransfer = JsonUtil.GetPathIntValue(path, "KeybindingTransfer", QLIE_KeybindingTransfer)
		QLIE_KeybindingTakeModifier = JsonUtil.GetPathIntValue(path, "KeybindingTakeModifier", QLIE_KeybindingTakeModifier)
		QLIE_KeybindingTakeAllModifier = JsonUtil.GetPathIntValue(path, "KeybindingTakeAllModifier", QLIE_KeybindingTakeAllModifier)
		QLIE_KeybindingTransferModifier = JsonUtil.GetPathIntValue(path, "KeybindingTransferModifier", QLIE_KeybindingTransferModifier)
		QLIE_KeybindingTakeGamepad = JsonUtil.GetPathIntValue(path, "KeybindingTakeGamepad", QLIE_KeybindingTakeGamepad)
		QLIE_KeybindingTakeAllGamepad = JsonUtil.GetPathIntValue(path, "KeybindingTakeAllGamepad", QLIE_KeybindingTakeAllGamepad)
		QLIE_KeybindingTransferGamepad = JsonUtil.GetPathIntValue(path, "KeybindingTransferGamepad", QLIE_KeybindingTransferGamepad)

		JsonUtil.Unload(path)
	endif

	ForcePageReset()
endfunction

;---------------------------------------------------
;-- Compatibility ----------------------------------
;---------------------------------------------------

state state_ShowIconLOTDDisplayed
	event OnSelectST()
		QLIE_ShowIconLOTDDisplayed = !QLIE_ShowIconLOTDDisplayed
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowIconLOTDDisplayed))
	endevent

	event OnDefaultST()
		QLIE_ShowIconLOTDDisplayed = true
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowIconLOTDDisplayed))
	endevent
endstate

state state_ShowIconLOTDCarried
	event OnSelectST()
		QLIE_ShowIconLOTDCarried = !QLIE_ShowIconLOTDCarried
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowIconLOTDCarried))
	endevent

	event OnDefaultST()
		QLIE_ShowIconLOTDCarried = true
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowIconLOTDCarried))
	endevent
endstate

state state_ShowIconLOTDNew

	event OnSelectST()
		QLIE_ShowIconLOTDNew = !QLIE_ShowIconLOTDNew
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowIconLOTDNew))
	endevent

	event OnDefaultST()
		QLIE_ShowIconLOTDNew = true
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowIconLOTDNew))
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_ShowIconLOTDNew_info")
	endevent
endstate

state state_ShowIconCompletionistNeeded

	event OnSelectST()
		QLIE_ShowIconCompletionistNeeded = !QLIE_ShowIconCompletionistNeeded
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowIconCompletionistNeeded))
	endevent

	event OnDefaultST()
		QLIE_ShowIconCompletionistNeeded = true
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowIconCompletionistNeeded))
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_ShowIconCompletionistNeeded_info")
	endevent
endstate

state state_ShowIconCompletionistCollected

	event OnSelectST()
		QLIE_ShowIconCompletionistCollected = !QLIE_ShowIconCompletionistCollected
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowIconCompletionistCollected))
	endevent

	event OnDefaultST()
		QLIE_ShowIconCompletionistCollected = true
		SetTextOptionValueST(GetEnabledStatusText(QLIE_ShowIconCompletionistCollected))
	endevent

	event OnHighlightST()
		SetInfoText("$qlie_ShowIconCompletionistCollected_info")
	endevent
endstate

;---------------------------------------------------
;-- Profile Management -----------------------------
;---------------------------------------------------

function ResetSettings()
	; General > Behavior
	QLIE_ShowInCombat = true
	QLIE_ShowWhenEmpty = false
	QLIE_ShowWhenUnlocked = true
	QLIE_ShowInThirdPerson = true
	QLIE_ShowWhenMounted = false
	QLIE_EnableForAnimals = true
	QLIE_EnableForDragons = true
	QLIE_BreakInvisibility = true

	; Display > Window Settings
	QLIE_WindowOffsetX = 100
	QLIE_WindowOffsetY = -200
	QLIE_WindowScale = 1.0
	QLIE_WindowAnchor = 0
	QLIE_WindowMinLines = 0
	QLIE_WindowMaxLines = 7
	QLIE_WindowOpacityNormal = 1.0
	QLIE_WindowOpacityEmpty = 0.3

	; Display > Icon Settings
	QLIE_ShowIconRead = true
	QLIE_ShowIconStolen = true
	QLIE_ShowIconEnchanted = true
	QLIE_ShowIconEnchantedKnown = true
	QLIE_ShowIconEnchantedSpecial = true

	; Display > Info Column Layout
	SetInfoColumns(InfoColumnPresetStrings[0], 0)

	; Sorting
	LoadSortPreset(0)

	; Controls
	LoadControlPreset(0)

	; Compatibility > LOTD Icons
	QLIE_ShowIconLOTDNew = true
	QLIE_ShowIconLOTDCarried = true
	QLIE_ShowIconLOTDDisplayed = true

	; Compatibility > Completionist Icons
	QLIE_ShowIconCompletionistNeeded = true
	QLIE_ShowIconCompletionistCollected = true

	if IsInMenuMode()
		ForcePageReset()
	endif
endfunction

function SaveProfile()
	if PapyrusUtil.GetScriptVersion() <= 31
		ShowMsg("$qlie_ProfileSave_failure")
		return
	endif

	JsonUtil.SetPathIntValue(ConfigPath, ".ShowInCombat", QLIE_ShowInCombat as int)
	JsonUtil.SetPathIntValue(ConfigPath, ".ShowWhenEmpty", QLIE_ShowWhenEmpty as int)
	JsonUtil.SetPathIntValue(ConfigPath, ".ShowWhenUnlocked", QLIE_ShowWhenUnlocked as int)
	JsonUtil.SetPathIntValue(ConfigPath, ".ShowInThirdPerson", QLIE_ShowInThirdPerson as int)
	JsonUtil.SetPathIntValue(ConfigPath, ".ShowWhenMounted", QLIE_ShowWhenMounted as int)
	JsonUtil.SetPathIntValue(ConfigPath, ".EnableForAnimals", QLIE_EnableForAnimals as int)
	JsonUtil.SetPathIntValue(ConfigPath, ".EnableForDragons", QLIE_EnableForDragons as int)
	JsonUtil.SetPathIntValue(ConfigPath, ".BreakInvisibility", QLIE_BreakInvisibility as int)

	JsonUtil.SetPathIntValue(ConfigPath, ".WindowOffsetX", QLIE_WindowOffsetX)
	JsonUtil.SetPathIntValue(ConfigPath, ".WindowOffsetY", QLIE_WindowOffsetY)
	JsonUtil.SetPathFloatValue(ConfigPath, ".WindowScale", QLIE_WindowScale)
	JsonUtil.SetPathIntValue(ConfigPath, ".WindowAnchor", QLIE_WindowAnchor)
	JsonUtil.SetPathIntValue(ConfigPath, ".WindowMinLines", QLIE_WindowMinLines)
	JsonUtil.SetPathIntValue(ConfigPath, ".WindowMaxLines", QLIE_WindowMaxLines)
	JsonUtil.SetPathFloatValue(ConfigPath, ".WindowOpacityNormal", QLIE_WindowOpacityNormal)
	JsonUtil.SetPathFloatValue(ConfigPath, ".WindowOpacityEmpty", QLIE_WindowOpacityEmpty)

	JsonUtil.SetPathIntValue(ConfigPath, ".ShowIconRead", QLIE_ShowIconRead as int)
	JsonUtil.SetPathIntValue(ConfigPath, ".ShowIconStolen", QLIE_ShowIconStolen as int)
	JsonUtil.SetPathIntValue(ConfigPath, ".ShowIconEnchanted", QLIE_ShowIconEnchanted as int)
	JsonUtil.SetPathIntValue(ConfigPath, ".ShowIconEnchantedKnown", QLIE_ShowIconEnchantedKnown as int)
	JsonUtil.SetPathIntValue(ConfigPath, ".ShowIconEnchantedSpecial", QLIE_ShowIconEnchantedSpecial as int)

	JsonUtil.SetPathStringArray(ConfigPath, ".InfoColumns", QLIE_InfoColumns)

	JsonUtil.SetPathStringArray(ConfigPath, ".SortRulesActive", QLIE_SortRulesActive)

	JsonUtil.SetPathIntValue(ConfigPath, "KeybindingTake", QLIE_KeybindingTake)
	JsonUtil.SetPathIntValue(ConfigPath, "KeybindingTakeAll", QLIE_KeybindingTakeAll)
	JsonUtil.SetPathIntValue(ConfigPath, "KeybindingTransfer", QLIE_KeybindingTransfer)
	JsonUtil.SetPathIntValue(ConfigPath, "KeybindingTakeModifier", QLIE_KeybindingTakeModifier)
	JsonUtil.SetPathIntValue(ConfigPath, "KeybindingTakeAllModifier", QLIE_KeybindingTakeAllModifier)
	JsonUtil.SetPathIntValue(ConfigPath, "KeybindingTransferModifier", QLIE_KeybindingTransferModifier)
	JsonUtil.SetPathIntValue(ConfigPath, "KeybindingTakeGamepad", QLIE_KeybindingTakeGamepad)
	JsonUtil.SetPathIntValue(ConfigPath, "KeybindingTakeAllGamepad", QLIE_KeybindingTakeAllGamepad)
	JsonUtil.SetPathIntValue(ConfigPath, "KeybindingTransferGamepad", QLIE_KeybindingTransferGamepad)

	JsonUtil.SetPathIntValue(ConfigPath, ".ShowIconLOTDNew", QLIE_ShowIconLOTDNew as int)
	JsonUtil.SetPathIntValue(ConfigPath, ".ShowIconLOTDCarried", QLIE_ShowIconLOTDCarried as int)
	JsonUtil.SetPathIntValue(ConfigPath, ".ShowIconLOTDDisplayed", QLIE_ShowIconLOTDDisplayed as int)

	JsonUtil.SetPathIntValue(ConfigPath, ".ShowIconCompletionistNeeded", QLIE_ShowIconCompletionistNeeded as int)
	JsonUtil.SetPathIntValue(ConfigPath, ".ShowIconCompletionistCollected", QLIE_ShowIconCompletionistCollected as int)

	JsonUtil.Save(ConfigPath, false)
	ShowMsg("$qlie_ProfileSave_success")
endfunction

function LoadProfile()
	if PapyrusUtil.GetScriptVersion() <= 31
		if IsInMenuMode()
			ShowMsg("$qlie_ProfileLoad_unsupported")
		else
			Notification("$qlie_NotificationProfileUnsupported")
		endif
		return
	endif

	if !JsonUtil.JsonExists(ConfigPath)
		if IsInMenuMode()
			ShowMsg("$qlie_ProfileLoad_missing")
		else
			Notification("$qlie_NotificationProfileMissing")
			ResetSettings()
		endif
		return
	endif

	if !JsonUtil.IsGood(ConfigPath)
		if IsInMenuMode()
			ShowMsg("$qlie_ProfileLoad_corrupt{" + JsonUtil.GetErrors(ConfigPath) + "}")
		else
			Notification("$qlie_NotificationProfileCorrupt");
			ResetSettings()
		endif
		return
	endif

	JsonUtil.Load(ConfigPath)

	QLIE_ShowInCombat = JsonUtil.GetPathIntValue(ConfigPath, ".ShowInCombat", QLIE_ShowInCombat as int)
	QLIE_ShowWhenEmpty = JsonUtil.GetPathIntValue(ConfigPath, ".ShowWhenEmpty", QLIE_ShowWhenEmpty as int)
	QLIE_BreakInvisibility = JsonUtil.GetPathIntValue(ConfigPath, ".BreakInvisibility", QLIE_BreakInvisibility as int)
	QLIE_ShowWhenUnlocked = JsonUtil.GetPathIntValue(ConfigPath, ".ShowWhenUnlocked", QLIE_ShowWhenUnlocked as int)
	QLIE_EnableForAnimals = JsonUtil.GetPathIntValue(ConfigPath, ".EnableForAnimals", QLIE_EnableForAnimals as int)

	QLIE_WindowOffsetX = JsonUtil.GetPathIntValue(ConfigPath, ".WindowOffsetX", QLIE_WindowOffsetX)
	QLIE_WindowOffsetY = JsonUtil.GetPathIntValue(ConfigPath, ".WindowOffsetY", QLIE_WindowOffsetY)
	QLIE_WindowScale = JsonUtil.GetPathFloatValue(ConfigPath, ".WindowScale", QLIE_WindowScale)
	QLIE_WindowAnchor = JsonUtil.GetPathIntValue(ConfigPath, ".WindowAnchor", QLIE_WindowAnchor)
	QLIE_WindowMinLines = JsonUtil.GetPathIntValue(ConfigPath, ".WindowMinLines", QLIE_WindowMinLines)
	QLIE_WindowMaxLines = JsonUtil.GetPathIntValue(ConfigPath, ".WindowMaxLines", QLIE_WindowMaxLines)
	QLIE_WindowOpacityNormal = JsonUtil.GetPathFloatValue(ConfigPath, ".WindowOpacityNormal", QLIE_WindowOpacityNormal)
	QLIE_WindowOpacityEmpty = JsonUtil.GetPathFloatValue(ConfigPath, ".WindowOpacityEmpty", QLIE_WindowOpacityEmpty)

	QLIE_ShowIconRead = JsonUtil.GetPathIntValue(ConfigPath, ".ShowIconRead", QLIE_ShowIconRead as int)
	QLIE_ShowIconStolen = JsonUtil.GetPathIntValue(ConfigPath, ".ShowIconStolen", QLIE_ShowIconStolen as int)
	QLIE_ShowIconEnchanted = JsonUtil.GetPathIntValue(ConfigPath, ".ShowIconEnchanted", QLIE_ShowIconEnchanted as int)
	QLIE_ShowIconEnchantedKnown = JsonUtil.GetPathIntValue(ConfigPath, ".ShowIconEnchantedKnown", QLIE_ShowIconEnchantedKnown as int)
	QLIE_ShowIconEnchantedSpecial = JsonUtil.GetPathIntValue(ConfigPath, ".ShowIconEnchantedSpecial", QLIE_ShowIconEnchantedSpecial as int)

	QLIE_InfoColumns = JsonUtil.PathStringElements(ConfigPath, ".InfoColumns", QLIE_InfoColumns)

	QLIE_SortRulesActive = JsonUtil.PathStringElements(ConfigPath, ".SortRulesActive", QLIE_SortRulesActive)

	QLIE_KeybindingTake = JsonUtil.GetPathIntValue(ConfigPath, "KeybindingTake", QLIE_KeybindingTake)
	QLIE_KeybindingTakeAll = JsonUtil.GetPathIntValue(ConfigPath, "KeybindingTakeAll", QLIE_KeybindingTakeAll)
	QLIE_KeybindingTransfer = JsonUtil.GetPathIntValue(ConfigPath, "KeybindingTransfer", QLIE_KeybindingTransfer)
	QLIE_KeybindingTakeModifier = JsonUtil.GetPathIntValue(ConfigPath, "KeybindingTakeModifier", QLIE_KeybindingTakeModifier)
	QLIE_KeybindingTakeAllModifier = JsonUtil.GetPathIntValue(ConfigPath, "KeybindingTakeAllModifier", QLIE_KeybindingTakeAllModifier)
	QLIE_KeybindingTransferModifier = JsonUtil.GetPathIntValue(ConfigPath, "KeybindingTransferModifier", QLIE_KeybindingTransferModifier)
	QLIE_KeybindingTakeGamepad = JsonUtil.GetPathIntValue(ConfigPath, "KeybindingTakeGamepad", QLIE_KeybindingTakeGamepad)
	QLIE_KeybindingTakeAllGamepad = JsonUtil.GetPathIntValue(ConfigPath, "KeybindingTakeAllGamepad", QLIE_KeybindingTakeAllGamepad)
	QLIE_KeybindingTransferGamepad = JsonUtil.GetPathIntValue(ConfigPath, "KeybindingTransferGamepad", QLIE_KeybindingTransferGamepad)

	QLIE_ShowIconLOTDDisplayed = JsonUtil.GetPathIntValue(ConfigPath, ".ShowIconLOTDDisplayed", QLIE_ShowIconLOTDDisplayed as int)
	QLIE_ShowIconLOTDCarried = JsonUtil.GetPathIntValue(ConfigPath, ".ShowIconLOTDCarried", QLIE_ShowIconLOTDCarried as int)
	QLIE_ShowIconLOTDNew = JsonUtil.GetPathIntValue(ConfigPath, ".ShowIconLOTDNew", QLIE_ShowIconLOTDNew as int)

	QLIE_ShowIconCompletionistNeeded = JsonUtil.GetPathIntValue(ConfigPath, ".ShowIconCompletionistNeeded", QLIE_ShowIconCompletionistNeeded as int)
	QLIE_ShowIconCompletionistCollected = JsonUtil.GetPathIntValue(ConfigPath, ".ShowIconCompletionistCollected", QLIE_ShowIconCompletionistCollected as int)

	JsonUtil.Unload(ConfigPath, false)

	if IsInMenuMode()
		ShowMsg("$qlie_ProfileLoad_success")
		ForcePageReset()
	else
		Notification("$qlie_NotificationProfileImported")
	endif
endfunction

function AutoLoadConfig()
	if AutoLoadedProfile
		return
	endif

	Initialize()
	LoadProfile()
	AutoLoadedProfile = true
endfunction
