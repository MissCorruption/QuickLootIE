#include "Settings.h"
#include "Papyrus/Papyrus.h"

namespace QuickLoot
{
	std::shared_ptr<std::vector<std::string>> Settings::GetUserDefinedSortPriority()
	{
		return QLIESortPriorityStrings.size() == 0 ? 
			std::make_shared<std::vector<std::string>>(Papyrus::GetSortingPreset(nullptr, 1)) : 
			std::make_shared<std::vector<std::string>>(QLIESortPriorityStrings);
	}

	bool Settings::CloseInCombat()
	{
		return QLIECloseInCombat;
	}

	bool Settings::CloseWhenEmpty()
	{
		return QLIECloseWhenEmpty;
	}

	bool Settings::DispelInvisibility()
	{
		return QLIEDispelInvisibility;
	}

	bool Settings::OpenWhenContainerUnlocked()
	{
		return QLIEOpenWhenContainerUnlocked;
	}

	bool Settings::DisableForAnimals()
	{
		return QLIEDisableForAnimals;
	}

	bool Settings::DisableForMonsters()
	{
		return false; //Unimplemented ?
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

	float Settings::WindowX()
	{
		return QLIEWindowX;
	}

	float Settings::WindowY()
	{
		return QLIEWindowY;
	}

	float Settings::WindowW()
	{
		return QLIEWindowW;
	}

	float Settings::WindowH()
	{
		return QLIEWindowH;
	}
}
