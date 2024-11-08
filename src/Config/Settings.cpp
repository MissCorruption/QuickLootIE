#include "Settings.h"

#include "Behaviors/LockpickActivation.h"
#include "Config/Papyrus.h"

namespace QuickLoot::Config
{
	std::shared_ptr<std::vector<std::string>> Settings::GetUserDefinedSortPriority()
	{
		return QLIESortPriorityStrings.size() == 0 ?
		           std::make_shared<std::vector<std::string>>(Papyrus::GetSortingPreset(nullptr, 1)) :
		           std::make_shared<std::vector<std::string>>(QLIESortPriorityStrings);
	}

	void Settings::Update()
	{
		Papyrus::UpdateVariables();

		if (EnableAfterUnlocking()) {
			Behaviors::LockpickActivation::Block();
		} else {
			Behaviors::LockpickActivation::Unblock();
		}
	}

	bool Settings::EnableInCombat()
	{
		return !QLIECloseInCombat;
	}

	bool Settings::EnableWhenEmpty()
	{
		return !QLIECloseWhenEmpty;
	}

	bool Settings::EnableAfterUnlocking()
	{
		return QLIEOpenWhenContainerUnlocked;
	}

	bool Settings::EnableInThirdPersonView()
	{
		// TODO implement
		return true;
	}

	bool Settings::EnableWhenMounted()
	{
		// TODO implement
		return true;
	}

	bool Settings::EnableForAnimals()
	{
		return !QLIEDisableForAnimals;
	}

	bool Settings::EnableForDragons()
	{
		// TODO implement
		return true;
	}

	bool Settings::DispelInvisibility()
	{
		return QLIEDispelInvisibility;
	}

	bool Settings::ShowBookRead()
	{
		return QLIEIconShowBookRead;
	}

	bool Settings::ShowEnchanted()
	{
		return QLIEIconShowEnchanted;
	}

	bool Settings::ShowDBMDisplayed()
	{
		return QLIEIconShowDBMDisplayed;
	}

	bool Settings::ShowDBMFound()
	{
		return QLIEIconShowDBMFound;
	}

	bool Settings::ShowDBMNew()
	{
		return QLIEIconShowDBMNew;
	}

	bool Settings::ShowCompNeeded()
	{
		return QLIEShowCompNeeded;
	}

	bool Settings::ShowCompCollected()
	{
		return QLIEShowCompCollected;
	}

	AnchorPoint Settings::GetAnchorPoint()
	{
		return static_cast<AnchorPoint>(QLIEAnchorPoint);
	}

	int32_t Settings::GetWindowX()
	{
		return QLIEWindowX;
	}

	int32_t Settings::GetWindowY()
	{
		return QLIEWindowY;
	}

	float Settings::GetWindowScale()
	{
		return QLIEWindowScale;
	}

	int32_t Settings::GetMinLines()
	{
		return QLIEMinLines;
	}

	int32_t Settings::GetMaxLines()
	{
		return QLIEMaxLines;
	}

	float Settings::GetNormalWindowTransparency()
	{
		return QLIETransparency_Normal;
	}

	float Settings::GetEmptyWindowTransparency()
	{
		return QLIETransparency_Empty;
	}

}
