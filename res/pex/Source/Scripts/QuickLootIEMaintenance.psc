scriptname QuickLootIEMaintenance extends quest

import QuickLootIENative
import utility
import debug

bool property bFirstTimeSetupFinished auto Hidden

float property fSKSE Auto Hidden
 
int property fVersion auto Hidden
int property fVMajor auto Hidden 
int property fVMinor auto Hidden
int property fVPatch auto Hidden
int property fVTweak auto Hidden

int curVersion
int curVMajor
int curVMinor
int curVPatch
int curVTweak

string property ModVersion auto Hidden

bool bUpdated

;---------------------------------------------------
;-- Events -----------------------------------------
;---------------------------------------------------

event OnInit()
	RegisterForSingleUpdate(1)
endEvent

;---------------------------------------------------
;-- Events -----------------------------------------
;---------------------------------------------------

event OnUpdate()
	SetFrameworkQuest(self as quest)
	
	if (!bFirstTimeSetupFinished)
		DoVersioning()
		return
	endif
	
	self.CheckVersioning()
endEvent

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function GetSKSEVersion()
	fSKSE = (SKSE.GetVersion() * 10000 + SKSE.GetVersionMinor() * 100 + SKSE.GetVersionBeta())
EndFunction	

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function DoVersioning() ;Initial versioning on a new game.
	
	GetSKSEVersion()
	fVMajor = 1
	fVMinor = 2
	fVPatch = 1
	fVTweak = 0
	fVersion = (fVMajor * 1000) + (fVMinor * 100) + (fVPatch * 10) + (fVTweak)
	ModVersion = (fVMajor + "." + fVMinor + "." + fVPatch + "." + fVTweak)
	
	LogWithPlugin("QuickLootIE UDS: Initial Versioning Completed")
	bFirstTimeSetupFinished = True
endFunction	

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function CheckVersioning() ;Versioning ran from OnPlayerLoadGame()
	
	GetSKSEVersion()
	curVMajor = 1
	curVMinor = 2
	curVPatch = 1
	curVTweak = 0
	curVersion = (curVMajor * 1000) + (curVMinor * 100) + (curVPatch * 10) + (curVTweak)
	
	While IsInMenuMode()
		Wait(0.1)
	endWhile

	if (fVersion < curVersion)
		UpdateKicker()
	endif
endFunction

;---------------------------------------------------
;-- Functions --------------------------------------
;---------------------------------------------------

function UpdateKicker()
	
	Notification("QuickLootIE UDS: Running Updates...")
	
	bUpdated = False

	if (fVersion < 1210)
		;Handle any Updates here
		
		fVMajor = curVMajor
		fVMinor = curVMinor
		fVPatch = curVPatch
		fVTweak = curVTweak
		fVersion = curVersion
		bUpdated = True
	endif
	
	if (!bUpdated)
		LogWithPlugin("QuickLootIE UDS: Update Failed...")
		return
	endif

	ModVersion = (fVMajor + "." + fVMinor + "." + fVPatch + "." + fVTweak)
	LogWithPlugin("QuickLootIE UDS: Update to Version " + fVMajor + "." + fVMinor + "." + fVPatch + "." + fVTweak + " Succeeded...")
	Notification("QuickLoot IE UDS: Updated To Version " + ModVersion)
endFunction
