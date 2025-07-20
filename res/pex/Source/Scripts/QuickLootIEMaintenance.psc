scriptname QuickLootIEMaintenance extends Quest

import QuickLootIEMCM
import QuickLootIENative
import Utility
import Debug

int property LastVersionMajor auto hidden
int property LastVersionMinor auto hidden
int property LastVersionPatch auto hidden
int property LastVersionTweak auto hidden

int property LastVersionNumber auto hidden
string property LastVersionString auto hidden

int property CurrentVersionMajor auto hidden
int property CurrentVersionMinor auto hidden
int property CurrentVersionPatch auto hidden
int property CurrentVersionTweak auto hidden

int property CurrentVersionNumber auto hidden
string property CurrentVersionString auto hidden

QuickLootIEMCM property MCMScript auto hidden

;---------------------------------------------------
;-- Events -----------------------------------------
;---------------------------------------------------

event OnInit()
	RegisterForSingleUpdate(1)
endevent

event OnUpdate()
	MCMScript = (self as Quest) as QuickLootIEMCM
	SetFrameworkQuest(self as Quest)

	InitCurrentVersion()
	CheckVersionChange()
endevent

;---------------------------------------------------
;-- Helper Functions -------------------------------
;---------------------------------------------------

int function CombineVersionNumber(int major, int minor, int patch, int tweak)
	return major * 1000 + minor * 100 + patch * 10 + tweak
endfunction

string function CombineVersionString(int major, int minor, int patch, int tweak)
	return major + "." + minor + "." + patch + "." + tweak
endfunction

function InitCurrentVersion()
	CurrentVersionMajor = 4
	CurrentVersionMinor = 0
	CurrentVersionPatch = 0
	CurrentVersionTweak = 0

	CurrentVersionNumber = CombineVersionNumber(CurrentVersionMajor, CurrentVersionMinor, CurrentVersionPatch, CurrentVersionTweak)
	CurrentVersionString = CombineVersionString(CurrentVersionMajor, CurrentVersionMinor, CurrentVersionPatch, CurrentVersionTweak)
endfunction

function UpdateLastVersion()
	LastVersionMajor = CurrentVersionMajor
	LastVersionMinor = CurrentVersionMinor
	LastVersionPatch = CurrentVersionPatch
	LastVersionTweak = CurrentVersionTweak

	LastVersionNumber = CurrentVersionNumber
	LastVersionString = CurrentVersionString
endfunction

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function CheckVersionChange()
	while IsInMenuMode()
		Wait(0.1)
	endwhile

	if LastVersionNumber < CurrentVersionNumber
		UpdateVersion()
	endif
endfunction

function UpdateVersion()
	Notification("QuickLoot IE: Running updates...")

	if LastVersionNumber < 4000
		Migrate4000()
	endif

	LogWithPlugin("Updated from version " + LastVersionString + " to " + CurrentVersionString)
	Notification("QuickLoot IE: Updated to version " + CurrentVersionString)

	UpdateLastVersion()
endfunction

function Migrate4000()
	; AutoLoadedProfile was replaced by IsInitialLoad
	MCMScript.IsInitialLoad = false

	; Migrate to new modifier format (key code instead of enum)
	MCMScript.QLIE_KeybindingTakeModifier = ModifierEnumToKeyCode(MCMScript.QLIE_KeybindingTakeModifier)
	MCMScript.QLIE_KeybindingTakeAllModifier = ModifierEnumToKeyCode(MCMScript.QLIE_KeybindingTakeAllModifier)
	MCMScript.QLIE_KeybindingTransferModifier = ModifierEnumToKeyCode(MCMScript.QLIE_KeybindingTransferModifier)
	MCMScript.QLIE_KeybindingDisableModifier = ModifierEnumToKeyCode(MCMScript.QLIE_KeybindingDisableModifier)
	MCMScript.QLIE_KeybindingEnableModifier = ModifierEnumToKeyCode(MCMScript.QLIE_KeybindingEnableModifier)

	Notification("Migrating control presets")
	MigrateControlPreset(MCMScript.ConfigPath)
	string[] presets = JsonUtil.JsonInFolder(MCMScript.ControlPresetPath)
	int i = 0
	while i < presets.Length
		MigrateControlPreset(MCMScript.ControlPresetPath + presets[i])
		i += 1
	endwhile
endfunction

function MigrateControlPreset(string path)
	if !JsonUtil.JsonExists(path)
		return
	endif

	if JsonUtil.GetPathIntValue(path, "KeybindingNewFormat") as bool
		return
	endif

	Notification(path)

	; Signal that we're using keycodes instead of the modifier enum
	JsonUtil.SetPathIntValue(path, "KeybindingNewFormat", 1)

	JsonUtil.SetPathIntValue(path, "KeybindingTakeModifier", ModifierEnumToKeyCode(JsonUtil.GetPathIntValue(path, "KeybindingTakeModifier", 0)))
	JsonUtil.SetPathIntValue(path, "KeybindingTakeAllModifier", ModifierEnumToKeyCode(JsonUtil.GetPathIntValue(path, "KeybindingTakeAllModifier", 0)))
	JsonUtil.SetPathIntValue(path, "KeybindingTransferModifier", ModifierEnumToKeyCode(JsonUtil.GetPathIntValue(path, "KeybindingTransferModifier", 0)))
	JsonUtil.SetPathIntValue(path, "KeybindingDisableModifier", ModifierEnumToKeyCode(JsonUtil.GetPathIntValue(path, "KeybindingDisableModifier", 0)))
	JsonUtil.SetPathIntValue(path, "KeybindingEnableModifier", ModifierEnumToKeyCode(JsonUtil.GetPathIntValue(path, "KeybindingEnableModifier", 0)))

	JsonUtil.Save(path)
endfunction

int function ModifierEnumToKeyCode(int enum)
	if enum == 1
		return 42 ; LShift
	elseif enum == 2
		return 29 ; LControl
	elseif enum == 3
		return 56 ; LAlt
	endif

	return -1
endfunction