#include "Settings.h"

#include "Behaviors/LockpickActivation.h"
#include "Config/Papyrus.h"

namespace QuickLoot::Config
{
	void Settings::Update()
	{
		Papyrus::UpdateVariables();

		if (ShowWhenUnlocked()) {
			Behaviors::LockpickActivation::Block();
		} else {
			Behaviors::LockpickActivation::Unblock();
		}
	}

	bool Settings::ShowInCombat() { return QLIE_ShowInCombat; }
	bool Settings::ShowWhenEmpty() { return QLIE_ShowWhenEmpty; }
	bool Settings::ShowWhenUnlocked() { return QLIE_ShowWhenUnlocked; }
	bool Settings::ShowInThirdPersonView() { return QLIE_ShowInThirdPerson; }
	bool Settings::ShowWhenMounted() { return QLIE_ShowWhenMounted; }
	bool Settings::EnableForAnimals() { return QLIE_EnableForAnimals; }
	bool Settings::EnableForDragons() { return QLIE_EnableForDragons; }
	bool Settings::DispelInvisibility() { return QLIE_BreakInvisibility; }

	int Settings::GetWindowX() { return QLIE_WindowOffsetX; }
	int Settings::GetWindowY() { return QLIE_WindowOffsetY; }
	float Settings::GetWindowScale() { return QLIE_WindowScale; }
	AnchorPoint Settings::GetWindowAnchor() { return static_cast<AnchorPoint>(QLIE_WindowAnchor); }
	int Settings::GetWindowMinLines() { return QLIE_WindowMinLines; }
	int Settings::GetWindowMaxLines() { return QLIE_WindowMaxLines; }
	float Settings::GetWindowOpacityNormal() { return QLIE_WindowOpacityNormal; }
	float Settings::GetWindowOpacityEmpty() { return QLIE_WindowOpacityEmpty; }

	bool Settings::ShowIconRead() { return QLIE_ShowIconRead; }
	bool Settings::ShowIconStolen() { return QLIE_ShowIconStolen; }
	bool Settings::ShowIconEnchanted() { return QLIE_ShowIconEnchanted; }
	bool Settings::ShowIconEnchantedKnown() { return QLIE_ShowIconEnchantedKnown; }
	bool Settings::ShowIconEnchantedSpecial() { return QLIE_ShowIconEnchantedSpecial; }

	const std::vector<std::string>& Settings::GetUserDefinedSortPriority() { return QLIE_SortRulesActive; }

	bool Settings::ShowDBMDisplayed() { return QLIE_ShowIconLOTDDisplayed; }
	bool Settings::ShowDBMFound() { return QLIE_ShowIconLOTDCarried; }
	bool Settings::ShowDBMNew() { return QLIE_ShowIconLOTDNew; }
	bool Settings::ShowCompNeeded() { return QLIE_ShowIconCompletionistNeeded; }
	bool Settings::ShowCompCollected() { return QLIE_ShowIconCompletionistCollected; }
}
