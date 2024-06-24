#pragma once
namespace QuickLoot
{
	class Settings
	{
	public:
		Settings(Settings const&) = delete;
		Settings(Settings const&&) = delete;
		Settings operator=(Settings&) = delete;
		Settings operator=(Settings&&) = delete;

		static bool CloseInCombat();
		static bool CloseWhenEmpty();
		static bool DispelInvisibility();
		static bool OpenWhenContainerUnlocked();
		static bool DisableForAnimals();
		static bool DisableForMonsters();

		static bool ShowBookRead();
		static bool ShowEnchanted();
		static bool ShowDBMDisplayed();
		static bool ShowDBMFound();
		static bool ShowDBMNew();

		static bool ShowCompNeeded();
		static bool ShowCompCollected();
		static float WindowX();
		static float WindowY();
		static float WindowW();
		static float WindowH();

		static std::shared_ptr<std::vector<std::string>> GetUserDefinedSortPriority();

	private:
		Settings() = default;
		~Settings() = default;
	};
}
