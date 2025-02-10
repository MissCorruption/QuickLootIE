#pragma once

#include "RE/B/BSTEvent.h"
#include "RE/I/IMenu.h"
#include "RE/N/NiNode.h"

/*
 * Custom menu designed to support SE/AE and VR.
 * VR has specific virtual functions that must be implemented. VR-only vfuncs will only be executed when running in VR.
 * The menu by default will execute VR's WorldSpaceMenu logic in VR, and Flatrim's IMenu logic in SE/AE.
 */
class UniversalMenu :
	public RE::IMenu,                            // 00
	public RE::BSTEventSink<RE::HudModeChangeEvent>  // 40
{
public:
	UniversalMenu(bool a_vrRegisterForHudModeChangeEvent, bool a_vrMatchAsTopMenu, bool a_vrQueueUpdateFixup);

	~UniversalMenu() override;  // 00

	// override
	virtual void PostCreate() override;                                                // 02
	virtual RE::UI_MESSAGE_RESULTS ProcessMessage(RE::UIMessage& a_message) override;  // 04
	virtual void Unk_09(RE::UI_MENU_Unk09 a_unk) override;                             // 09

	// add
	virtual void SetupMenuNode();                 // 0B - Sets up menuNode
	virtual RE::NiNode* GetMenuParentNode() = 0;  // 0C - purecall, menuNode is attached to this in PostCreate()
	virtual void SetTransform() = 0;              // 0D - purecall, set rotation,translation, etc. called in PostCreate()

	RE::NiPointer<RE::NiNode> menuNode;  // 48 - Created automatically in SetupMenuNode()
	bool registerForHudModeChangeEvent;  // 50 - Set/Used in UniversalMenu constructor
	bool matchAsTopMenu{ false };        // 51 - Only used when calling MessageMatchesMenu so far
	bool queueUpdateFixup{ false };      // 52 - Update menuNode immediately, or on next MAIN_DRAW when calling Unk_09
	std::uint8_t pad53{ 0 };
	std::uint32_t pad54{ 0 };

private:
	void ConstructFlatrim();
	void ConstructVR(bool a_registerForHudModeChangeEvent, bool a_matchAsTopMenu, bool a_queueUpdateFixup);
};
static_assert(sizeof(UniversalMenu) == 0x58);
