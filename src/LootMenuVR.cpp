#include "LootMenuVR.h"

#include "LootMenuManager.h"

#include <numbers>

#include "RE/I/IMenu.h"
#include "RE/U/UI.h"

// This is a bunch of stuff CLib declares but never defines.
// None of it is used, but without a definition we get linker errors.
// TODO check if this is still the case after updating CLib
namespace RE
{
	class bhkMouseSpringAction : public hkReferencedObject
	{};

	template <class Allocator, class Key, class T>
	std::uint32_t NiTMapBase<Allocator, Key, T>::hash_function(key_type) const
	{
		return 0;
	}

	template <class Allocator, class Key, class T>
	bool NiTMapBase<Allocator, Key, T>::key_eq(key_type, key_type) const
	{
		return false;
	}

	template <class Allocator, class Key, class T>
	void NiTMapBase<Allocator, Key, T>::assign_value(value_type*, key_type, mapped_type)
	{}

	template <class Allocator, class Key, class T>
	void NiTMapBase<Allocator, Key, T>::clear_value(value_type*)
	{}
}

namespace QuickLoot
{
	void LootMenuVR::Register()
	{
		RE::UI::GetSingleton()->Register(MENU_NAME, CreateInstance);
		logger::info("Registered {}", MENU_NAME);
	}

	RE::IMenu* LootMenuVR::CreateInstance()
	{
		logger::debug("Creating new vr loot menu instance");
		return static_cast<WorldSpaceMenu*>(new LootMenuVR());
	}

	LootMenuVR::LootMenuVR() :
		WorldSpaceMenu(false, false, false)
	{
		PROFILE_SCOPE_NAMED("LootMenuVR Constructor")

		const auto wsm = static_cast<WorldSpaceMenu*>(this);

		wsm->menuFlags.reset(Flag::kDisablePauseMenu, Flag::kUsesCursor, Flag::kModal);
		wsm->menuFlags.set(Flag::kAllowSaving, Flag::kHasButtonBar);
		wsm->uiMovie = static_cast<LootMenu*>(this)->uiMovie;

		WorldSpaceMenu::SetupMenuNode();

		static_assert(offsetof(WorldSpaceMenu, menuNode) == 0x48);
	}

	RE::NiPointer<RE::NiNode> LootMenuVR::GetAttachingNode()
	{
		auto& vrData = RE::PlayerCharacter::GetSingleton()->GetVRPlayerRuntimeData();
		return vrData.isRightHandMainHand ? vrData.RightWandNode : vrData.LeftWandNode;
	}

	void LootMenuVR::DestroyMenuNode()
	{
		if (!menuNode) {
			return;
		}

		GetAttachingNode()->DetachChild2(menuNode.get());
		menuNode.reset();
	}

	void LootMenuVR::RefreshMenuNode()
	{
		DestroyMenuNode();
		SetupMenuNode();
		SetTransform();
	}

	RE::UI_MESSAGE_RESULTS LootMenuVR::ProcessMessage(RE::UIMessage& message)
	{
		if (message.type == RE::UI_MESSAGE_TYPE::kHide) {
			Hide();

			return RE::UI_MESSAGE_RESULTS::kHandled;
		}

		return WorldSpaceMenu::ProcessMessage(message);
	}

	void LootMenuVR::AdvanceMovie(float interval, std::uint32_t currentTime)
	{
		// First run all queued tasks, which may set refresh flags
		LootMenuManager::ProcessPendingTasks(*this);

		// Refresh any components that require updates
		Refresh();

		// Redraw visuals
		WorldSpaceMenu::AdvanceMovie(interval, currentTime);
	}

	void LootMenuVR::SetTransform()
	{
		const float dx = Settings::VrOffsetX();
		const float dy = Settings::VrOffsetY();
		const float dz = Settings::VrOffsetZ();

		const float rx = static_cast<float>(Settings::VrAngleX() * std::numbers::pi / 180);
		const float ry = static_cast<float>(Settings::VrAngleY() * std::numbers::pi / 180);
		const float rz = static_cast<float>(Settings::VrAngleZ() * std::numbers::pi / 180);

		const float scale = Settings::VrScale();

		menuNode->local.translate = RE::NiPoint3(dx, dy, dz);
		menuNode->local.rotate.EulerAnglesToAxesZXY(rx, ry, rz);
		menuNode->local.scale = scale;

		GetAttachingNode()->AttachChild(menuNode.get(), true);
		RE::NiUpdateData data{};
		menuNode->Update(data);
	}

	RE::NiNode* LootMenuVR::GetMenuParentNode()
	{
		return RE::PlayerCharacter::GetSingleton()->GetVRNodeData()->UprightHmdNode.get();
	}

	RE::BSEventNotifyControl LootMenuVR::ProcessEvent(const RE::HudModeChangeEvent*, RE::BSTEventSource<RE::HudModeChangeEvent>*)
	{
		return RE::BSEventNotifyControl::kContinue;
	}
}
