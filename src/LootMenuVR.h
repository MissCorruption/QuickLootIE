#pragma once

#include "LootMenu.h"

namespace QuickLoot
{
	class LootMenuVR : public RE::WorldSpaceMenu, public LootMenu
	{
	public:
		static void Register();
		static IMenu* CreateInstance();

		LootMenuVR();

		static RE::NiPointer<RE::NiNode> GetAttachingNode();
		void DestroyMenuNode();
		void RefreshMenuNode();

		// WorldSpaceMenu implementation

		RE::UI_MESSAGE_RESULTS ProcessMessage(RE::UIMessage& a_message) override;
		void AdvanceMovie(float interval, std::uint32_t currentTime) override;
		void SetTransform() override;
		RE::NiNode* GetMenuParentNode() override;
		RE::BSEventNotifyControl ProcessEvent(const RE::HudModeChangeEvent* a_event, RE::BSTEventSource<RE::HudModeChangeEvent>* a_eventSource) override;
	};
}
