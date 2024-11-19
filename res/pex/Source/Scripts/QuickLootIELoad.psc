scriptname QuicklootIELoad extends ReferenceAlias

QuickLootIEMaintenance property QLIEAPI auto

;---------------------------------------------------
;-- Events -----------------------------------------
;---------------------------------------------------

event OnPlayerLoadGame()
	QLIEAPI.RegisterForSingleUpdate(0.1)
endevent
