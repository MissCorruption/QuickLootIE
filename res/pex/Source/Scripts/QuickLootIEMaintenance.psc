scriptname QuickLootIEMaintenance extends Quest

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

;---------------------------------------------------
;-- Events -----------------------------------------
;---------------------------------------------------

event OnInit()
	RegisterForSingleUpdate(1)
endevent

event OnUpdate()
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
	CurrentVersionMajor = 3
	CurrentVersionMinor = 4
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

	if LastVersionNumber < 2100
		;
	endif

	UpdateLastVersion()

	LogWithPlugin("Updated to version " + CurrentVersionString)
	Notification("QuickLoot IE: Updated to version " + CurrentVersionString)
endfunction
