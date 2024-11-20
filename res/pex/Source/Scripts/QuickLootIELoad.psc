scriptname QuicklootIELoad extends ReferenceAlias

QuickLootIEMaintenance property MaintenanceScript auto

;---------------------------------------------------
;-- Events -----------------------------------------
;---------------------------------------------------

event OnPlayerLoadGame()
	MaintenanceScript.RegisterForSingleUpdate(0.1)
endevent
