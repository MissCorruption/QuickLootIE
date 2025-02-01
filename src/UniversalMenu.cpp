#include "UniversalMenu.h"

UniversalMenu::~UniversalMenu()
{
	// TODO, unregister if registered on hudmodechangevent
	if (menuNode.get()) {
		if (menuNode.get()->parent) {
			menuNode.get()->parent->DetachChild2(menuNode.get());
		}
		menuNode.reset();
	}

	menuName.~BSFixedString();
	fxDelegate.reset();
	uiMovie.reset();
}

UniversalMenu::UniversalMenu(bool a_VRregisterHudModeChangeEvent, bool a_VRmatchAsTopMenu, bool a_VRqueueUpdateFixup)
{
	if (REL::Module::IsVR()) {
		ConstructVR(a_VRregisterHudModeChangeEvent, a_VRmatchAsTopMenu, a_VRqueueUpdateFixup);
	} else {
		ConstructFlatrim();
	}
}

void UniversalMenu::ConstructFlatrim() {
	this->depthPriority = (int8_t) 3;
	auto version = REL::Module::get().version();
	if (version.patch() >= 1130) {
		this->inputContext.set((RE::IMenu::Context) 19);
	}
	this->inputContext.set((RE::IMenu::Context) 18);
}

void UniversalMenu::ConstructVR(bool a_registerHudModeChangeEvent, bool a_matchAsTopMenu, bool a_queueUpdateFixup)
{
	this->unk30 = RE::UI_MENU_Unk09::kNone;
	this->inputContext.set((RE::IMenu::Context) 22);

	this->registerForHudModeChangeEvent = a_registerHudModeChangeEvent;
	this->matchAsTopMenu = a_matchAsTopMenu;
	this->queueUpdateFixup = a_queueUpdateFixup;
	this->Unk_09(RE::UI_MENU_Unk09::kNone);
	this->menuFlags.set(
		RE::UI_MENU_FLAGS::kRequiresUpdate,
		RE::UI_MENU_FLAGS::kAllowSaving,
		RE::UI_MENU_FLAGS::kAssignCursorToRenderer);
	this->depthPriority = (int8_t) 2;
	this->unk34 = (std::byte) 1;
	this->menuName = "N/A";
	if (this->registerForHudModeChangeEvent) {
		// TODO: Register the menu for the hud mode change event
		// See SkyrimVR.exe+53D050
	}
}

void UniversalMenu::PostCreate()
{
	if (REL::Module::IsVR()) {
		using func_t = decltype(&UniversalMenu::PostCreate);
		REL::Relocation<func_t> func{ REL::Offset(0x53C140) };
		return func(this);
	}
	// SE/AE is empt
}

RE::UI_MESSAGE_RESULTS UniversalMenu::ProcessMessage(RE::UIMessage& a_message)
{
	if (REL::Module::IsVR()) {
		using func_t = decltype(&UniversalMenu::ProcessMessage);
		REL::Relocation<func_t> func{ REL::Offset(0x53C2B0) };
		return func(this, a_message);
	} else {
		return IMenu::ProcessMessage(a_message);
	}
}
void UniversalMenu::Unk_09(RE::UI_MENU_Unk09 a_unk)
{
	assert(REL::Module::IsVR());
	using func_t = decltype(&UniversalMenu::Unk_09);
	REL::Relocation<func_t> func{ REL::Offset(0x53C220) };
	return func(this, a_unk);
}

void UniversalMenu::SetupMenuNode()
{
	assert(REL::Module::IsVR());
	using func_t = decltype(&UniversalMenu::SetupMenuNode);
	REL::Relocation<func_t> func{ REL::Offset(0x53C450) };
	return func(this);
}

