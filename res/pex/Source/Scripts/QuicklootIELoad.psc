Scriptname QuicklootIELoad extends ReferenceAlias  

QuickLootIEMaintenance Property QLIEAPI Auto

;---------------------------------------------------
;-- Events -----------------------------------------
;---------------------------------------------------

Event OnPlayerLoadGame()
	QLIEAPI.RegisterForSingleUpdate(0.1)
endEvent
