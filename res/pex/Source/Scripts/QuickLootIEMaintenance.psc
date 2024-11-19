scriptname QuickLootIEMaintenance extends Quest

import QuickLootIENative
import utility
import debug

bool property bFirstTimeSetupFinished auto hidden

float property fSKSE auto hidden
 
int property fVersion auto hidden
int property fVMajor auto hidden 
int property fVMinor auto hidden
int property fVPatch auto hidden
int property fVTweak auto hidden

int curVersion
int curVMajor
int curVMinor
int curVPatch
int curVTweak

string property ModVersion auto hidden

bool bUpdated

;---------------------------------------------------
;-- Events -----------------------------------------
;---------------------------------------------------

event OnInit()
	RegisterForSingleUpdate(1)
endevent

;---------------------------------------------------
;-- Events -----------------------------------------
;---------------------------------------------------

event OnUpdate()
	SetFrameworkQuest(self as Quest)
	
	if (!bFirstTimeSetupFinished)
		DoVersioning()
		return
	endif
	
	self.CheckVersioning()
endevent

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function GetSKSEVersion()
	fSKSE = (SKSE.GetVersion() * 10000 + SKSE.GetVersionMinor() * 100 + SKSE.GetVersionBeta())
endfunction	

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function DoVersioning() ; Initial versioning on a new game.
	
	GetSKSEVersion()
	fVMajor = 1
	fVMinor = 2
	fVPatch = 1
	fVTweak = 0
	fVersion = (fVMajor * 1000) + (fVMinor * 100) + (fVPatch * 10) + (fVTweak)
	ModVersion = (fVMajor + "." + fVMinor + "." + fVPatch + "." + fVTweak)
	
	LogWithPlugin("QuickLootIE UDS: Initial Versioning Completed")
	bFirstTimeSetupFinished = true
endfunction	

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function CheckVersioning() ; Versioning ran from OnPlayerLoadGame()
	
	GetSKSEVersion()
	curVMajor = 1
	curVMinor = 2
	curVPatch = 1
	curVTweak = 0
	curVersion = (curVMajor * 1000) + (curVMinor * 100) + (curVPatch * 10) + (curVTweak)
	
	while IsInMenuMode()
		Wait(0.1)
	endwhile

	if (fVersion < curVersion)
		UpdateKicker()
	endif
endfunction

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function UpdateKicker()
	
	Notification("QuickLootIE UDS: Running Updates...")
	
	bUpdated = false

	if (fVersion < 1210)
		; Handle any updates here
		
		fVMajor = curVMajor
		fVMinor = curVMinor
		fVPatch = curVPatch
		fVTweak = curVTweak
		fVersion = curVersion
		bUpdated = true
	endif
	
	if (!bUpdated)
		LogWithPlugin("QuickLootIE UDS: Update Failed...")
		return
	endif

	ModVersion = (fVMajor + "." + fVMinor + "." + fVPatch + "." + fVTweak)
	LogWithPlugin("QuickLootIE UDS: Update to Version " + fVMajor + "." + fVMinor + "." + fVPatch + "." + fVTweak + " Succeeded...")
	Notification("QuickLoot IE UDS: Updated To Version " + ModVersion)
endfunction
